/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filterselectiondialog.h"
#include "mailfilter.h"

#include <KListWidgetSearchLine>
#include <QPushButton>

#include <QListWidget>
#include <QVBoxLayout>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>

using namespace MailCommon;

FilterSelectionDialog::FilterSelectionDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QLatin1String("filterselection"));
    setModal(true);
    setWindowTitle(i18n("Select Filters"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *top = new QVBoxLayout;
    setLayout(top);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FilterSelectionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FilterSelectionDialog::reject);
    mOkButton->setDefault(true);

    filtersListWidget = new QListWidget();
    KListWidgetSearchLine *searchLine = new KListWidgetSearchLine(this, filtersListWidget);
    searchLine->setPlaceholderText(
        i18nc("@info Displayed grayed-out inside the textbox, verb to search",
              "Search"));

    top->addWidget(searchLine);
    top->addWidget(filtersListWidget);
    filtersListWidget->setAlternatingRowColors(true);
    filtersListWidget->setSortingEnabled(false);
    filtersListWidget->setSelectionMode(QAbstractItemView::NoSelection);

    QHBoxLayout *const buttonLayout = new QHBoxLayout();
    top->addLayout(buttonLayout);
    selectAllButton = new QPushButton(i18n("Select All"));
    buttonLayout->addWidget(selectAllButton);
    unselectAllButton = new QPushButton(i18n("Unselect All"));
    buttonLayout->addWidget(unselectAllButton);
    top->addWidget(buttonBox);

    connect(selectAllButton, &QPushButton::clicked, this, &FilterSelectionDialog::slotSelectAllButton);
    connect(unselectAllButton, &QPushButton::clicked, this, &FilterSelectionDialog::slotUnselectAllButton);

    readConfig();
}

FilterSelectionDialog::~FilterSelectionDialog()
{
    writeConfig();
}

void FilterSelectionDialog::reject()
{
    qDeleteAll(originalFilters);
    QDialog::reject();
}

void FilterSelectionDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "FilterSelectionDialog");
    group.writeEntry("Size", size());
}

void FilterSelectionDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "FilterSelectionDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(300, 350));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void FilterSelectionDialog::setFilters(const QList<MailFilter *> &filters)
{
    if (filters.isEmpty()) {
        mOkButton->setEnabled(false);
        return;
    }

    originalFilters = filters;
    filtersListWidget->clear();

    foreach (const MailFilter *filter, filters) {
        QListWidgetItem *item = new QListWidgetItem(filter->name(), filtersListWidget);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(Qt::Checked);
    }
}

QList<MailFilter *> FilterSelectionDialog::selectedFilters() const
{
    QList<MailFilter *> filters;

    const int filterCount = filtersListWidget->count();
    for (int i = 0; i < filterCount; ++i) {
        const QListWidgetItem *item = filtersListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            filters << originalFilters[ i ];
        } else {
            delete originalFilters[ i ];
        }
    }

    return filters;
}

void FilterSelectionDialog::slotUnselectAllButton()
{
    const int filterCount = filtersListWidget->count();
    for (int i = 0; i < filterCount; ++i) {
        QListWidgetItem *const item = filtersListWidget->item(i);
        item->setCheckState(Qt::Unchecked);
    }
}

void FilterSelectionDialog::slotSelectAllButton()
{
    const int filterCount = filtersListWidget->count();
    for (int i = 0; i < filterCount; ++i) {
        QListWidgetItem *const item = filtersListWidget->item(i);
        item->setCheckState(Qt::Checked);
    }
}

