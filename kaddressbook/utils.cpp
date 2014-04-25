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


#include "utils.h"

#include <QItemSelectionModel>

#include <KABC/Addressee>
#include <AkonadiCore/EntityTreeModel>

Akonadi::Item::List Utils::collectSelectedContactsItem(QItemSelectionModel *model)
{
    Akonadi::Item::List lst;

    const QModelIndexList indexes = model->selectedRows( 0 );
    for ( int i = 0; i < indexes.count(); ++i ) {
        const QModelIndex index = indexes.at( i );
        if ( index.isValid() ) {
            const Akonadi::Item item =
                    index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
            if ( item.isValid() && item.hasPayload<KABC::Addressee>() ) {
                lst.append( item );
            }
        }
    }
    return lst;
}

