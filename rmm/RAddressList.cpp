/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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

#include <iostream>

#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_HeaderBody.h>
#include <RMM_Token.h>

using namespace RMM;

RAddressList::RAddressList()
    :    RHeaderBody()
{
    // Empty.
}


RAddressList::RAddressList(const RAddressList & list)
    :   RHeaderBody(list),
        list_(list.list_)
{
    // Empty.
}

RAddressList::RAddressList(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}

RAddressList::~RAddressList()
{
    // Empty.
}
        
    RAddressList &
RAddressList::operator = (const RAddressList & al)
{
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
        
    RAddress
RAddressList::at(unsigned int i)
{
    parse();

    cerr << "list count == " << list_.count() << endl;
    if (!list_.isEmpty())
        return *(list_.at(i));

    return RAddress();
}

    unsigned int
RAddressList::count()
{
    parse();
    return list_.count();
}

    void
RAddressList::_parse()
{
    list_.clear();

    QStrList l;
    RTokenise(strRep_, ",\n\r", l);

    if (l.count() == 0 && !strRep_.isEmpty()) { // Lets try what we have then.

        RAddress a(strRep_);
        list_.append(a);
        
    } else {

        QStrListIterator lit(l);

        for (; lit.current(); ++lit) {
            RAddress a(lit.current());
            list_.append(a);
        }
    }
}

    void
RAddressList::_assemble()
{
    bool firstTime = true;

    QValueList<RAddress>::Iterator it;

    strRep_ = "";
    
    for (it = list_.begin(); it != list_.end(); ++it) {
        if (!firstTime) 
            strRep_ += QCString(",\n    ");
        firstTime = false;
        strRep_ += (*it).asString();
    }
}

    void
RAddressList::createDefault()
{
    if (count() == 0) {
        RAddress a;
        a.createDefault();
        list_.append(a);
    }
}

// vim:ts=4:sw=4:tw=78
