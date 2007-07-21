/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KMOBILETOOLSIFACESADDRESSBOOK_H
#define KMOBILETOOLSIFACESADDRESSBOOK_H

#include <QtCore/QObject>

#include <libkmobiletools/addressbook.h>
#include <libkmobiletools/kmobiletools_export.h>

namespace KABC {
    class Addressee;
}

namespace KMobileTools {
class ContactsList;

namespace Ifaces {

/**
    This interface provides methods to access the mobile phone's address book

    @author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT Addressbook {
public:
    /**
     * Returns an OR-combination of available memory slots
     */
    virtual KMobileTools::Addressbook::MemorySlots availableMemorySlots() const = 0;

    /**
     * Fetches the address book from the phone
     */
    virtual void fetchAddressbook() = 0;

    /**
     * Fetches the address book stored in the given @p memorySlot
     */
    virtual void fetchAddressbook( KMobileTools::Addressbook::MemorySlot memorySlot ) = 0;

    /**
     * Returns the fetched address book
     *
     * @return the fetches address book
     */
    virtual ContactsList addressbook() const = 0;

    /**
     * Adds the @p addressee to the address book
     *
     * @param addressee the addresse to add
     */
    virtual void addAddressee( const KABC::Addressee& addressee ) = 0;

    /**
     * Replaces the existing @p oldAddressee on the phone with @p newAddressee
     *
     * @param oldAddressee the existing address book entry
     * @param newAddressee the new address book entry
     */
    virtual void editAddressee( const KABC::Addressee& oldAddressee,
                                const KABC::Addressee& newAddressee ) = 0;

    /**
     * Removes the @p addressee from the address book
     *
     * @param addressee the addresse to remove
     */
    virtual void removeAddressee( const KABC::Addressee& addressee ) = 0;

    virtual ~Addressbook();

protected:
//Q_SIGNALS:

    /**
     * This signal is emitted when the address book has been fetched from
     * the phone
     */
    virtual void addressbookFetched() = 0;

    /**
     * This signal is emitted when an addressee has been added to the phone's
     * address book
     *
     * @p addressee the addresse that has been added
     */
    virtual void addresseeAdded( const KABC::Addressee& addressee ) = 0;

    /**
     * This signal is emitted when an addressee has been edited on the phone's
     * address book
     *
     * @param oldAddressee the old address book entry
     * @param newAddressee the new address book entry
     */
    virtual void addresseeEdited( const KABC::Addressee& oldAddressee,
                                  const KABC::Addressee& newAddressee ) = 0;

    /**
     * This signal is emitted when an addressee has been removed from the phone's
     * address book
     *
     * @p addressee the addresse that has been removed
     */
    virtual void addresseeRemoved( const KABC::Addressee& addressee ) = 0;

};

}
}

Q_DECLARE_INTERFACE(KMobileTools::Ifaces::Addressbook, "org.kde.KMobileTools.Ifaces.Addressbook/0.1")


#endif
