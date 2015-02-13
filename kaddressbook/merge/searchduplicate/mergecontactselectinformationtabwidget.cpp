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

void MergeContactSelectInformationTabWidget::addNeedSelectInformationWidget(const Akonadi::Item::List &list, bool needUpdateTabWidget)
{
    if (!list.isEmpty()) {
        addNewWidget(list);
        if (needUpdateTabWidget)
            updateTabWidget();
    }
}

void MergeContactSelectInformationTabWidget::addNewWidget(const Akonadi::Item::List &list)
{
    KABMergeContacts::MergeContactSelectInformationScrollArea *area = new KABMergeContacts::MergeContactSelectInformationScrollArea;
    //TODO fixme
    //area->setContacts();
    //FIXME add text
    addTab(area, QString());
}

void MergeContactSelectInformationTabWidget::setNeedSelectInformationWidgets(const QList<Akonadi::Item::List> &list)
{
    Q_FOREACH(const Akonadi::Item::List &lst, list) {
        addNewWidget(lst);
    }
    updateTabWidget();
}
