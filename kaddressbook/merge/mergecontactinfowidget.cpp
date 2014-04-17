/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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
#include "mergecontactinfowidget.h"

#include "kaddressbookgrantlee/widget/grantleecontactviewer.h"

#include <KLocalizedString>

#include <Akonadi/Item>

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>

using namespace KABMergeContacts;

MergeContactInfoWidget::MergeContactInfoWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mStackWidget = new QStackedWidget;
    mStackWidget->setObjectName(QLatin1String("stackedwidget"));

    mContactViewer = new KAddressBookGrantlee::GrantleeContactViewer;
    mContactViewer->setObjectName(QLatin1String("contactwidget"));

    mStackWidget->addWidget(mContactViewer);

    mNoContactSelected = new QLabel;
    mNoContactSelected->setObjectName(QLatin1String("nocontact"));
    mStackWidget->addWidget(mNoContactSelected);

    mTooManyContactSelected = new QLabel(i18n("No infos for severals contacts selected"));
    mTooManyContactSelected->setObjectName(QLatin1String("toomanycontacts"));
    mStackWidget->addWidget(mTooManyContactSelected);

    lay->addWidget(mStackWidget);
    setLayout(lay);
    mStackWidget->setCurrentWidget(mNoContactSelected);
}


MergeContactInfoWidget::~MergeContactInfoWidget()
{

}

void MergeContactInfoWidget::setContact(const Akonadi::Item::List &items)
{
    const int numberContact(items.count());
    if (numberContact == 1) {
        mContactViewer->setContact(items.first());
        mStackWidget->setCurrentWidget(mContactViewer);
    } else if (numberContact == 0) {
        mStackWidget->setCurrentWidget(mNoContactSelected);
    } else {
        mStackWidget->setCurrentWidget(mTooManyContactSelected);
    }
}
