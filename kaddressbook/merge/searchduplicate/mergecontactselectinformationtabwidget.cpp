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

#include "mergecontactselectinformationtabwidget.h"
#include "merge/widgets/mergecontactselectinformationscrollarea.h"
#include <QTabBar>

using namespace KABMergeContacts;

MergeContactSelectInformationTabWidget::MergeContactSelectInformationTabWidget(QWidget *parent)
    : QTabWidget(parent)
{

}

MergeContactSelectInformationTabWidget::~MergeContactSelectInformationTabWidget()
{

}

void MergeContactSelectInformationTabWidget::updateTabWidget()
{
    tabBar()->setVisible(count()>1);
}

bool MergeContactSelectInformationTabWidget::tabBarVisible() const
{
    return tabBar()->isVisible();
}

void MergeContactSelectInformationTabWidget::addNewWidget(const KABMergeContacts::MergeConflictResult &list, const Akonadi::Collection &col)
{
    KABMergeContacts::MergeContactSelectInformationScrollArea *area = new KABMergeContacts::MergeContactSelectInformationScrollArea;
    area->setContacts(list.conflictInformation, list.list);
    area->setCollection(col);
    //KF5 add i18n
    addTab(area, QString::fromLatin1("Duplicate contact %1").arg(count()+1));
}

void MergeContactSelectInformationTabWidget::setRequiresSelectInformationWidgets(const QList<KABMergeContacts::MergeConflictResult> &list, const Akonadi::Collection &col)
{
    Q_FOREACH(const KABMergeContacts::MergeConflictResult &lst, list) {
        addNewWidget(lst, col);
    }
    updateTabWidget();
}
