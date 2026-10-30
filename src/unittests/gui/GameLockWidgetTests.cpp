/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "GameLockWidgetTests.h"

#include "common/Settings.h"
#include "gui/widgets/AppListWidget.h"
#include "gui/widgets/GameLockWidget.h"

#include <QCheckBox>
#include <QDir>
#include <QGroupBox>
#include <QRadioButton>
#include <QSignalSpy>

namespace {

void storeSelectedModeSettings()
{
  Settings::setValue(Settings::GameLock::Enabled, true);
  Settings::setValue(Settings::GameLock::Mode, QStringLiteral("selected"));
  Settings::setValue(Settings::GameLock::Apps, QStringList{"eldenring.exe", "cs2.exe"});
  Settings::setValue(Settings::GameLock::Ignore, QStringList{"chrome.exe"});
  Settings::setValue(Settings::GameLock::WhenWindowed, true);
}

QStringList sorted(QStringList list)
{
  list.sort();
  return list;
}

} // namespace

void GameLockWidgetTests::initTestCase()
{
  QDir(m_settingsPath).removeRecursively();
  QVERIFY(QDir().mkpath(m_settingsPath));
  Settings::setSettingsFile(m_settingsFile);
  Settings::setStateFile(m_stateFile);
}

void GameLockWidgetTests::addApps_mixedEntries_normalizedAndUnique()
{
  AppListWidget list;
  QSignalSpy spy(&list, &AppListWidget::changed);

  list.addApps({"EldenRing.exe", R"(C:\Games\Counter-Strike\cs2.exe)", "eldenring.exe", ""});
  list.addApps({"CS2.EXE"});

  QCOMPARE(sorted(list.apps()), QStringList({"cs2.exe", "eldenring.exe"}));
  QCOMPARE(spy.count(), 1);
}

void GameLockWidgetTests::load_storedSettings_shownUnmodified()
{
  storeSelectedModeSettings();

  GameLockWidget widget;

  QVERIFY(widget.findChild<QGroupBox *>("groupGameLock")->isChecked());
  QVERIFY(widget.findChild<QRadioButton *>("rbSelected")->isChecked());
  QVERIFY(widget.findChild<QCheckBox *>("cbWhenWindowed")->isChecked());
  QCOMPARE(sorted(widget.findChild<AppListWidget *>("listApps")->apps()), QStringList({"cs2.exe", "eldenring.exe"}));
  QCOMPARE(widget.findChild<AppListWidget *>("listIgnore")->apps(), QStringList({"chrome.exe"}));
  QVERIFY(!widget.isModified());
}

void GameLockWidgetTests::save_changedMode_storedAndUnmodified()
{
  storeSelectedModeSettings();
  GameLockWidget widget;
  QSignalSpy spy(&widget, &GameLockWidget::changed);

  widget.findChild<QRadioButton *>("rbAny")->setChecked(true);
  QVERIFY(spy.count() > 0);
  QVERIFY(widget.isModified());
  widget.save();

  QCOMPARE(Settings::value(Settings::GameLock::Mode).toString(), QStringLiteral("any"));
  QVERIFY(!widget.isModified());
}

void GameLockWidgetTests::resetToDefault_storedSettings_defaultAndModified()
{
  storeSelectedModeSettings();
  GameLockWidget widget;

  widget.resetToDefault();

  QVERIFY(widget.isDefault());
  QVERIFY(widget.isModified());
  QVERIFY(!widget.findChild<QGroupBox *>("groupGameLock")->isChecked());
  QVERIFY(widget.findChild<AppListWidget *>("listApps")->apps().isEmpty());
}

QTEST_MAIN(GameLockWidgetTests)
