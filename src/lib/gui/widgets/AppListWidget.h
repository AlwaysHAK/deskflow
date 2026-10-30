/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <QStringList>
#include <QWidget>

class QListWidget;
class QPushButton;

/**
 * @brief An editable list of executable names, filled from running applications or
 * typed by hand. Entries are kept normalized (lower case file names) and unique.
 */
class AppListWidget : public QWidget
{
  Q_OBJECT

public:
  explicit AppListWidget(QWidget *parent = nullptr);
  ~AppListWidget() override = default;

  QStringList apps() const;
  void setApps(const QStringList &apps);

  /**
   * @brief Add \p apps after normalizing them, skipping entries already listed.
   */
  void addApps(const QStringList &apps);

Q_SIGNALS:
  void changed();

protected:
  void changeEvent(QEvent *e) override;

private:
  void updateText();
  void updateButtons();
  void addRunningApp();
  void addAppByName();
  void removeSelected();

  QListWidget *m_list = nullptr;
  QPushButton *m_btnAddRunning = nullptr;
  QPushButton *m_btnAddByName = nullptr;
  QPushButton *m_btnRemove = nullptr;
};
