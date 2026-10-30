/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "base/Log.h"

#include <QTest>

class CursorLockTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void isLocked_nothingSet_unlocked();
  void isLocked_manual_locksEveryComputer();
  void isLocked_gameLock_onlyOnItsComputer();
  void setGameLock_gameEnds_manualLockKept();
  void setManual_manualEnds_gameLockKept();
  void setGameLock_clientGone_primaryLockKept();
  void setGameLock_sameState_reportsNoChange();
  void gameLockMonitor_foregroundChanges_switchingFollows();

private:
  Log m_log;
};
