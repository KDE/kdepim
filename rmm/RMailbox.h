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
# pragma implementation "RMM_Mailbox.h"
#endif

#include <qstrlist.h>

#include <RMM_Mailbox.h>
#include <RMM_Token.h>
#include <RMM_Defines.h>

using namespace RMM;

RMailbox::RMailbox()
    :    RMessageComponent()
{
    // Empty.
}

RMailbox::RMailbox(const RMailbox & mailbox)
    :   RMessageComponent(mailbox),
        phrase_       (mailbox.phrase_),
        route_        (mailbox.route_),
        localPart_    (mailbox.localPart_),
        domain_       (mailbox.domain_)
{
    // Empty.
}

RMailbox::~RMailbox()
{
    // Empty.
}

RMailbox::RMailbox(const QCString & s)
    :    RMessageComponent(s)
{
    // Empty.
}

    RMailbox &
RMailbox::operator = (const RMailbox & mailbox)
{
    if (this == &mailbox) return *this; // Avoid a = a
    
    phrase_       = mailbox.phrase_;
    route_        = mailbox.route_;
    localPart_    = mailbox.localPart_;
    domain_       = mailbox.domain_;
    
    RMessageComponent::operator = (mailbox);
    
    assembled_    = false;
    return *this;
}

    RMailbox &
RMailbox::operator = (const QCString & s)
{
    RMessageComponent::operator = (s);
    
    assembled_    = false;
    return *this;
}

    bool
RMailbox::operator == (RMailbox & m)
{
    parse();
    m.parse();

    return (
        phrase_     == m.phrase_    &&
        route_        == m.route_        &&
        localPart_    == m.localPart_    &&
        domain_        == m.domain_);
}

    QDataStream &
RMM::operator >> (QDataStream & s, RMailbox & mailbox)
{
    s    >> mailbox.phrase_
        >> mailbox.route_
        >> mailbox.localPart_
        >> mailbox.domain_;
    mailbox.parsed_        = true;
    mailbox.assembled_    = false;
    return s;
}
    
    QDataStream &
RMM::operator << (QDataStream & s, RMailbox & mailbox)
{
    mailbox.parse();
    s   << mailbox.phrase_
        << mailbox.route_
        << mailbox.localPart_
        << mailbox.domain_;
    return s;
}


    QCString
RMailbox::phrase()
{
    parse();
    return phrase_;
}


    void
RMailbox::setPhrase(const QCString & s)
{
    phrase_ = s.data();
    assembled_ = false;
}

    QCString
RMailbox::route()
{
    parse();
    return route_;
}

    void
RMailbox::setRoute(const QCString & s)
{
    route_ = s.data();
    assembled_ = false;
}

    QCString
RMailbox::localPart()
{
    parse();
    return localPart_;
}

    void
RMailbox::setLocalPart(const QCString & s)
{
    localPart_ = s.data();
    assembled_ = false;
}


    QCString
RMailbox::domain()
{
    parse();
    return domain_;
}

    void
RMailbox::setDomain(const QCString & s)
{
    domain_ = s.data();
    assembled_ = false;
}

    void
RMailbox::_parse()
{
    if (strRep_.find('@') == -1) { // Must contain '@' somewhere. (RFC822)
        rmmDebug("This is not a valid mailbox !");
        return;
    }
    
    QStrList l;
    RTokenise(strRep_, " \n", l, true);

    bool hasRouteAddress(false);

    QStrListIterator it(l);

    for (; it.current(); ++it)
        hasRouteAddress = (*(it.current()) == '<');
    
    if (hasRouteAddress) { // It's phrase route-addr

        // Deal with the phrase part. Just put in a string.
        phrase_ = "";
        int i = 0;
        QCString s = l.at(i++);
        
        // We're guaranteed to hit '<' since hasRouteAddress == true.
        while (s.at(0) != '<') {
            phrase_ += s;
            s = l.at(i++);
            if (s.at(0) != '<')
                phrase_ += ' ';
        }
        --i;
        
        phrase_ = phrase_.stripWhiteSpace();

        // So by now we are left with only the route part.
        route_ = "";

        for (Q_UINT32 n = i; n < l.count(); n++) {
            route_ += l.at(n);
            if (n + 1 < l.count()) route_ += ' ';
        }

    } else { // It's just addr-spec
        
        while (strRep_.at(0) == '<')
            strRep_.remove(0, 1);
    
        while (strRep_.at(strRep_.length()) == '>')
            strRep_.remove(strRep_.length(), 1);

        // Re-use l. It's guaranteed to be cleared by RTokenise.
        RTokenise(strRep_, "@", l);
        
        localPart_ = l.at(0);
        localPart_ = localPart_.stripWhiteSpace();
        if (l.count() == 2) {
            domain_ = l.at(1);
            domain_ = domain_.stripWhiteSpace();
        } else domain_ = "";

        // Easy, eh ?
    }
}


    void
RMailbox::_assemble()
{
    strRep_ = "";
    if (localPart_.isEmpty()) // This is 'phrase route-addr' style
        strRep_ = phrase_ + " " + route_;
    else
        strRep_ = localPart_ + "@" + domain_;
}

    void
RMailbox::createDefault()
{
    phrase_       = "";
    route_        = "";
    localPart_    = "foo";
    domain_       = "bar";
    strRep_       = "<foo@bar>";
    
    assembled_ = false;
}

// vim:ts=4:sw=4:tw=78
