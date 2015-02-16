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

#ifndef MERGECONTACTSELECTINFORMATIONSCROLLAREA_H
#define MERGECONTACTSELECTINFORMATIONSCROLLAREA_H

#include <QWidget>
#include "kaddressbook_export.h"
#include "merge/job/mergecontacts.h"
class QStackedWidget;
namespace KABMergeContacts
{
class MergeContactSelectInformationWidget;
class MergeContactInfoWidget;
class KADDRESSBOOK_EXPORT MergeContactSelectInformationScrollArea : public QWidget
{
    Q_OBJECT
public:
    explicit MergeContactSelectInformationScrollArea(QWidget *parent = Q_NULLPTR);
    ~MergeContactSelectInformationScrollArea();
    void setContacts(MergeContacts::ConflictInformations conflictTypes, const Akonadi::Item::List &listItem);
    void setCollection(const Akonadi::Collection &col);
private slots:
    void slotMergeContacts();
    void slotMergeDone(const Akonadi::Item &item);
private:
    Akonadi::Collection mCollection;
    Akonadi::Item::List mListItem;
    MergeContactSelectInformationWidget *mSelectInformationWidget;
    QStackedWidget *mStackWidget;
    MergeContactInfoWidget *mMergedContactWidget;
};
}

#endif // MERGECONTACTSELECTINFORMATIONSCROLLAREA_H
