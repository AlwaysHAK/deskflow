/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <QTest>

class GameLockWidgetTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void initTestCase();
  void addApps_mixedEntries_normalizedAndUnique();
  void load_storedSettings_shownUnmodified();
  void save_changedMode_storedAndUnmodified();
  void resetToDefault_storedSettings_defaultAndModified();

private:
  inline static const QString m_settingsPath = QStringLiteral("tmp/gamelockwidget");
  inline static const QString m_settingsFile = QStringLiteral("%1/Deskflow.conf").arg(m_settingsPath);
  inline static const QString m_stateFile = QStringLiteral("%1/Deskflow.state").arg(m_settingsPath);
};
