/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "searchduplicateresultwidget.h"
#include "resultduplicatetreewidget.h"
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QSplitter>
#include <kaddressbookgrantlee/widget/grantleecontactviewer.h>

using namespace KABMergeContacts;
SearchDuplicateResultWidget::SearchDuplicateResultWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);

    QSplitter *splitter = new QSplitter;
    splitter->setObjectName(QLatin1String("splitter"));
    mainLayout->addWidget(splitter);
    mResult = new ResultDuplicateTreeWidget;
    mResult->setObjectName(QLatin1String("result_treewidget"));
    mContactViewer = new KAddressBookGrantlee::GrantleeContactViewer;
    mContactViewer->setObjectName(QLatin1String("contact_viewer"));
    splitter->addWidget(mResult);
    splitter->addWidget(mContactViewer);
}

SearchDuplicateResultWidget::~SearchDuplicateResultWidget()
{

}

void SearchDuplicateResultWidget::setContacts(const QList<Akonadi::Item::List> &lstItem)
{

}

