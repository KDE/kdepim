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

#include "addressbookentryitem.h"

AddressbookEntryItem::AddressbookEntryItem( QListWidget* parent )
 : QListWidgetItem( parent, QListWidgetItem::UserType )
{
    m_state = AddressbookEntryItem::Default;
    m_defaultFont = font();
}


AddressbookEntryItem::~AddressbookEntryItem()
{
}

void AddressbookEntryItem::setAddressee( const KMobileTools::AddressbookEntry& addressee )
{
    m_entry = addressee;
}

KMobileTools::AddressbookEntry AddressbookEntryItem::addressee() const
{
    return m_entry;
}

void AddressbookEntryItem::setState( AddressbookEntryItem::State state ) {
    m_state = state;

    QFont font = m_defaultFont;
    switch( state ) {
        case AddressbookEntryItem::Default:
            break;

        case AddressbookEntryItem::AdditionRequested:
            font.setWeight( 35 );
            break;

        case AddressbookEntryItem::EditingRequested:
            font.setItalic( true );
            break;

        case AddressbookEntryItem::RemovalRequested:
            font.setStrikeOut( true );
            break;
    }
    setFont( font );
}

AddressbookEntryItem::State AddressbookEntryItem::state() const {
    return m_state;
}
