/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "RunningAppsDialog.h"

#include <QDialogButtonBox>
#include <QEvent>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {
enum Column
{
  Title,
  Executable
};
} // namespace

RunningAppsDialog::RunningAppsDialog(QWidget *parent, const Source &source)
    : QDialog(parent),
      m_source{source},
      m_lblHint{new QLabel(this)},
      m_lineFilter{new QLineEdit(this)},
      m_tree{new QTreeWidget(this)},
      m_btnRefresh{new QPushButton(this)},
      m_buttonBox{new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this)}
{
  m_lblHint->setWordWrap(true);
  m_lineFilter->setClearButtonEnabled(true);
  m_btnRefresh->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ViewRefresh));

  m_tree->setColumnCount(2);
  m_tree->setRootIsDecorated(false);
  m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_tree->setSortingEnabled(true);
  m_tree->sortByColumn(Title, Qt::AscendingOrder);
  m_tree->header()->setSectionResizeMode(Title, QHeaderView::Stretch);
  m_tree->header()->setSectionResizeMode(Executable, QHeaderView::ResizeToContents);
  m_tree->setMinimumSize(520, 320);

  updateText();

  auto filterLayout = new QHBoxLayout;
  filterLayout->addWidget(m_lineFilter);
  filterLayout->addWidget(m_btnRefresh);

  auto layout = new QVBoxLayout;
  layout->addWidget(m_lblHint);
  layout->addLayout(filterLayout);
  layout->addWidget(m_tree);
  layout->addWidget(m_buttonBox);
  setLayout(layout);

  connect(m_lineFilter, &QLineEdit::textChanged, this, &RunningAppsDialog::applyFilter);
  connect(m_btnRefresh, &QPushButton::clicked, this, &RunningAppsDialog::refresh);
  connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &RunningAppsDialog::updateButtons);
  connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  refresh();
  m_lineFilter->setFocus();
}

QStringList RunningAppsDialog::selectedExecutables() const
{
  QStringList executables;
  for (const auto *item : m_tree->selectedItems())
    executables.append(item->text(Executable));
  return executables;
}

void RunningAppsDialog::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);
  if (e->type() == QEvent::LanguageChange)
    updateText();
}

void RunningAppsDialog::updateText()
{
  setWindowTitle(tr("Add running application"));
  m_lblHint->setText(
      tr("Start the game, then pick it here. If a launcher starts the game, pick the game's own window, "
         "not the launcher.")
  );
  m_lineFilter->setPlaceholderText(tr("Filter by window title or executable"));
  m_btnRefresh->setToolTip(tr("Refresh the list of running applications"));
  m_tree->setHeaderLabels({tr("Window"), tr("Executable")});
}

void RunningAppsDialog::refresh()
{
  m_tree->clear();

  QFileIconProvider iconProvider;
  for (const auto &app : m_source()) {
    auto *item = new QTreeWidgetItem(m_tree);
    item->setText(Title, app.title);
    item->setToolTip(Title, app.path);
    item->setText(Executable, app.executable);
    if (!app.path.isEmpty())
      item->setIcon(Title, iconProvider.icon(QFileInfo(app.path)));
  }

  applyFilter(m_lineFilter->text());
  updateButtons();
}

void RunningAppsDialog::applyFilter(const QString &text)
{
  for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
    auto *item = m_tree->topLevelItem(i);
    const bool matches = text.isEmpty() || item->text(Title).contains(text, Qt::CaseInsensitive) ||
                         item->text(Executable).contains(text, Qt::CaseInsensitive);
    item->setHidden(!matches);
    if (!matches)
      item->setSelected(false);
  }
}

void RunningAppsDialog::updateButtons()
{
  m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!m_tree->selectedItems().isEmpty());
}
