/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "RunningApps.h"

#include "common/GameLockPolicy.h"

#include <QDir>

#include <algorithm>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dwmapi.h>

#include <string>
#endif

namespace deskflow::gui {

#ifdef Q_OS_WIN
namespace {

QString processPath(DWORD pid)
{
  HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if (process == nullptr)
    return {};

  std::wstring path(32768, L'\0');
  auto size = static_cast<DWORD>(path.size());
  const bool queried = QueryFullProcessImageNameW(process, 0, path.data(), &size);
  CloseHandle(process);
  if (!queried)
    return {};

  return QDir::fromNativeSeparators(QString::fromWCharArray(path.data(), static_cast<qsizetype>(size)));
}

// a window the user can switch to: visible, not owned by another window, not a tool window
// and not cloaked (suspended store apps, windows on other virtual desktops)
bool isAppWindow(HWND window)
{
  if (!IsWindowVisible(window) || GetWindow(window, GW_OWNER) != nullptr || GetWindowTextLengthW(window) == 0)
    return false;

  if ((GetWindowLongPtrW(window, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) != 0)
    return false;

  DWORD cloaked = 0;
  return FAILED(DwmGetWindowAttribute(window, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) || cloaked == 0;
}

BOOL CALLBACK collectWindow(HWND window, LPARAM param)
{
  auto &apps = *reinterpret_cast<QList<RunningApp> *>(param);
  if (!isAppWindow(window))
    return TRUE;

  DWORD pid = 0;
  GetWindowThreadProcessId(window, &pid);
  if (pid == GetCurrentProcessId())
    return TRUE;

  // processes that refuse the query can't be matched by name, so they are not offered
  const auto path = processPath(pid);
  const auto executable = GameLockPolicy::normalizeExecutable(path);
  if (executable.isEmpty() ||
      std::ranges::any_of(apps, [&executable](const RunningApp &app) { return app.executable == executable; }))
    return TRUE;

  std::wstring title(static_cast<size_t>(GetWindowTextLengthW(window)) + 1, L'\0');
  const int length = GetWindowTextW(window, title.data(), static_cast<int>(title.size()));
  apps.append({executable, QString::fromWCharArray(title.data(), length), path});
  return TRUE;
}

} // namespace
#endif

QList<RunningApp> runningApps()
{
  QList<RunningApp> apps;
#ifdef Q_OS_WIN
  EnumWindows(collectWindow, reinterpret_cast<LPARAM>(&apps));
  std::ranges::sort(apps, [](const RunningApp &a, const RunningApp &b) {
    return a.title.compare(b.title, Qt::CaseInsensitive) < 0;
  });
#endif
  return apps;
}

} // namespace deskflow::gui
