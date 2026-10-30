/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "common/ForegroundWindow.h"

#include <QString>
#include <QStringList>

namespace deskflow {

/**
 * @brief Decides whether the foreground window should hold the cursor on this computer.
 *
 * The policy is read from the settings of the computer that owns the window, because the
 * app lists name programs installed on that computer.
 */
class GameLockPolicy
{
public:
  enum class Mode
  {
    AnyFullscreen,
    SelectedApps
  };

  /**
   * @brief Read the policy from the settings file.
   */
  static GameLockPolicy fromSettings();

  /**
   * @brief Build a policy from raw setting values, normalizing the app lists.
   * @param mode `any` or `selected`, anything else is treated as `any`
   */
  static GameLockPolicy fromValues(
      bool enabled, const QString &mode, const QStringList &apps, const QStringList &ignored, bool lockWhenWindowed
  );

  static Mode modeFromString(const QString &mode);
  static QString modeToString(Mode mode);

  /**
   * @brief Lower case file name of \p executable without its directory.
   * A name without an extension gets `.exe` appended, an empty string stays empty.
   */
  static QString normalizeExecutable(const QString &executable);

  /**
   * @brief Normalize every entry, split comma separated entries, drop empty
   * and duplicate entries and keep the order of the rest.
   */
  static QStringList normalizeExecutables(const QStringList &executables);

  bool isEnabled() const;
  Mode mode() const;
  const QStringList &apps() const;
  const QStringList &ignored() const;
  bool lockWhenWindowed() const;

  /**
   * @brief True iff \p window should hold the cursor on this computer.
   *
   * The ignore list wins over everything. In `any` mode a fullscreen window locks. In
   * `selected` mode the window's executable must be selected, and it must be fullscreen
   * unless lockWhenWindowed is set. A window whose executable is unknown never matches a
   * list entry, so it only locks in `any` mode.
   */
  bool shouldLock(const ForegroundWindow &window) const;

private:
  bool m_enabled = false;
  Mode m_mode = Mode::AnyFullscreen;
  QStringList m_apps;
  QStringList m_ignored;
  bool m_lockWhenWindowed = false;
};

} // namespace deskflow
