/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "base/Log.h"

#include <QTest>

class GameLockMonitorTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void start_fullscreenGameInFront_locks();
  void start_watchesForegroundUntilStopped();
  void check_stillFullscreen_reportsOnce();
  void check_gameClosed_releases();
  void check_altTabToOtherWindow_releasesRightAway();
  void check_gameMinimized_releasesRightAway();
  void check_gameLeavesFullscreen_releasesAfterGrace();
  void check_gameBackToFullscreenWithinGrace_staysLocked();
  void check_switchBetweenFullscreenGames_staysLocked();
  void check_ignoredApp_neverLocks();
  void check_disabledPolicy_neverLocks();
  void stop_whileLocked_unlocksWithoutReport();

private:
  Log m_log;
};
