/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef AKONADI_CONTACTMETADATA_P_H
#define AKONADI_CONTACTMETADATA_P_H

#include <QtCore/QStringList>
#include <QtCore/QVariant>

namespace Akonadi
{

class Item;

/**
 * @short A dummy class until we can access the ContactMetaData class from kdepimlibs
 */
class ContactMetaData
{
  public:
    /**
     * Creates a contact meta data object.
     */
#ifdef KDEPIM_STATIC_LIBS
    ContactMetaData();
#else
    ContactMetaData() {}
#endif

    /**
     * Destroys the contact meta data object.
     */
#ifdef KDEPIM_STATIC_LIBS
    ~ContactMetaData();
#else
    ~ContactMetaData() {}
#endif

    /**
     * Loads the meta data for the given @p contact.
     */
#ifdef KDEPIM_STATIC_LIBS
    void load( const Akonadi::Item &contact );
#else
    void load( const Akonadi::Item &contact ) {}
#endif

    /**
     * Stores the meta data to the given @p contact.
     */
#ifdef KDEPIM_STATIC_LIBS
    void store( Akonadi::Item &contact );
#else
    void store( Akonadi::Item &contact ) {}
#endif

    /**
     * Sets the mode that is used for the display
     * name of that contact.
     */
#ifdef KDEPIM_STATIC_LIBS
    void setDisplayNameMode( int mode );
#else
    void setDisplayNameMode( int mode ) {}
#endif

    /**
     * Returns the mode that is used for the display
     * name of that contact.
     */
#ifdef KDEPIM_STATIC_LIBS
    int displayNameMode() const;
#else
    int displayNameMode() const { return 0; }
#endif

    /**
     * Sets the @p descriptions of the custom fields of that contact.
     *
     * The description list contains a QVariantMap for each custom field
     * with the following keys:
     *   - key   (string) The identifier of the field
     *   - title (string) The i18n'ed title of the field
     *   - type  (string) The type description of the field
     *     Possible values for type description are
     *       - text
     *       - numeric
     *       - boolean
     *       - date
     *       - time
     *       - datetime
     */
#ifdef KDEPIM_STATIC_LIBS
    void setCustomFieldDescriptions( const QVariantList &descriptions );
#else
    void setCustomFieldDescriptions( const QVariantList &descriptions ) {}
#endif

    /**
     * Returns the descriptions of the custom fields of the contact.
     */
#ifdef KDEPIM_STATIC_LIBS
    QVariantList customFieldDescriptions() const;
#else
    QVariantList customFieldDescriptions() const { return QVariantList(); }
#endif
};

}

#endif
