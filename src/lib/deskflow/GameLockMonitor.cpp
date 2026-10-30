/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "deskflow/GameLockMonitor.h"

#include "base/IEventQueue.h"
#include "base/Log.h"
#include "deskflow/IPlatformComputer.h"

namespace deskflow {

GameLockMonitor::Deps GameLockMonitor::depsFor(IPlatformComputer *computer)
{
  return {
      .foregroundWindow = [computer] { return computer->foregroundWindow(); },
      .watchForegroundWindow = [computer](bool watch) { computer->watchForegroundWindow(watch); },
      .eventTarget = computer->getEventTarget()
  };
}

GameLockMonitor::GameLockMonitor(
    IEventQueue *events, const GameLockPolicy &policy, const Deps &deps, const Changed &changed
)
    : m_events(events),
      m_policy(policy),
      m_deps(deps),
      m_changed(changed)
{
  // do nothing
}

GameLockMonitor::~GameLockMonitor()
{
  stop();
}

void GameLockMonitor::start()
{
  if (m_running) {
    return;
  }
  m_running = true;

  const auto mode = GameLockPolicy::modeToString(m_policy.mode());
  LOG_INFO("game lock is on, mode: %s", qPrintable(mode));
  LOG_DEBUG(
      "game lock apps: \"%s\", ignored: \"%s\", lock when windowed: %s", qPrintable(m_policy.apps().join(", ")),
      qPrintable(m_policy.ignored().join(", ")), m_policy.lockWhenWindowed() ? "yes" : "no"
  );

  m_timer = m_events->newTimer(s_pollInterval, nullptr);
  m_events->addHandler(EventTypes::Timer, m_timer, [this](const auto &) { check(); });
  if (m_deps.eventTarget != nullptr) {
    m_events->addHandler(EventTypes::ComputerForegroundChanged, m_deps.eventTarget, [this](const auto &) { check(); });
  }
  m_deps.watchForegroundWindow(true);

  check();
}

void GameLockMonitor::stop()
{
  if (!m_running) {
    return;
  }
  m_running = false;

  m_deps.watchForegroundWindow(false);
  if (m_deps.eventTarget != nullptr) {
    m_events->removeHandler(EventTypes::ComputerForegroundChanged, m_deps.eventTarget);
  }
  m_events->removeHandler(EventTypes::Timer, m_timer);
  m_events->deleteTimer(m_timer);
  m_timer = nullptr;

  m_locked = false;
  m_lockedWindow = 0;
  m_failedChecks = 0;
}

void GameLockMonitor::check()
{
  const auto window = m_deps.foregroundWindow();

  if (m_policy.shouldLock(window)) {
    m_failedChecks = 0;
    m_lockedWindow = window.window;
    if (!m_locked) {
      setLocked(true, window);
    }
    return;
  }

  if (!m_locked) {
    return;
  }

  // the game holding the lock is still in front but does not look fullscreen for a moment,
  // e.g. while it switches display mode, so give it another check before letting go.  a
  // different window in front (alt-tab, the game closed or crashed) releases right away.
  if (window.present && window.window == m_lockedWindow && ++m_failedChecks < s_releaseGraceChecks) {
    return;
  }

  setLocked(false, window);
}

bool GameLockMonitor::isLocked() const
{
  return m_locked;
}

void GameLockMonitor::setLocked(bool locked, const ForegroundWindow &window)
{
  m_locked = locked;
  m_failedChecks = 0;
  if (locked) {
    LOG_INFO("game lock engaged by \"%s\"", window.executable.empty() ? "unnamed app" : window.executable.c_str());
  } else {
    m_lockedWindow = 0;
    LOG_INFO("game lock released");
  }
  m_changed(locked);
}

} // namespace deskflow
