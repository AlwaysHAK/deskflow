/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "GameLockWidget.h"

#include "AppListWidget.h"
#include "common/GameLockPolicy.h"
#include "common/Settings.h"

#include <QCheckBox>
#include <QEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

using deskflow::GameLockPolicy;

namespace {

QStringList sorted(QStringList list)
{
  list.sort();
  return list;
}

QStringList storedApps(const QString &key)
{
  return sorted(GameLockPolicy::normalizeExecutables(Settings::value(key).toStringList()));
}

} // namespace

GameLockWidget::GameLockWidget(QWidget *parent)
    : QWidget(parent),
      m_groupGameLock{new QGroupBox(this)},
      m_lblMode{new QLabel(this)},
      m_rbAny{new QRadioButton(this)},
      m_rbSelected{new QRadioButton(this)},
      m_lblApps{new QLabel(this)},
      m_listApps{new AppListWidget(this)},
      m_cbWhenWindowed{new QCheckBox(this)},
      m_lblIgnore{new QLabel(this)},
      m_listIgnore{new AppListWidget(this)}
{
  m_groupGameLock->setObjectName(QStringLiteral("groupGameLock"));
  m_rbAny->setObjectName(QStringLiteral("rbAny"));
  m_rbSelected->setObjectName(QStringLiteral("rbSelected"));
  m_listApps->setObjectName(QStringLiteral("listApps"));
  m_cbWhenWindowed->setObjectName(QStringLiteral("cbWhenWindowed"));
  m_listIgnore->setObjectName(QStringLiteral("listIgnore"));

  m_groupGameLock->setCheckable(true);
  m_lblApps->setBuddy(m_listApps);
  m_lblIgnore->setBuddy(m_listIgnore);

  updateText();

  auto modeButtons = new QVBoxLayout;
  modeButtons->addWidget(m_rbAny);
  modeButtons->addWidget(m_rbSelected);
  auto modeLayout = new QFormLayout;
  modeLayout->addRow(m_lblMode, modeButtons);

  auto groupLayout = new QVBoxLayout;
  groupLayout->addLayout(modeLayout);
  groupLayout->addWidget(m_lblApps);
  groupLayout->addWidget(m_listApps);
  groupLayout->addWidget(m_cbWhenWindowed);
  groupLayout->addWidget(m_lblIgnore);
  groupLayout->addWidget(m_listIgnore);
  m_groupGameLock->setLayout(groupLayout);

  auto layout = new QVBoxLayout;
  layout->addWidget(m_groupGameLock);
  layout->addStretch();
  setLayout(layout);

  connect(m_groupGameLock, &QGroupBox::toggled, this, &GameLockWidget::updateControls);
  connect(m_rbSelected, &QRadioButton::toggled, this, &GameLockWidget::updateControls);

  connect(m_groupGameLock, &QGroupBox::toggled, this, &GameLockWidget::changed);
  connect(m_rbSelected, &QRadioButton::toggled, this, &GameLockWidget::changed);
  connect(m_cbWhenWindowed, &QCheckBox::toggled, this, &GameLockWidget::changed);
  connect(m_listApps, &AppListWidget::changed, this, &GameLockWidget::changed);
  connect(m_listIgnore, &AppListWidget::changed, this, &GameLockWidget::changed);

  load();
}

void GameLockWidget::load()
{
  m_groupGameLock->setChecked(Settings::value(Settings::GameLock::Enabled).toBool());
  setMode(Settings::value(Settings::GameLock::Mode).toString());
  m_listApps->setApps(Settings::value(Settings::GameLock::Apps).toStringList());
  m_listIgnore->setApps(Settings::value(Settings::GameLock::Ignore).toStringList());
  m_cbWhenWindowed->setChecked(Settings::value(Settings::GameLock::WhenWindowed).toBool());
  updateControls();
}

void GameLockWidget::save() const
{
  // empty lists are left out of the settings file
  const auto listValue = [](const QStringList &list) { return list.isEmpty() ? QVariant() : QVariant(list); };

  Settings::setValue(Settings::GameLock::Enabled, m_groupGameLock->isChecked());
  Settings::setValue(Settings::GameLock::Mode, mode());
  Settings::setValue(Settings::GameLock::Apps, listValue(m_listApps->apps()));
  Settings::setValue(Settings::GameLock::Ignore, listValue(m_listIgnore->apps()));
  Settings::setValue(Settings::GameLock::WhenWindowed, m_cbWhenWindowed->isChecked());
}

void GameLockWidget::resetToDefault()
{
  m_groupGameLock->setChecked(Settings::defaultValue(Settings::GameLock::Enabled).toBool());
  setMode(Settings::defaultValue(Settings::GameLock::Mode).toString());
  m_listApps->setApps({});
  m_listIgnore->setApps({});
  m_cbWhenWindowed->setChecked(Settings::defaultValue(Settings::GameLock::WhenWindowed).toBool());
  updateControls();
}

bool GameLockWidget::isModified() const
{
  const auto storedMode = GameLockPolicy::modeToString(
      GameLockPolicy::modeFromString(Settings::value(Settings::GameLock::Mode).toString())
  );

  return (m_groupGameLock->isChecked() != Settings::value(Settings::GameLock::Enabled).toBool()) ||
         (mode() != storedMode) || (sorted(m_listApps->apps()) != storedApps(Settings::GameLock::Apps)) ||
         (sorted(m_listIgnore->apps()) != storedApps(Settings::GameLock::Ignore)) ||
         (m_cbWhenWindowed->isChecked() != Settings::value(Settings::GameLock::WhenWindowed).toBool());
}

bool GameLockWidget::isDefault() const
{
  return (m_groupGameLock->isChecked() == Settings::defaultValue(Settings::GameLock::Enabled).toBool()) &&
         (mode() == Settings::defaultValue(Settings::GameLock::Mode).toString()) && m_listApps->apps().isEmpty() &&
         m_listIgnore->apps().isEmpty() &&
         (m_cbWhenWindowed->isChecked() == Settings::defaultValue(Settings::GameLock::WhenWindowed).toBool());
}

void GameLockWidget::setWritable(bool writable)
{
  m_writable = writable;
  updateControls();
}

void GameLockWidget::changeEvent(QEvent *e)
{
  QWidget::changeEvent(e);
  if (e->type() == QEvent::LanguageChange)
    updateText();
}

void GameLockWidget::updateText()
{
  m_groupGameLock->setTitle(tr("Keep the cursor on this computer while a game runs"));
  m_groupGameLock->setToolTip(
      tr("Pauses switching at the screen edges while a game runs on this computer, so hitting an edge in the "
         "middle of a game does not move the cursor to another computer.\n"
         "Set this up on the computer that runs the game, whether it is the server or a client. "
         "The switch hotkeys and the lock cursor hotkey keep working while the cursor is held here.")
  );

  m_lblMode->setText(tr("Lock while:"));
  m_rbAny->setText(tr("Any application runs fullscreen"));
  m_rbAny->setToolTip(
      tr("Lock while the application in front covers its whole monitor: fullscreen and borderless games, "
         "fullscreen video, presentations. Maximized windows do not count.")
  );
  m_rbSelected->setText(tr("One of the selected applications runs fullscreen"));
  m_rbSelected->setToolTip(tr("Lock only while one of the selected applications is in front"));

  m_lblApps->setText(tr("Selected applications:"));
  m_cbWhenWindowed->setText(tr("Lock for the selected applications even when they are not fullscreen"));
  m_cbWhenWindowed->setToolTip(tr("For games that run in a normal window"));

  m_lblIgnore->setText(tr("Never lock for:"));
  m_lblIgnore->setToolTip(
      tr("These applications never hold the cursor, even fullscreen, e.g. a browser you watch videos in. "
         "This list wins over the selected applications.")
  );
  m_listIgnore->setToolTip(m_lblIgnore->toolTip());
}

void GameLockWidget::updateControls()
{
  // the group box enables its children when it is checked, so take that into account here
  const bool selectedMode = m_groupGameLock->isChecked() && m_rbSelected->isChecked();

  m_groupGameLock->setEnabled(m_writable);
  m_lblApps->setEnabled(selectedMode);
  m_listApps->setEnabled(selectedMode);
  m_cbWhenWindowed->setEnabled(selectedMode);
}

QString GameLockWidget::mode() const
{
  using enum GameLockPolicy::Mode;
  return GameLockPolicy::modeToString(m_rbSelected->isChecked() ? SelectedApps : AnyFullscreen);
}

void GameLockWidget::setMode(const QString &mode)
{
  const bool selected = GameLockPolicy::modeFromString(mode) == GameLockPolicy::Mode::SelectedApps;
  m_rbSelected->setChecked(selected);
  m_rbAny->setChecked(!selected);
}
