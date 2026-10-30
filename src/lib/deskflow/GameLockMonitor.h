/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "common/ForegroundWindow.h"
#include "common/GameLockPolicy.h"

#include <functional>

class EventQueueTimer;
class IEventQueue;
class IPlatformComputer;

namespace deskflow {

/**
 * @brief Watches the foreground window of this computer and reports when game lock
 * should hold the cursor here.
 *
 * It checks the foreground window on a low frequency timer and whenever the computer
 * reports a foreground change, and only while it runs.
 */
class GameLockMonitor
{
public:
  /// What the monitor needs from the computer it watches
  struct Deps
  {
    std::function<ForegroundWindow()> foregroundWindow;
    std::function<void(bool)> watchForegroundWindow;
    /// target of EventTypes::ComputerForegroundChanged
    void *eventTarget = nullptr;
  };

  using Changed = std::function<void(bool locked)>;

  /// How often the foreground window is checked, in seconds
  inline static const double s_pollInterval = 0.25;

  /// How many checks in a row the window holding the lock may fail the policy before the
  /// lock is released, so the cursor does not slip away while a game switches display mode
  inline static const int s_releaseGraceChecks = 2;

  static Deps depsFor(IPlatformComputer *computer);

  GameLockMonitor(IEventQueue *events, const GameLockPolicy &policy, const Deps &deps, const Changed &changed);
  GameLockMonitor(const GameLockMonitor &) = delete;
  GameLockMonitor &operator=(const GameLockMonitor &) = delete;
  ~GameLockMonitor();

  /**
   * @brief Start watching and check the foreground window right away.
   */
  void start();

  /**
   * @brief Stop watching. The lock is dropped without reporting it, the owner is
   * expected to reset its own state.
   */
  void stop();

  /**
   * @brief Check the foreground window now and report a change of the lock.
   */
  void check();

  bool isLocked() const;

private:
  void setLocked(bool locked, const ForegroundWindow &window);

  IEventQueue *m_events = nullptr;
  GameLockPolicy m_policy;
  Deps m_deps;
  Changed m_changed;
  EventQueueTimer *m_timer = nullptr;
  bool m_running = false;
  bool m_locked = false;
  std::uintptr_t m_lockedWindow = 0;
  int m_failedChecks = 0;
};

} // namespace deskflow
