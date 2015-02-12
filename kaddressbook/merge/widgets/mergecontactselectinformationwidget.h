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

#ifndef MERGECONTACTSELECTINFORMATIONWIDGET_H
#define MERGECONTACTSELECTINFORMATIONWIDGET_H

#include <QWidget>
#include "kaddressbook_export.h"
#include "merge/job/mergecontacts.h"
#include <AkonadiCore/Item>
namespace KABMergeContacts {
class MergeContactSelectListWidget;
class KADDRESSBOOK_EXPORT MergeContactSelectInformationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MergeContactSelectInformationWidget(QWidget *parent=Q_NULLPTR);
    ~MergeContactSelectInformationWidget();

    void setContacts(KABMergeContacts::MergeContacts::ConflictInformations conflictTypes, const Akonadi::Item::List &listItem);
    KContacts::Addressee createContact();

private:
    void addInformationWidget(MergeContacts::ConflictInformation conflictType, const Akonadi::Item::List &listItem);
    QList<MergeContactSelectListWidget *> mListMergeSelectInformation;
};
}

#endif // MERGECONTACTSELECTINFORMATIONWIDGET_H
