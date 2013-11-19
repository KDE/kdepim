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


#include "searchpotentialduplicatecontactjob.h"

#include <KABC/Addressee>

SearchPotentialDuplicateContactJob::SearchPotentialDuplicateContactJob(const Akonadi::Item::List &list, QObject *parent)
    : QObject(parent),
      mListItem(list)
{
}

SearchPotentialDuplicateContactJob::~SearchPotentialDuplicateContactJob()
{

}

void SearchPotentialDuplicateContactJob::start()
{
    QList<Akonadi::Item> result = mListItem;
    while(!result.isEmpty()) {
        //qDebug()<<" loop";
        result = checkList(result);
    }
    Q_EMIT finished(this);
}

QList<QList<Akonadi::Item> > SearchPotentialDuplicateContactJob::potentialDuplicateContacts() const
{
    return mListDuplicate;
}

QList<Akonadi::Item> SearchPotentialDuplicateContactJob::checkList(const QList<Akonadi::Item> &lstItem)
{
    QList<Akonadi::Item> notDuplicate;
    QList<Akonadi::Item> lst;
    if (!lstItem.isEmpty()) {
        Akonadi::Item firstItem = lstItem.at(0);
        for (int j = 1; j < lstItem.count(); ++j) {
            if (isDuplicate(firstItem, mListItem.at(j))) {
                lst.append(lstItem.at(j));
            } else {
                notDuplicate.append(lstItem.at(j));
            }
        }
        if (!lst.isEmpty()) {
            lst.append(firstItem);
            mListDuplicate.append(lst);
        }

        //qDebug()<<"not duplicate number"<<notDuplicate.count();
        //qDebug()<<" duplicate number "<<lst.count();
    }
    //qDebug()<<" notDuplicate.count"<<notDuplicate.count();
    return notDuplicate;
}

bool SearchPotentialDuplicateContactJob::isDuplicate(const Akonadi::Item &itemA, const Akonadi::Item &itemB)
{
    KABC::Addressee addressA = itemA.payload<KABC::Addressee>();
    KABC::Addressee addressB = itemB.payload<KABC::Addressee>();
    //
    if (!addressA.name().isEmpty() && !addressB.name().isEmpty()) {
        //qDebug()<<" addressB"<<addressB.name()<<" addressA.name()"<<addressA.name();
        if (addressA.name() == addressB.name()) {
            //qDebug()<<" return true;";
            return true;
        }
    }
    if (!addressA.nickName().isEmpty() && !addressB.nickName().isEmpty()) {
        if (addressA.nickName() == addressB.nickName()) {
            return true;
        }
    }
    return false;

}
