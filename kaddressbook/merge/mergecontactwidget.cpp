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

#include "mergecontactwidget.h"

#include <KLocalizedString>

#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>

MergeContactWidget::MergeContactWidget(const Akonadi::Item::List &items, QWidget *parent)
    : QWidget(parent),
      mItems(items)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mListWidget = new QListWidget;
    mListWidget->setObjectName(QLatin1String("listcontact"));
    lay->addWidget(mListWidget);
    connect(mListWidget, SIGNAL(itemSelectionChanged()), SLOT(slotUpdateMergeButton()));

    mMergeButton = new QPushButton(i18n("merge"));
    mMergeButton->setObjectName(QLatin1String("mergebutton"));
    lay->addWidget(mMergeButton);
    mMergeButton->setEnabled(false);

    setLayout(lay);
}

MergeContactWidget::~MergeContactWidget()
{

}

void MergeContactWidget::slotUpdateMergeButton()
{
    mMergeButton->setEnabled(!mListWidget->selectedItems().isEmpty());
}
