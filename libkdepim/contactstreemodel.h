/*
    This file is part of Akonadi Contact.

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

#ifndef AKONADI_CONTACTSTREEMODEL_H
#define AKONADI_CONTACTSTREEMODEL_H

#include "kdepim_export.h"

#include <akonadi/entitytreemodel.h>

namespace Akonadi {

/**
 * @short A model for contacts and contact groups as stored in Akonadi.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 * @since 4.4
 */
class KDEPIM_EXPORT ContactsTreeModel : public EntityTreeModel
{
  Q_OBJECT

  public:
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

    /**
     * Describes a list of columns of the contacts tree model.
     */
    typedef QList<Column> Columns;

    /**
     * Describes the role for contacts and contact groups.
     */
    enum Roles
    {
      DateRole = UserRole + 1   ///< The QDate object for the current index.
    };

    /**
     * Creates a new contacts tree model.
     *
     * @param session The Session to use to communicate with Akonadi.
     * @param monitor The ChangeRecorder whose entities should be represented in the model.
     * @param parent The parent object.
     */
    explicit ContactsTreeModel( ChangeRecorder *monitor, QObject *parent = 0 );

    /**
     * Destroys the contacts tree model.
     */
    virtual ~ContactsTreeModel();

    /**
     * Sets the @p columns that the model should show.
     */
    void setColumns( const Columns &columns );

    /**
     * Returns the columns that the model currently shows.
     */
    Columns columns() const;

    virtual QVariant entityData( const Item &item, int column, int role = Qt::DisplayRole ) const;
    virtual QVariant entityData( const Collection &collection, int column, int role = Qt::DisplayRole ) const;
    virtual QVariant entityHeaderData( int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup ) const;
    virtual int entityColumnCount( HeaderGroup headerGroup ) const;

  private:
    class Private;
    Private* const d;
};

}

#endif
