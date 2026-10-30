/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "AppListWidget.h"

#include "common/GameLockPolicy.h"
#include "gui/dialogs/RunningAppsDialog.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

using deskflow::GameLockPolicy;

AppListWidget::AppListWidget(QWidget *parent)
    : QWidget(parent),
      m_list{new QListWidget(this)},
      m_btnAddRunning{new QPushButton(this)},
      m_btnAddByName{new QPushButton(this)},
      m_btnRemove{new QPushButton(this)}
{
  m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_list->setSortingEnabled(true);
  m_list->setMaximumHeight(m_list->fontMetrics().height() * 4 + 2 * m_list->frameWidth());

  m_btnAddRunning->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd));
  m_btnRemove->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove));
  m_btnRemove->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

  updateText();

  auto secondRow = new QHBoxLayout;
  secondRow->addWidget(m_btnAddByName);
  secondRow->addWidget(m_btnRemove);

  auto buttonLayout = new QVBoxLayout;
  buttonLayout->addWidget(m_btnAddRunning);
  buttonLayout->addLayout(secondRow);
  buttonLayout->addStretch();

  auto layout = new QHBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_list);
  layout->addLayout(buttonLayout);
  setLayout(layout);

  connect(m_list, &QListWidget::itemSelectionChanged, this, &AppListWidget::updateButtons);
  connect(m_btnAddRunning, &QPushButton::clicked, this, &AppListWidget::addRunningApp);
  connect(m_btnAddByName, &QPushButton::clicked, this, &AppListWidget::addAppByName);
  connect(m_btnRemove, &QPushButton::clicked, this, &AppListWidget::removeSelected);

  updateButtons();
}

QStringList AppListWidget::apps() const
{
  QStringList apps;
  for (int i = 0; i < m_list->count(); ++i)
    apps.append(m_list->item(i)->text());
  return apps;
}

void AppListWidget::setApps(const QStringList &apps)
{
  m_list->clear();
  m_list->addItems(GameLockPolicy::normalizeExecutables(apps));
  updateButtons();
  Q_EMIT changed();
}

void AppListWidget::addApps(const QStringList &apps)
{
  const auto current = this->apps();
  bool added = false;
  for (const auto &app : GameLockPolicy::normalizeExecutables(apps)) {
    if (current.contains(app))
      continue;
    m_list->addItem(app);
    added = true;
  }

  if (added) {
    updateButtons();
    Q_EMIT changed();
  }
}

void AppListWidget::changeEvent(QEvent *e)
{
  QWidget::changeEvent(e);
  if (e->type() == QEvent::LanguageChange)
    updateText();
}

void AppListWidget::updateText()
{
  m_btnAddRunning->setText(tr("Add running application..."));
  m_btnAddRunning->setToolTip(tr("Pick from the applications that have a window open right now"));
  m_btnAddByName->setText(tr("Add by name..."));
  m_btnAddByName->setToolTip(tr("Type the executable name, e.g. eldenring.exe"));
  m_btnRemove->setAccessibleName(tr("Remove"));
  m_btnRemove->setToolTip(tr("Remove the selected applications"));
}

void AppListWidget::updateButtons()
{
  m_btnRemove->setEnabled(isEnabled() && !m_list->selectedItems().isEmpty());
}

void AppListWidget::addRunningApp()
{
  RunningAppsDialog dialog(this);
  if (dialog.exec() == QDialog::Accepted)
    addApps(dialog.selectedExecutables());
}

void AppListWidget::addAppByName()
{
  bool ok = false;
  const auto name = QInputDialog::getText(
      this, tr("Add application"), tr("Executable name or path (e.g. eldenring.exe):"), QLineEdit::Normal, {}, &ok
  );
  if (ok)
    addApps({name});
}

void AppListWidget::removeSelected()
{
  const auto selected = m_list->selectedItems();
  if (selected.isEmpty())
    return;

  qDeleteAll(selected);
  updateButtons();
  Q_EMIT changed();
}
