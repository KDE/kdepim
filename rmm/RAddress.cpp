/* This file is part of the KDE project

   Copyright (C) 1999, 2000 Rik Hemsley <rik@kde.org>
             (C) 1999, 2000 Wilco Greven <j.w.greven@student.utwente.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <iostream>

#include <qstring.h>
#include <qstrlist.h>

#include <RMM_Token.h>
#include <RMM_Enum.h>
#include <RMM_Address.h>
#include <RMM_Defines.h>

using namespace RMM;

RAddress::RAddress()
    :   RHeaderBody()
{
    // Empty.
}

RAddress::RAddress(const RAddress & addr)
    :   RHeaderBody     (addr),
        mailboxList_    (addr.mailboxList_),
        phrase_         (addr.phrase_.copy())
{
    // Empty.
}

RAddress::RAddress(const QCString & addr)
    :   RHeaderBody(addr)
{
    // Empty.
}

RAddress::~RAddress()
{
    // Empty.
}

    RAddress &
RAddress::operator = (const RAddress & addr)
{
    if (this == &addr) return *this; // Don't do a = a.

    mailboxList_    = addr.mailboxList_;
    phrase_         = addr.phrase_.copy();
 
    RHeaderBody::operator = (addr);

    return *this;
}

    RAddress &
RAddress::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RAddress::operator == (RAddress & a)
{
    parse();
    a.parse();

    bool ok(false);
    
    if (a.type() == RAddress::Mailbox) {

        ok = ((*(mailboxList_.at(0))) == (*(a.mailboxList_.at(0))));

    } else {
        // TODO
        qDebug("Not implemented !");
    }
    
    return ok;
}

    void
RAddress::_parse()
{
    strRep_ = strRep_.stripWhiteSpace();

    // RFC822: group: phrase ":" [#mailbox] ";"
    // -> If a group, MUST end in ";".

    mailboxList_.clear();

    if (strRep_.right(1) == ";")
    { // This is a group !
		// TOUNDO :)
		
		// phrase_ lasts from 0 to start
		// the address tokens comes at start+ till
		// the end.  We need to remove the semicolon too
  
		int start = strRep_.find(':');

        if (-1 != start)
            phrase_ = strRep_.left(start);

        else {
            rmmDebug("This group is invalid. No ':'");
            return;
        }
				
		// ok. Got the group name.  Lets get the address.
		QCString address(strRep_.mid(start+1, strRep_.length()-1));
		
		QStrList list;
		RTokenise(address, ",", list, true, false); //Tokenise?  What a brit :)
		
	    for (QStrListIterator it(list); it.current(); ++it)
		    mailboxList_.append(RMailbox(*it.current()));

    } else

        mailboxList_.append(RMailbox(strRep_));
}

    void
RAddress::_assemble()
{
    if (type() == RAddress::Group)
    {
        strRep_ = phrase_;

        strRep_ += ": ";

        QValueList<RMailbox>::Iterator it = mailboxList_.begin();

        bool firstTime = true;

        for (; it != mailboxList_.end(); ++it) {

            if (!firstTime)
               strRep_ +=  + ", ";

            firstTime = false;

            strRep_ += (*it).asString();
        }

        strRep_ += " ;";


    } else
        
        strRep_ = (*mailboxList_.at(0)).asString();
}

    void
RAddress::createDefault()
{
    mailboxList_.clear();
    RMailbox m;
    m.createDefault();
    mailboxList_.append(m);
    phrase_ = "";
    parsed_ = assembled_ = true;
}

    RAddress::Type
RAddress::type()
{
    parse();
    return phrase_.isEmpty() ? Mailbox : Group;
}

    QDataStream &
RMM::operator >> (QDataStream & s, RMM::RAddress & addr)
{
    unsigned int count;

    s   >> addr.phrase_
        >> count;
    
    for (unsigned int i = 0; i < count; i++) {
        RMailbox m;
        s >> m;
        addr.mailboxList_.append(m);
    }

    addr.parsed_ = true; 
    addr.assembled_ = false; 

    return s;
}

    QDataStream &
RMM::operator << (QDataStream & s, RMM::RAddress & addr)
{
    addr.parse();

    s   << addr.phrase_
        << (unsigned int)(addr.mailboxList_.count());

    QValueList<RMailbox>::Iterator it;

    for (it = addr.mailboxList_.begin(); it != addr.mailboxList_.end(); ++it)
        s << *it;
    
    return s;
}

    QCString
RAddress::phrase()
{
    parse();

    if (phrase_.isEmpty())
        return (*(mailboxList_.at(0))).phrase();
    else
        return phrase_;
}

    void
RAddress::setPhrase(const QCString & s)
{
    parse();
    if (phrase_.isEmpty())
        (*(mailboxList_.at(0))).setPhrase(s);
    else
        phrase_ = s.copy();
}

    QCString
RAddress::route()
{
    parse();
    return (*(mailboxList_.at(0))).route();
}

    void
RAddress::setRoute(const QCString & s)
{
    parse();
    (*(mailboxList_.at(0))).setRoute(s);
}

    QCString
RAddress::localPart()
{
    parse();
    return (*(mailboxList_.at(0))).localPart();
}

    void
RAddress::setLocalPart(const QCString & s)
{
    parse();
    (*(mailboxList_.at(0))).setLocalPart(s);
}


    QCString
RAddress::domain()
{
    parse();
    return (*(mailboxList_.at(0))).domain();
}

    void
RAddress::setDomain(const QCString & s)
{
    parse();
    (*(mailboxList_.at(0))).setDomain(s);
}

    QValueList<RMailbox>
RAddress::mailboxList()
{
    parse();
    return mailboxList_;
}


// vim:ts=4:sw=4:tw=78
