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

#ifndef KMOBILETOOLSADDRESSBOOKENTRY_H
#define KMOBILETOOLSADDRESSBOOKENTRY_H

#include <kabc/addressee.h>
#include <libkmobiletools/kmobiletools_export.h>

namespace KMobileTools {

/**
	@author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT AddressbookEntry : public KABC::Addressee
{
public:
    /**
    * This enum type defines the type of phone book memory slots.
    *
    * - Any: Any memory slot (generic)
    * - Sim : Sim card
    * - Phone : Phone
    * - DataCard : Data card
    * - Unknown: Unknown storage location
    */
    enum MemorySlot { Phone = 0x01, Sim = 0x02, DataCard = 0x04, Unknown = 0x08 };

    Q_DECLARE_FLAGS(MemorySlots, MemorySlot)

    /**
     * Constructs an empty address book entry for the given @p memorySlot
     *
     * @param memorySlot the slot where the entry is located
     */
    AddressbookEntry( AddressbookEntry::MemorySlot memorySlot = Unknown );

    /**
     * Destructs the address book entry
     */
    virtual ~AddressbookEntry();

    /**
     * Sets the memory slot the entry is located at
     *
     * @param memorySlot the memory slot
     */
    void setMemorySlot( AddressbookEntry::MemorySlot memorySlot );

    /**
     * Returns the memory slot the entry is located at
     *
     * @return the memory slot
     */
    AddressbookEntry::MemorySlot memorySlot() const;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KMobileTools::AddressbookEntry::MemorySlots)

#endif
