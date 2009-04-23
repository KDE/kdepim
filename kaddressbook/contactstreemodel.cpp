/*
    This file is part of KContactManager.

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

#include "contactstreemodel.h"

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kicon.h>
#include <klocale.h>

using namespace Akonadi;

ContactsTreeModel::ContactsTreeModel( Session *session, Monitor *monitor, QObject *parent )
  : EntityTreeModel( session, monitor, parent )
{
}

ContactsTreeModel::~ContactsTreeModel()
{
}

QVariant ContactsTreeModel::getData( Item item, int column, int role ) const
{
  if ( item.mimeType() == KABC::Addressee::mimeType() ) {
    if ( !item.hasPayload<KABC::Addressee>() ) {

      // Pass modeltest
      if ( role == Qt::DisplayRole )
        return item.remoteId();

      return QVariant();
    }

    const KABC::Addressee contact = item.payload<KABC::Addressee>();

    if ( role == Qt::DecorationRole ) {
      if ( column == 0 ) {
        const KABC::Picture picture = contact.photo();
        if ( picture.isIntern() ) {
          return picture.data().scaled( QSize( 16, 16 ) );
        } else {
          return KIcon( QLatin1String( "x-office-contact" ) );
        }
      }
      return QVariant();
    } else if ( (role == Qt::DisplayRole) || (role == Qt::EditRole) ) {
      switch ( column ) {
        case 0:
          if ( !contact.formattedName().isEmpty() )
            return contact.formattedName();
          else
            return contact.assembledName();
        case 1:
          return contact.givenName();
          break;
        case 2:
          return contact.familyName();
          break;
        case 3:
          return contact.preferredEmail();
          break;
        default:
          break;
      }
    }
  } else if ( item.mimeType() == KABC::ContactGroup::mimeType() ) {
    if ( !item.hasPayload<KABC::ContactGroup>() ) {

      // Pass modeltest
      if ( role == Qt::DisplayRole )
        return item.remoteId();

      return QVariant();
    }

    if ( role == Qt::DecorationRole ) {
      if ( column == 0 )
        return KIcon( QLatin1String( "x-mail-distribution-list" ) );
      else
        return QVariant();
    } else if ( (role == Qt::DisplayRole) || (role == Qt::EditRole) ) {
      switch ( column ) {
        case 0:
          {
            const KABC::ContactGroup group = item.payload<KABC::ContactGroup>();
            return group.name();
          }
          break;
        default:
          break;
      }
    }
  }

  return EntityTreeModel::getData( item, column, role );
}

QVariant ContactsTreeModel::getData( Collection collection, int column, int role ) const
{
  if ( role == Qt::DisplayRole ) {
    switch ( column ) {
      case 0:
        return EntityTreeModel::getData( collection, column, role );
      default:
        return QString(); // pass model test
    }
  }

  return EntityTreeModel::getData( collection, column, role );
}

int ContactsTreeModel::columnCount( const QModelIndex &index ) const
{
  Q_UNUSED(index);
  return 4;
}

QVariant ContactsTreeModel::getHeaderData( int section, Qt::Orientation orientation, int role, int headerSet ) const
{
  if ( role == Qt::DisplayRole ) {
    if ( orientation == Qt::Horizontal ) {
      if ( headerSet == EntityTreeModel::CollectionTreeHeaders ) {

        if ( section >= 1 )
          return QVariant();

        switch ( section ) {
          case 0:
            return i18nc( "@title:column, contact groups", "Group" );
            break;
        }
      } else if ( headerSet == EntityTreeModel::ItemListHeaders ) {
        if ( section >= 4 )
          return QVariant();

        switch ( section ) {
          case 0:
            return i18nc( "@title:column, name of a person", "Name" );
            break;
          case 1:
            return KABC::Addressee::givenNameLabel();
            break;
          case 2:
            return KABC::Addressee::familyNameLabel();
            break;
          case 3:
            return KABC::Addressee::emailLabel();
            break;
        }
      }
    }
  }

  return EntityTreeModel::getHeaderData( section, orientation, role, headerSet );
}

#include "contactstreemodel.moc"
