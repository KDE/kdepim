/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathIndex.h"
#endif

// Local includes
#include "EmpathIndexRecord.h"
#include "EmpathIndex.h"
#include "EmpathFolder.h"

EmpathIndex::EmpathIndex()
    :    QDict<EmpathIndexRecord>(101)
{
    setAutoDelete(true);
}

EmpathIndex::~EmpathIndex()
{
    // empty
}

    int
EmpathIndex::compareItems(void * item1, void * item2)
{
    // Compare parent id of the two messages.
    Q_UINT32 i1 = ((EmpathIndexRecord *)item1)->date().asUnixTime();
    Q_UINT32 i2 = ((EmpathIndexRecord *)item2)->date().asUnixTime();

    // Follow true definition -
    // 0   if item1 == item2
    // > 0 if item1  > item2
    // < 0 if item1  < item2
    
    return i2 - i1;
}

    EmpathIndexRecord *
EmpathIndex::messageDescription(RMM::RMessageID & id) const
{
    EmpathIndexIterator it(*this);
    
    for (; it.current(); ++it)
        if (it.current()->messageID() == id) return it.current();

    return 0;
}

    Q_UINT32
EmpathIndex::countUnread() const
{
    Q_UINT32 unread = count();
    EmpathIndexIterator it(*this);
    for (; it.current(); ++it)
        if (it.current()->status() & RMM::Read) --unread;
    return unread;
}

    void
EmpathIndex::sync()
{
    folder_->update();
}

// vim:ts=4:sw=4:tw=78
