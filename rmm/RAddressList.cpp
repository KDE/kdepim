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
# pragma implementation "RMM_AddressList.h"
#endif

#include <qlist.h>

#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_HeaderBody.h>
#include <RMM_Group.h>
#include <RMM_Mailbox.h>
#include <RMM_Token.h>

using namespace RMM;

RAddressList::RAddressList()
    :    RHeaderBody()
{
    rmmDebug("ctor");
    list_.setAutoDelete(true);
    assembled_    = false;
}


RAddressList::RAddressList(const RAddressList & list)
    :    RHeaderBody(list)
{
    rmmDebug("ctor");
    list_.setAutoDelete(true);
    list_ = list.list_;
    assembled_    = false;
}

RAddressList::RAddressList(const QCString & s)
    :    RHeaderBody(s)
{
    rmmDebug("ctor");
}

RAddressList::~RAddressList()
{
    rmmDebug("dtor");
}
        
    RAddressList &
RAddressList::operator = (const RAddressList & al)
{
    rmmDebug("operator =");
    if (this == &al) return *this; // Don't do a = a.
    
    list_ = al.list_;
    RHeaderBody::operator = (al);

    assembled_    = false;
    return *this;
}
    
    RAddressList &
RAddressList::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RAddressList::operator == (RAddressList & al)
{
    parse();
    if (al.list_.count() != list_.count()) return false;
    return true; // FIXME: Duh ? This isn't right.
}
        
    RAddress *
RAddressList::at(int i)
{
    return list_.at(i);
}

    unsigned int
RAddressList::count()
{
    return list_.count();
}

    void
RAddressList::_parse()
{
    list_.clear();

    QStrList l;
    RTokenise(strRep_, ",\n\r", l);

    if (l.count() == 0 && !strRep_.isEmpty()) { // Lets try what we have then.

        rmmDebug("new RAddress");
        RAddress * a = new RAddress;
        CHECK_PTR(a);
        *a = strRep_;
        list_.append(a);
        
    } else {

        QStrListIterator lit(l);

        for (; lit.current(); ++lit) {
            rmmDebug("new RAddress");
            RAddress * a = new RAddress;
            CHECK_PTR(a);
            *a = lit.current();
            list_.append(a);
        }
    }
}

    void
RAddressList::_assemble()
{
    bool firstTime = true;
    
    RAddressListIterator it(list_);

    strRep_ = "";
    
    for (; it.current(); ++it) {
        
        it.current()->assemble();
        
        if (!firstTime) {
            strRep_ += QCString(",\n    ");
            firstTime = false;
        }

        strRep_ += it.current()->asString();
    }
}


    void
RAddressList::createDefault()
{
    rmmDebug("createDefault() called");
    if (count() == 0) {
        RAddress * a = new RAddress;
        a->createDefault();
        list_.append(a);
    }
}

// vim:ts=4:sw=4:tw=78
