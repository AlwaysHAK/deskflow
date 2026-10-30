/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "GameLockMonitorTests.h"

#include "MockEventQueue.h"
#include "deskflow/GameLockMonitor.h"

#include <vector>

using deskflow::ForegroundWindow;
using deskflow::GameLockMonitor;
using deskflow::GameLockPolicy;

namespace {

ForegroundWindow fullscreen(const std::string &executable, std::uintptr_t window)
{
  return {.present = true, .fullscreen = true, .executable = executable, .window = window};
}

ForegroundWindow windowed(const std::string &executable, std::uintptr_t window)
{
  return {.present = true, .fullscreen = false, .executable = executable, .window = window};
}

const ForegroundWindow kNothingInFront;

/// Stands in for the computer: the test sets the foreground window and reads the reports
struct FakeComputer
{
  ForegroundWindow foreground;
  std::vector<bool> watchCalls;
  std::vector<bool> reports;
  MockEventQueue events;

  std::unique_ptr<GameLockMonitor> makeMonitor(const GameLockPolicy &policy)
  {
    GameLockMonitor::Deps deps{
        .foregroundWindow = [this] { return foreground; },
        .watchForegroundWindow = [this](bool watch) { watchCalls.push_back(watch); },
        .eventTarget = this
    };
    return std::make_unique<GameLockMonitor>(&events, policy, deps, [this](bool locked) { reports.push_back(locked); });
  }
};

GameLockPolicy anyFullscreen()
{
  return GameLockPolicy::fromValues(true, "any", {}, {"chrome.exe"}, false);
}

} // namespace

void GameLockMonitorTests::start_fullscreenGameInFront_locks()
{
  FakeComputer computer;
  computer.foreground = fullscreen("eldenring.exe", 1);
  auto monitor = computer.makeMonitor(anyFullscreen());

  monitor->start();

  QVERIFY(monitor->isLocked());
  QCOMPARE(computer.reports, std::vector<bool>({true}));
}

void GameLockMonitorTests::start_watchesForegroundUntilStopped()
{
  FakeComputer computer;
  auto monitor = computer.makeMonitor(anyFullscreen());

  monitor->start();
  monitor->start();
  monitor->stop();
  monitor->stop();

  QCOMPARE(computer.watchCalls, std::vector<bool>({true, false}));
}

void GameLockMonitorTests::check_stillFullscreen_reportsOnce()
{
  FakeComputer computer;
  computer.foreground = fullscreen("eldenring.exe", 1);
  auto monitor = computer.makeMonitor(anyFullscreen());
  monitor->start();

  monitor->check();
  monitor->check();

  QCOMPARE(computer.reports, std::vector<bool>({true}));
}

void GameLockMonitorTests::check_gameClosed_releases()
{
  FakeComputer computer;
  computer.foreground = fullscreen("eldenring.exe", 1);
  auto monitor = computer.makeMonitor(anyFullscreen());
  monitor->start();

  computer.foreground = kNothingInFront;
  monitor->check();

  QVERIFY(!monitor->isLocked());
  QCOMPARE(computer.reports, std::vector<bool>({true, false}));
}

void GameLockMonitorTests::check_altTabToOtherWindow_releasesRightAway()
{
  FakeComputer computer;
  computer.foreground = fullscreen("eldenring.exe", 1);
  auto monitor = computer.makeMonitor(anyFullscreen());
  monitor->start();

  computer.foreground = windowed("discord.exe", 2);
  monitor->check();

  QCOMPARE(computer.reports, std::vector<bool>({true, false}));
}

void GameLockMonitorTests::check_gameMinimized_releasesRightAway()
{
  FakeComputer computer;
  computer.foreground = fullscreen("eldenring.exe", 1);
  auto monitor = computer.makeMonitor(anyFullscreen());
  monitor->start();

  // a minimized window is reported as nothing in front, even though it is the same window
  computer.foreground = {.present = false, .fullscreen = false, .executable = "eldenring.exe", .window = 1};
  monitor->check();

  QCOMPARE(computer.reports, std::vector<bool>({true, false}));
}

void GameLockMonitorTests::check_gameLeavesFullscreen_releasesAfterGrace()
{
  FakeComputer computer;
  computer.foreground = fullscreen("eldenring.exe", 1);
  auto monitor = computer.makeMonitor(anyFullscreen());
  monitor->start();

  computer.foreground = windowed("eldenring.exe", 1);
  for (int i = 1; i < GameLockMonitor::s_releaseGraceChecks; ++i) {
    monitor->check();
    QVERIFY(monitor->isLocked());
  }
  monitor->check();

  QVERIFY(!monitor->isLocked());
  QCOMPARE(computer.reports, std::vector<bool>({true, false}));
}

void GameLockMonitorTests::check_gameBackToFullscreenWithinGrace_staysLocked()
{
  FakeComputer computer;
  computer.foreground = fullscreen("eldenring.exe", 1);
  auto monitor = computer.makeMonitor(anyFullscreen());
  monitor->start();

  // e.g. a display mode switch, twice in a row
  for (int i = 0; i < 2; ++i) {
    computer.foreground = windowed("eldenring.exe", 1);
    monitor->check();
    computer.foreground = fullscreen("eldenring.exe", 1);
    monitor->check();
  }

  QVERIFY(monitor->isLocked());
  QCOMPARE(computer.reports, std::vector<bool>({true}));
}

void GameLockMonitorTests::check_switchBetweenFullscreenGames_staysLocked()
{
  FakeComputer computer;
  computer.foreground = fullscreen("eldenring.exe", 1);
  auto monitor = computer.makeMonitor(anyFullscreen());
  monitor->start();

  computer.foreground = fullscreen("cs2.exe", 2);
  monitor->check();
  computer.foreground = windowed("cs2.exe", 2);
  monitor->check();

  // the grace applies to the window that holds the lock now
  QVERIFY(monitor->isLocked());
  QCOMPARE(computer.reports, std::vector<bool>({true}));
}

void GameLockMonitorTests::check_ignoredApp_neverLocks()
{
  FakeComputer computer;
  computer.foreground = fullscreen("Chrome.exe", 1);
  auto monitor = computer.makeMonitor(anyFullscreen());

  monitor->start();
  monitor->check();

  QVERIFY(!monitor->isLocked());
  QVERIFY(computer.reports.empty());
}

void GameLockMonitorTests::check_disabledPolicy_neverLocks()
{
  FakeComputer computer;
  computer.foreground = fullscreen("eldenring.exe", 1);
  auto monitor = computer.makeMonitor(GameLockPolicy::fromValues(false, "any", {}, {}, false));

  monitor->start();

  QVERIFY(!monitor->isLocked());
  QVERIFY(computer.reports.empty());
}

void GameLockMonitorTests::stop_whileLocked_unlocksWithoutReport()
{
  FakeComputer computer;
  computer.foreground = fullscreen("eldenring.exe", 1);
  auto monitor = computer.makeMonitor(anyFullscreen());
  monitor->start();

  monitor->stop();

  QVERIFY(!monitor->isLocked());
  QCOMPARE(computer.reports, std::vector<bool>({true}));
}

QTEST_MAIN(GameLockMonitorTests)
