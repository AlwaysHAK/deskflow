/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <QTest>

class GameLockPolicyTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void initTestCase();
  void normalizeExecutable_data();
  void normalizeExecutable();
  void normalizeExecutables_mixedEntries_splitAndDeduped();
  void modeFromString_data();
  void modeFromString();
  void shouldLock_data();
  void shouldLock();
  void fromSettings_missingKeys_defaults();
  void fromSettings_iniFile_parsed();
  void fromSettings_savedValues_roundTrip();

private:
  // Settings only reloads when the file name changes, so every test uses its own file
  inline static const QString m_settingsPath = QStringLiteral("tmp/gamelock");
  inline static const QString m_stateFile = QStringLiteral("%1/Deskflow.state").arg(m_settingsPath);
  inline static const QString m_initialFile = QStringLiteral("%1/initial.conf").arg(m_settingsPath);
  inline static const QString m_defaultsFile = QStringLiteral("%1/defaults.conf").arg(m_settingsPath);
  inline static const QString m_iniFile = QStringLiteral("%1/ini.conf").arg(m_settingsPath);
  inline static const QString m_savedFile = QStringLiteral("%1/saved.conf").arg(m_settingsPath);
};
