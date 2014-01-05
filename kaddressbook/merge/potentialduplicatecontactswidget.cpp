/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "potentialduplicatecontactswidget.h"
#include "searchpotentialduplicatecontactjob.h"
#include <KABC/Addressee>

#include <KLocalizedString>

#include <QVBoxLayout>
#include <QLabel>

PotentialDuplicateContactsWidget::PotentialDuplicateContactsWidget(QWidget *parent)
    : QWidget(parent),
      mMainLayout(0)
{
}

PotentialDuplicateContactsWidget::~PotentialDuplicateContactsWidget()
{

}

void PotentialDuplicateContactsWidget::setAddressList(const Akonadi::Item::List &list)
{
    //Clear widget
    searchDuplicateContact(list);
}

void PotentialDuplicateContactsWidget::searchDuplicateContact(const Akonadi::Item::List &list)
{
    if (list.count() > 1) {
        SearchPotentialDuplicateContactJob *job = new SearchPotentialDuplicateContactJob(list, this);
        connect(job, SIGNAL(finished(SearchPotentialDuplicateContactJob*)), this, SLOT(slotSearchDuplicateContactFinished(SearchPotentialDuplicateContactJob*)));
        job->start();
    } else {
        createEmptyWidget();
    }
}

void PotentialDuplicateContactsWidget::slotSearchDuplicateContactFinished(SearchPotentialDuplicateContactJob *job)
{
    QList<QList<Akonadi::Item> > list = job->potentialDuplicateContacts();
    if (list.isEmpty()) {
        createEmptyWidget();
    } else {
        createWidgets(list);
    }
}

void PotentialDuplicateContactsWidget::createWidgets(const QList<QList<Akonadi::Item> > &lst)
{
    Q_FOREACH (const QList<Akonadi::Item> &itemLst, lst) {
        createDuplicateWidget(itemLst);
    }
}

void PotentialDuplicateContactsWidget::createDuplicateWidget(const QList<Akonadi::Item> &item)
{
    //TODO
}

void PotentialDuplicateContactsWidget::createEmptyWidget()
{
    delete mMainLayout;
    mMainLayout = new QVBoxLayout;
    QLabel *lab = new QLabel(i18n("No duplicate contact found."));
    mMainLayout->addWidget(lab);
    setLayout(mMainLayout);
}
