/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

PotentialDuplicateContactsWidget::PotentialDuplicateContactsWidget(QWidget *parent)
    : QWidget(parent)
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
    }
}

void PotentialDuplicateContactsWidget::slotSearchDuplicateContactFinished(SearchPotentialDuplicateContactJob *job)
{
    //TODO
}
