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
# pragma implementation "RMM_Address.h"
#endif

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
        name_           (addr.name_),
        phrase_         (addr.phrase_)
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
    name_           = addr.name_;
    phrase_         = addr.phrase_;
 
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
        // TODO

    } else {
        ok = (
// FIXME            mailboxList_    == a.mailboxList_   &&
            name_           == a.name_          &&
            phrase_         == a.phrase_
        );
    }
    
    return ok;
}

    void
RAddress::_parse()
{
    cerr << "RAddress::_parse(): strRep : `" << strRep_ << "'" << endl;
    QCString s = strRep_.stripWhiteSpace();

    // RFC822: group: phrase ":" [#mailbox] ";"
    // -> If a group, MUST end in ";".

    if (s.right(1) == ";") { // This is a group !

        rmmDebug("I'm a group.");
        rmmDebug("Not implemented !");

    } else {

        RMailbox m(s);
        m.parse();
        mailboxList_.append(m);
    }
}

    void
RAddress::_assemble()
{
    if (type() == RAddress::Group) {

        rmmDebug("assembling group not implemented");

    } else {
        
        RMailbox m(*(mailboxList_.at(0)));
        strRep_ = m.asString();
    }
}

    void
RAddress::createDefault()
{
    mailboxList_.clear();
    RMailbox m;
    m.createDefault();
    mailboxList_.append(m);
    assembled_ = true;
}

    RAddress::Type
RAddress::type()
{
    parse();
    return name_.isEmpty() ? Mailbox : Group;
}


    QDataStream &
RMM::operator >> (QDataStream & s, RMM::RAddress & addr)
{
    unsigned int count;

    s   >> addr.name_
        >> addr.phrase_
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

    s   << addr.name_
        << addr.phrase_
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
    return phrase_;
}


    void
RAddress::setPhrase(const QCString & s)
{
    parse();
    phrase_ = s.data();
}

    QCString
RAddress::route()
{
    parse();
    RMailbox m(*(mailboxList_.at(0)));
    return m.route();
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
    RMailbox m(*(mailboxList_.at(0)));
    return m.localPart();
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
    RMailbox m(*(mailboxList_.at(0)));
    return m.domain();
}

    void
RAddress::setDomain(const QCString & s)
{
    parse();
    (*(mailboxList_.at(0))).setDomain(s);
}

   QCString
RAddress::name()
{
    parse();
    return name_;
}

    void
RAddress::setName(const QCString & s)
{
    parse();
    name_ = s;
}

    QValueList<RMailbox>
RAddress::mailboxList()
{
    parse();
    return mailboxList_;
}


// vim:ts=4:sw=4:tw=78
