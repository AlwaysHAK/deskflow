/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <QList>
#include <QString>

namespace deskflow::gui {

/**
 * @brief An application with a visible window, as offered by the game lock app picker.
 */
struct RunningApp
{
  /// lower case executable file name, e.g. "eldenring.exe"
  QString executable;
  /// title of the application's main window
  QString title;
  /// full path of the executable, used for its icon
  QString path;
};

/**
 * @brief Applications with a visible top-level window, one entry per executable,
 * sorted by window title. Empty on platforms without game lock.
 */
QList<RunningApp> runningApps();

} // namespace deskflow::gui
