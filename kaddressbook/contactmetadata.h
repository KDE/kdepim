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


#ifndef CONTACTMETADATA_H
#define CONTACTMETADATA_H

#include <kabc/addressee.h>

#include <QtCore/QStringList>

/**
 * @short A helper class for storing contact specific settings.
 */
class ContactMetaData
{
  public:
    /**
     * Creates a contact meta data object, that is associated
     * with the given @p contact.
     */
    ContactMetaData( const KABC::Addressee &contact );

    /**
     * Sets the mode that is used for the display
     * name of that contact.
     */
    void setDisplayNameMode( int mode );

    /**
     * Returns the mode that is used for the display
     * name of that contact.
     */
    int displayNameMode() const;

    /**
     * Removes all meta data for that contact.
     */
    void remove();

  private:
    KABC::Addressee mContact;
};

#endif
