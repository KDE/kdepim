/*
    This file is part of KContactManager.

    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>
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

#ifndef CONTACTSTREEMODEL_H
#define CONTACTSTREEMODEL_H

#include "entitytreemodel.h"

namespace Akonadi {

class ContactsTreeModel : public EntityTreeModel
{
  Q_OBJECT

  public:
    enum Roles
    {
      EmailCompletionRole = EntityTreeModel::UserRole
    };

    /**
     * Describes the columns that can be shown by the model.
     */
    enum Column
    {
      /**
       * Shows the formatted name or, if empty, the assembled name.
       */
      FullName,

      /**
       * Shows only given and family name.
       */
      ShortName,

      /**
       * Shows the birthday.
       */
      Birthday,

      /**
       * Shows the formatted home address.
       */
      HomeAddress,

      /**
       * Shows the formatted business address.
       */
      BusinessAddress,

      /**
       * Shows the phone numbers.
       */
      PhoneNumbers,

      /**
       * Shows the preferred email address.
       */
      PreferredEmail,

      /**
       * Shows all email address.
       */
      AllEmails,

      /**
       * Shows organization name.
       */
      Organization,

      /**
       * Shows homepage url.
       */
      Homepage,

      /**
       * Shows the note.
       */
      Note
    };
    typedef QList<Column> Columns;

    ContactsTreeModel( Session *session, Monitor *monitor, QObject *parent = 0 );
    virtual ~ContactsTreeModel();

    void setColumns( const Columns &columns );

    virtual QVariant getData( const Item &item, int column, int role = Qt::DisplayRole ) const;
    virtual QVariant getData( const Collection &collection, int column, int role = Qt::DisplayRole ) const;
    virtual int columnCount( const QModelIndex &index = QModelIndex() ) const;
    virtual QVariant getHeaderData( int section, Qt::Orientation orientation, int role, int headerSet ) const;

  private:
    Columns mColumns;
};

}

#endif
