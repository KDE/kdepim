/*
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "contactswitcher.h"

#include <KLocalizedString>

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ContactSwitcher::ContactSwitcher(QWidget *parent)
    : QWidget(parent), mView(Q_NULLPTR)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    mPreviousButton = new QPushButton(i18nc("@action:button Previous contact", "Previous"));
    mPreviousButton->setToolTip(
        i18nc("@info:tooltip", "Move to the previous contact in the list"));
    mPreviousButton->setWhatsThis(
        i18nc("@info:whatsthis",
              "Press this button to move to the previous contact in the list."));

    mNextButton = new QPushButton(i18nc("@action:button Next contact", "Next"));
    mNextButton->setToolTip(
        i18nc("@info:tooltip", "Move to the next contact in the list"));
    mNextButton->setWhatsThis(
        i18nc("@info:whatsthis",
              "Press this button to move to the next contact in the list."));

    mStatusLabel = new QLabel();

    layout->addWidget(mPreviousButton);
    layout->addWidget(mNextButton);
    layout->addStretch(1);
    layout->addWidget(mStatusLabel);

    connect(mPreviousButton, &QPushButton::clicked, this, &ContactSwitcher::previousClicked);
    connect(mNextButton, &QPushButton::clicked, this, &ContactSwitcher::nextClicked);
}

void ContactSwitcher::setView(QAbstractItemView *view)
{
    mView = view;

    Q_ASSERT_X(mView->model(), "ContactSwitcher::setView", "The view has no model set!");

    connect(mView->model(), &QAbstractItemModel::layoutChanged, this, &ContactSwitcher::updateStatus);
    connect(mView->model(), &QAbstractItemModel::rowsInserted, this, &ContactSwitcher::updateStatus);
    connect(mView->model(), &QAbstractItemModel::rowsRemoved, this, &ContactSwitcher::updateStatus);
    connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContactSwitcher::updateStatus);

    updateStatus();
}

void ContactSwitcher::nextClicked()
{
    if (!mView || !mView->model()) {
        return;
    }

    const QModelIndex index = mView->selectionModel()->currentIndex();

    int row = 0;
    if (index.isValid()) {
        row = index.row() + 1;
    }

    mView->selectionModel()->setCurrentIndex(mView->model()->index(row, 0),
            QItemSelectionModel::Rows |
            QItemSelectionModel::ClearAndSelect);

    updateStatus();
}

void ContactSwitcher::previousClicked()
{
    if (!mView || !mView->model()) {
        return;
    }

    const QModelIndex index = mView->selectionModel()->currentIndex();

    int row = 0;
    if (index.isValid()) {
        row = index.row() - 1;
    }

    mView->selectionModel()->setCurrentIndex(mView->model()->index(row, 0),
            QItemSelectionModel::Rows |
            QItemSelectionModel::ClearAndSelect);

    updateStatus();
}

void ContactSwitcher::updateStatus()
{
    if (!mView || !mView->model()) {
        return;
    }

    const QModelIndex index = mView->selectionModel()->currentIndex();

    int row = 0;
    if (index.isValid()) {
        row = index.row();
    }

    mPreviousButton->setEnabled(row != 0);
    mNextButton->setEnabled(row != (mView->model()->rowCount() - 1));

    mStatusLabel->setText(i18nc("@info:status",
                                "%1 out of %2", row + 1, mView->model()->rowCount()));
}

