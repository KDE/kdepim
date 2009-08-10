/*
    This file is part of KAddressBook.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef COLLECTIONSELECTIONDIALOG_H
#define COLLECTIONSELECTIONDIALOG_H

#include <kdialog.h>

class QAbstractItemModel;

namespace Akonadi
{
class Collection;
}

namespace KABC
{
class CollectionComboBox;
}

class CollectionSelectionDialog : public KDialog
{
  public:
    explicit CollectionSelectionDialog( QAbstractItemModel *collectionModel, QWidget *parent = 0 );
    ~CollectionSelectionDialog();

    Akonadi::Collection selectedCollection() const;

  private:
    KABC::CollectionComboBox *mCollectionCombo;
};

#endif
