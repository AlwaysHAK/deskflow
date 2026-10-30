/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <QWidget>

class AppListWidget;
class QCheckBox;
class QGroupBox;
class QLabel;
class QRadioButton;

/**
 * @brief Game lock settings of this computer: keep the cursor here while a game runs.
 */
class GameLockWidget : public QWidget
{
  Q_OBJECT

public:
  explicit GameLockWidget(QWidget *parent = nullptr);
  ~GameLockWidget() override = default;

  void load();
  void save() const;
  void resetToDefault();
  bool isModified() const;
  bool isDefault() const;
  void setWritable(bool writable);

Q_SIGNALS:
  void changed();

protected:
  void changeEvent(QEvent *e) override;

private:
  void updateText();
  void updateControls();
  QString mode() const;
  void setMode(const QString &mode);

  bool m_writable = true;
  QGroupBox *m_groupGameLock = nullptr;
  QLabel *m_lblMode = nullptr;
  QRadioButton *m_rbAny = nullptr;
  QRadioButton *m_rbSelected = nullptr;
  QLabel *m_lblApps = nullptr;
  AppListWidget *m_listApps = nullptr;
  QCheckBox *m_cbWhenWindowed = nullptr;
  QLabel *m_lblIgnore = nullptr;
  AppListWidget *m_listIgnore = nullptr;
};
