/* This file is part of the KDE libraries
   Copyright (C) 2002 Holger Freyther <freyher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef kaddressbooksyncentry_h
#define kaddressbooksyncentry_h

#include <kabc/addressbook.h>
#include "ksyncentry.h"

/** This is the KAddressbookSyncEntry it encapsules
 *  the KABC addressbook.
 *
 */

class KAddressbookSyncEntry : public KSyncEntry{
 public:
    KAddressbookSyncEntry();
    KAddressbookSyncEntry(KABC::AddressBook * );
    ~KAddressbookSyncEntry();

    /** @return returns the addressbook
     *
     */
    KABC::AddressBook* addressbook();
    /**
     * @param adr sets the AddreesBook to adr
     */

    void setAddressbook(KABC::AddressBook *adr );
    virtual QString type() { return QString::fromLatin1("KAddressbookSyncEntry" ); };

    KABC::AddressBook* modified() { return m_mod; };
    void setModified( KABC::AddressBook* );

    KABC::AddressBook* added() { return m_add; };
    void setAdded( KABC::AddressBook* );

    KABC::AddressBook* deleted() { return m_del; }
    void setDeleted( KABC::AddressBook* );

    virtual QString name();
    virtual void setName(const QString & );
    virtual QString id();
    virtual void setId(const QString & );
    virtual QString oldId();
    virtual void setOldId(const QString & );

    virtual QString timestamp();
    virtual void setTimestamp(const QString & );
    virtual bool equals(KSyncEntry * );
    virtual KSyncEntry* clone();
  private:
    KABC::AddressBook* m_addressb;
    KABC::AddressBook* m_mod;
    KABC::AddressBook* m_del;
    KABC::AddressBook* m_add;
    class AddressbookSyncEntryPrivate;
    AddressbookSyncEntryPrivate *d;
    QString m_name;
    QString m_oldId;
    QString m_time;
};

#endif









