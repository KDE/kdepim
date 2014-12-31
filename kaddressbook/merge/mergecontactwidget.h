/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef MERGECONTACTWIDGET_H
#define MERGECONTACTWIDGET_H

#include <QWidget>
#include <AkonadiCore/Item>
#include "kaddressbook_export.h"

class QPushButton;
namespace Akonadi
{
class CollectionComboBox;
}
namespace KABMergeContacts
{
class MergeContactWidgetList;

class KADDRESSBOOK_EXPORT MergeContactWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MergeContactWidget(const Akonadi::Item::List &items, QWidget *parent = Q_NULLPTR);
    ~MergeContactWidget();

    void clear();

Q_SIGNALS:
    void mergeContact(const Akonadi::Item::List &lst, const Akonadi::Collection &col);
    void contactSelected(const Akonadi::Item &item);

private Q_SLOTS:
    void slotUpdateMergeButton();
    void slotMergeContacts();

private:
    Akonadi::Item::List listSelectedContacts() const;
    Akonadi::Item currentItem() const;
    void fillListContact();
    Akonadi::Item::List mItems;
    MergeContactWidgetList *mListWidget;
    QPushButton *mMergeButton;
    Akonadi::CollectionComboBox *mCollectionCombobox;
};
}

#endif // MERGECONTACTWIDGET_H
