/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "CursorLockTests.h"

#include "../deskflow/MockEventQueue.h"
#include "deskflow/GameLockMonitor.h"
#include "server/CursorLock.h"

using deskflow::ForegroundWindow;
using deskflow::GameLockMonitor;
using deskflow::GameLockPolicy;
using deskflow::server::CursorLock;

namespace {
const std::string kServer = "server";
const std::string kGamer = "gamer";
const std::string kLaptop = "laptop";
} // namespace

void CursorLockTests::isLocked_nothingSet_unlocked()
{
  CursorLock lock;

  QVERIFY(!lock.isLocked(kServer));
}

void CursorLockTests::isLocked_manual_locksEveryComputer()
{
  CursorLock lock;

  lock.setManual(true);

  QVERIFY(lock.isLocked(kServer));
  QVERIFY(lock.isLocked(kGamer));
}

void CursorLockTests::isLocked_gameLock_onlyOnItsComputer()
{
  CursorLock lock;

  lock.setGameLock(kGamer, true);

  // the cursor may still move from the laptop to the gamer and back out with a hotkey
  QVERIFY(lock.isLocked(kGamer));
  QVERIFY(!lock.isLocked(kLaptop));
  QVERIFY(!lock.isManual());
}

void CursorLockTests::setGameLock_gameEnds_manualLockKept()
{
  CursorLock lock;
  lock.setManual(true);
  lock.setGameLock(kGamer, true);

  lock.setGameLock(kGamer, false);

  QVERIFY(lock.isManual());
  QVERIFY(lock.isLocked(kGamer));
}

void CursorLockTests::setManual_manualEnds_gameLockKept()
{
  CursorLock lock;
  lock.setGameLock(kGamer, true);
  lock.setManual(true);

  lock.setManual(false);

  QVERIFY(lock.isGameLocked(kGamer));
  QVERIFY(lock.isLocked(kGamer));
}

void CursorLockTests::setGameLock_clientGone_primaryLockKept()
{
  CursorLock lock;
  lock.setGameLock(kServer, true);
  lock.setGameLock(kGamer, true);

  lock.setGameLock(kGamer, false);

  QVERIFY(!lock.isLocked(kGamer));
  QVERIFY(lock.isLocked(kServer));
}

void CursorLockTests::setGameLock_sameState_reportsNoChange()
{
  CursorLock lock;

  QVERIFY(!lock.setGameLock(kGamer, false));
  QVERIFY(lock.setGameLock(kGamer, true));
  QVERIFY(!lock.setGameLock(kGamer, true));
  QVERIFY(lock.setGameLock(kGamer, false));
}

void CursorLockTests::gameLockMonitor_foregroundChanges_switchingFollows()
{
  MockEventQueue events;
  CursorLock lock;
  ForegroundWindow foreground{.present = true, .fullscreen = true, .executable = "eldenring.exe", .window = 1};
  GameLockMonitor::Deps deps{
      .foregroundWindow = [&foreground] { return foreground; },
      .watchForegroundWindow = [](bool) {},
      .eventTarget = nullptr
  };
  GameLockMonitor monitor(
      &events, GameLockPolicy::fromValues(true, "selected", {"eldenring.exe"}, {}, false), deps,
      [&lock](bool locked) { lock.setGameLock(kServer, locked); }
  );

  monitor.start();
  QVERIFY(lock.isLocked(kServer));

  lock.setManual(true);
  foreground = {.present = true, .fullscreen = false, .executable = "explorer.exe", .window = 2};
  monitor.check();
  QVERIFY(!lock.isGameLocked(kServer));
  QVERIFY(lock.isLocked(kServer));

  lock.setManual(false);
  QVERIFY(!lock.isLocked(kServer));
}

QTEST_MAIN(CursorLockTests)
