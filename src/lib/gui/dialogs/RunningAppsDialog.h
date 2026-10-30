/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "gui/RunningApps.h"

#include <QDialog>

#include <functional>

class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;

/**
 * @brief Lets the user pick applications with a visible window, so a game can be
 * chosen without typing its executable name.
 */
class RunningAppsDialog : public QDialog
{
  Q_OBJECT

public:
  using Source = std::function<QList<deskflow::gui::RunningApp>()>;

  explicit RunningAppsDialog(QWidget *parent = nullptr, const Source &source = deskflow::gui::runningApps);
  ~RunningAppsDialog() override = default;

  /**
   * @brief Executable names of the selected applications.
   */
  QStringList selectedExecutables() const;

protected:
  void changeEvent(QEvent *e) override;

private:
  void updateText();
  void refresh();
  void applyFilter(const QString &text);
  void updateButtons();

  Source m_source;
  QLabel *m_lblHint = nullptr;
  QLineEdit *m_lineFilter = nullptr;
  QTreeWidget *m_tree = nullptr;
  QPushButton *m_btnRefresh = nullptr;
  QDialogButtonBox *m_buttonBox = nullptr;
};
