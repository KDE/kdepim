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

#ifndef ADDRESSBOOKENTRYITEM_H
#define ADDRESSBOOKENTRYITEM_H

#include <QtGui/QListWidgetItem>
#include <QtGui/QListWidget>

#include <libkmobiletools/addressbookentry.h>

/**
 * @author Matthias Lechner <matthias@lmme.de>
 */
class AddressbookEntryItem : public QListWidgetItem
{
public:
    enum State {
        Default = 0x01,
        AdditionRequested = 0x02,   // item is marked as to be added
        EditingRequested = 0x04,    // item is marked as to be edited
        RemovalRequested = 0x08     // item is marked as to be removed
    };

    AddressbookEntryItem( QListWidget* parent = 0 );

    /**
     * Sets the addressee this item holds
     *
     * @param addressee the addressee
     */
    void setAddressee( const KMobileTools::AddressbookEntry& addressee );

    /**
     * Returns the addressee this item holds
     *
     * @return the addressee
     */
    KMobileTools::AddressbookEntry addressee() const;

    void setState( AddressbookEntryItem::State state );

    AddressbookEntryItem::State state() const;

    ~AddressbookEntryItem();

private:
    KMobileTools::AddressbookEntry m_entry;
    AddressbookEntryItem::State m_state;
    QFont m_defaultFont;
};

#endif
