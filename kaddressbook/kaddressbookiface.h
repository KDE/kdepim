/*
    This file is part of KAddressBook.
    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KADDRESSBOOKIFACE_H
#define KADDRESSBOOKIFACE_H

#include <dcopobject.h>
#include <kurl.h>
#include <qstringlist.h>
#include <kdepimmacros.h>

class KDE_EXPORT KAddressBookIface : virtual public DCOPObject
{
  K_DCOP

  k_dcop:
    virtual void addEmail( QString addr ) = 0;
    virtual void importVCard( const KURL& url ) = 0;
    virtual void importVCardFromData( const QString& vCard ) = 0;

    virtual ASYNC showContactEditor( QString uid ) = 0;

    /**
      Shows dialog for creation of a new contact.  Returns once a contact
      is created (or canceled).
     */
    virtual void newContact() = 0;

    /**
      Shows dialog for creation of a new distribution list.  Returns once a distribution list
      is created (or canceled).
     */
    virtual void newDistributionList() = 0;

    /**
      Save changes to the address book files.
     */
    virtual QString getNameByPhone( QString phone ) = 0;
    virtual void save() = 0;
    virtual void exit() = 0;

    /**
     * Internal, DCOP-enabled for the kontact plugin.
     * Return true if handled, false if command-line was empty.
     */
    virtual bool handleCommandLine() = 0;

    virtual void loadProfile( const QString& path ) = 0;
    virtual void saveToProfile( const QString& path ) const = 0;
};

#endif
