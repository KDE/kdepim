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


#ifndef SEARCHPOTENTIALDUPLICATECONTACTJOB_H
#define SEARCHPOTENTIALDUPLICATECONTACTJOB_H

#include <QObject>
#include <Akonadi/Item>

#include "kaddressbook_export.h"

namespace KABMergeContacts {
class KADDRESSBOOK_EXPORT SearchPotentialDuplicateContactJob : public QObject
{
    Q_OBJECT
public:
    explicit SearchPotentialDuplicateContactJob(const Akonadi::Item::List &list, QObject *parent = 0);
    ~SearchPotentialDuplicateContactJob();

    void start();

    QList<QList<Akonadi::Item> > potentialDuplicateContacts() const;

Q_SIGNALS:
    void finished(const QList<QList<Akonadi::Item> >&);

private:
    QList<Akonadi::Item> checkList(const QList<Akonadi::Item> &lstItem);
    bool isDuplicate(const Akonadi::Item &itemA, const Akonadi::Item &itemB);
    Akonadi::Item::List mListItem;
    QList<QList<Akonadi::Item> > mListDuplicate;
};
}

#endif // SEARCHPOTENTIALDUPLICATECONTACTJOB_H
