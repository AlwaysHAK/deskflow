/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "common/GameLockPolicy.h"

#include "common/Settings.h"

namespace deskflow {

namespace {
const auto kModeAny = QStringLiteral("any");
const auto kModeSelected = QStringLiteral("selected");
const auto kExeSuffix = QStringLiteral(".exe");
} // namespace

GameLockPolicy GameLockPolicy::fromSettings()
{
  return fromValues(
      Settings::value(Settings::GameLock::Enabled).toBool(), Settings::value(Settings::GameLock::Mode).toString(),
      Settings::value(Settings::GameLock::Apps).toStringList(),
      Settings::value(Settings::GameLock::Ignore).toStringList(),
      Settings::value(Settings::GameLock::WhenWindowed).toBool()
  );
}

GameLockPolicy GameLockPolicy::fromValues(
    bool enabled, const QString &mode, const QStringList &apps, const QStringList &ignored, bool lockWhenWindowed
)
{
  GameLockPolicy policy;
  policy.m_enabled = enabled;
  policy.m_mode = modeFromString(mode);
  policy.m_apps = normalizeExecutables(apps);
  policy.m_ignored = normalizeExecutables(ignored);
  policy.m_lockWhenWindowed = lockWhenWindowed;
  return policy;
}

GameLockPolicy::Mode GameLockPolicy::modeFromString(const QString &mode)
{
  if (mode.trimmed().compare(kModeSelected, Qt::CaseInsensitive) == 0)
    return Mode::SelectedApps;
  return Mode::AnyFullscreen;
}

QString GameLockPolicy::modeToString(Mode mode)
{
  return mode == Mode::SelectedApps ? kModeSelected : kModeAny;
}

QString GameLockPolicy::normalizeExecutable(const QString &executable)
{
  QString name = executable.trimmed();

  // tolerate names pasted with quotes, e.g. a path copied from explorer
  while (name.startsWith(QLatin1Char('"')))
    name.removeFirst();
  while (name.endsWith(QLatin1Char('"')))
    name.removeLast();

  const auto separator = std::max(name.lastIndexOf(QLatin1Char('\\')), name.lastIndexOf(QLatin1Char('/')));
  name = name.mid(separator + 1).trimmed().toLower();

  if (!name.isEmpty() && !name.contains(QLatin1Char('.')))
    name.append(kExeSuffix);

  return name;
}

QStringList GameLockPolicy::normalizeExecutables(const QStringList &executables)
{
  QStringList result;
  for (const auto &entry : executables) {
    for (const auto &part : entry.split(QLatin1Char(','))) {
      if (const auto name = normalizeExecutable(part); !name.isEmpty() && !result.contains(name))
        result.append(name);
    }
  }
  return result;
}

bool GameLockPolicy::isEnabled() const
{
  return m_enabled;
}

GameLockPolicy::Mode GameLockPolicy::mode() const
{
  return m_mode;
}

const QStringList &GameLockPolicy::apps() const
{
  return m_apps;
}

const QStringList &GameLockPolicy::ignored() const
{
  return m_ignored;
}

bool GameLockPolicy::lockWhenWindowed() const
{
  return m_lockWhenWindowed;
}

bool GameLockPolicy::shouldLock(const ForegroundWindow &window) const
{
  if (!m_enabled || !window.present)
    return false;

  const auto executable = normalizeExecutable(QString::fromStdString(window.executable));
  const bool known = !executable.isEmpty();

  if (known && m_ignored.contains(executable))
    return false;

  if (m_mode == Mode::AnyFullscreen)
    return window.fullscreen;

  if (!known || !m_apps.contains(executable))
    return false;

  return window.fullscreen || m_lockWhenWindowed;
}

} // namespace deskflow
