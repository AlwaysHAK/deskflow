/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "GameLockPolicyTests.h"

#include "common/GameLockPolicy.h"
#include "common/Settings.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

using deskflow::ForegroundWindow;
using deskflow::GameLockPolicy;
using Mode = GameLockPolicy::Mode;

Q_DECLARE_METATYPE(GameLockPolicy::Mode)

namespace {

void writeSettingsFile(const QString &path, const QByteArray &contents)
{
  QDir().mkpath(QFileInfo(path).absolutePath());
  QFile file(path);
  QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
  file.write(contents);
}

} // namespace

void GameLockPolicyTests::initTestCase()
{
  QDir(m_settingsPath).removeRecursively();
  writeSettingsFile(m_initialFile, "");
  Settings::setSettingsFile(m_initialFile);
  Settings::setStateFile(m_stateFile);
}

void GameLockPolicyTests::normalizeExecutable_data()
{
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("expected");

  QTest::newRow("lower case") << "eldenring.exe" << "eldenring.exe";
  QTest::newRow("mixed case") << "EldenRing.EXE" << "eldenring.exe";
  QTest::newRow("whitespace") << "  cs2.exe \t" << "cs2.exe";
  QTest::newRow("windows path") << R"(C:\Games\ELDEN RING\Game\eldenring.exe)" << "eldenring.exe";
  QTest::newRow("quoted path") << R"("C:\Program Files\Game\Game.exe")" << "game.exe";
  QTest::newRow("forward slashes") << "C:/Games/witcher3.exe" << "witcher3.exe";
  QTest::newRow("no extension") << "eldenring" << "eldenring.exe";
  QTest::newRow("other extension") << "game.bin" << "game.bin";
  QTest::newRow("empty") << "" << "";
  QTest::newRow("only quotes") << R"("")" << "";
}

void GameLockPolicyTests::normalizeExecutable()
{
  QFETCH(QString, input);
  QFETCH(QString, expected);

  QCOMPARE(GameLockPolicy::normalizeExecutable(input), expected);
}

void GameLockPolicyTests::normalizeExecutables_mixedEntries_splitAndDeduped()
{
  const QStringList input = {"EldenRing.exe", "eldenring.exe, cs2.exe", "", " CS2.EXE ", "witcher3"};

  const auto result = GameLockPolicy::normalizeExecutables(input);

  QCOMPARE(result, QStringList({"eldenring.exe", "cs2.exe", "witcher3.exe"}));
}

void GameLockPolicyTests::modeFromString_data()
{
  QTest::addColumn<QString>("input");
  QTest::addColumn<Mode>("expected");

  QTest::newRow("any") << "any" << Mode::AnyFullscreen;
  QTest::newRow("selected") << "selected" << Mode::SelectedApps;
  QTest::newRow("selected upper case") << " SELECTED " << Mode::SelectedApps;
  QTest::newRow("unknown") << "games" << Mode::AnyFullscreen;
  QTest::newRow("empty") << "" << Mode::AnyFullscreen;
}

void GameLockPolicyTests::modeFromString()
{
  QFETCH(QString, input);
  QFETCH(Mode, expected);

  QCOMPARE(GameLockPolicy::modeFromString(input), expected);
  QCOMPARE(GameLockPolicy::modeFromString(GameLockPolicy::modeToString(expected)), expected);
}

void GameLockPolicyTests::shouldLock_data()
{
  QTest::addColumn<bool>("enabled");
  QTest::addColumn<QString>("mode");
  QTest::addColumn<QStringList>("apps");
  QTest::addColumn<QStringList>("ignored");
  QTest::addColumn<bool>("whenWindowed");
  QTest::addColumn<bool>("present");
  QTest::addColumn<bool>("fullscreen");
  QTest::addColumn<QString>("executable");
  QTest::addColumn<bool>("expected");

  const QStringList none;
  const QStringList games = {"eldenring.exe", "cs2.exe"};
  const QStringList browsers = {"chrome.exe"};

  // clang-format off
  QTest::newRow("disabled, fullscreen game")
      << false << "any" << none << none << false << true << true << "eldenring.exe" << false;

  QTest::newRow("any, fullscreen game")
      << true << "any" << none << none << false << true << true << "eldenring.exe" << true;
  QTest::newRow("any, maximized window")
      << true << "any" << none << none << false << true << false << "notepad.exe" << false;
  QTest::newRow("any, nothing in front")
      << true << "any" << none << none << false << false << true << "" << false;
  QTest::newRow("any, unnamed fullscreen app")
      << true << "any" << none << none << false << true << true << "" << true;
  QTest::newRow("any, ignored fullscreen browser")
      << true << "any" << none << browsers << false << true << true << "chrome.exe" << false;
  QTest::newRow("any, ignore list is case insensitive")
      << true << "any" << none << browsers << false << true << true << "Chrome.EXE" << false;
  QTest::newRow("any, selection is not used")
      << true << "any" << games << none << false << true << true << "vlc.exe" << true;
  QTest::newRow("any, windowed option is not used")
      << true << "any" << games << none << true << true << false << "eldenring.exe" << false;

  QTest::newRow("selected, selected game fullscreen")
      << true << "selected" << games << none << false << true << true << "cs2.exe" << true;
  QTest::newRow("selected, selected game windowed")
      << true << "selected" << games << none << false << true << false << "cs2.exe" << false;
  QTest::newRow("selected, selected game windowed, lock when windowed")
      << true << "selected" << games << none << true << true << false << "cs2.exe" << true;
  QTest::newRow("selected, selected game minimized, lock when windowed")
      << true << "selected" << games << none << true << false << false << "cs2.exe" << false;
  QTest::newRow("selected, other app fullscreen")
      << true << "selected" << games << none << false << true << true << "vlc.exe" << false;
  QTest::newRow("selected, unnamed fullscreen app")
      << true << "selected" << games << none << true << true << true << "" << false;
  QTest::newRow("selected, ignore list wins over selection")
      << true << "selected" << games << QStringList{"cs2.exe"} << true << true << true << "cs2.exe" << false;
  QTest::newRow("selected, nothing selected")
      << true << "selected" << none << none << true << true << true << "cs2.exe" << false;
  // clang-format on
}

void GameLockPolicyTests::shouldLock()
{
  QFETCH(bool, enabled);
  QFETCH(QString, mode);
  QFETCH(QStringList, apps);
  QFETCH(QStringList, ignored);
  QFETCH(bool, whenWindowed);
  QFETCH(bool, present);
  QFETCH(bool, fullscreen);
  QFETCH(QString, executable);
  QFETCH(bool, expected);

  const auto policy = GameLockPolicy::fromValues(enabled, mode, apps, ignored, whenWindowed);
  ForegroundWindow window;
  window.present = present;
  window.fullscreen = fullscreen;
  window.executable = executable.toStdString();
  window.window = 0x1234;

  QCOMPARE(policy.shouldLock(window), expected);
}

void GameLockPolicyTests::fromSettings_missingKeys_defaults()
{
  writeSettingsFile(m_defaultsFile, "[core]\nport=24800\n");
  Settings::setSettingsFile(m_defaultsFile);

  const auto policy = GameLockPolicy::fromSettings();

  QVERIFY(!policy.isEnabled());
  QCOMPARE(policy.mode(), Mode::AnyFullscreen);
  QVERIFY(policy.apps().isEmpty());
  QVERIFY(policy.ignored().isEmpty());
  QVERIFY(!policy.lockWhenWindowed());
}

void GameLockPolicyTests::fromSettings_iniFile_parsed()
{
  writeSettingsFile(
      m_iniFile, "[gameLock]\n"
                 "enabled=true\n"
                 "mode=selected\n"
                 "apps=EldenRing.exe, cs2.exe\n"
                 "ignore=chrome.exe\n"
                 "whenWindowed=true\n"
  );
  Settings::setSettingsFile(m_iniFile);

  const auto policy = GameLockPolicy::fromSettings();

  QVERIFY(policy.isEnabled());
  QCOMPARE(policy.mode(), Mode::SelectedApps);
  QCOMPARE(policy.apps(), QStringList({"eldenring.exe", "cs2.exe"}));
  QCOMPARE(policy.ignored(), QStringList({"chrome.exe"}));
  QVERIFY(policy.lockWhenWindowed());
}

void GameLockPolicyTests::fromSettings_savedValues_roundTrip()
{
  writeSettingsFile(m_savedFile, "");
  Settings::setSettingsFile(m_savedFile);

  Settings::setValue(Settings::GameLock::Enabled, true);
  Settings::setValue(Settings::GameLock::Mode, GameLockPolicy::modeToString(Mode::SelectedApps));
  Settings::setValue(Settings::GameLock::Apps, QStringList{"eldenring.exe"});
  Settings::setValue(Settings::GameLock::Ignore, QStringList{"chrome.exe", "msedge.exe"});
  Settings::save(false);

  // load the saved file again from disk
  Settings::setSettingsFile(m_initialFile);
  Settings::setSettingsFile(m_savedFile);

  const auto policy = GameLockPolicy::fromSettings();

  QVERIFY(policy.isEnabled());
  QCOMPARE(policy.mode(), Mode::SelectedApps);
  QCOMPARE(policy.apps(), QStringList({"eldenring.exe"}));
  QCOMPARE(policy.ignored(), QStringList({"chrome.exe", "msedge.exe"}));
  QVERIFY(!policy.lockWhenWindowed());
}

QTEST_MAIN(GameLockPolicyTests)
