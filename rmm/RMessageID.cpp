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
# pragma implementation "RMM_MessageID.h"
#endif

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>

#include <qstring.h>
#include <qregexp.h>
#include <qstrlist.h>
#include <RMM_MessageID.h>
#include <RMM_Token.h>

using namespace RMM;

int RMessageID::seq_ = 0;

RMessageID::RMessageID()
    :    RHeaderBody()
{
    rmmDebug("ctor");
}

RMessageID::RMessageID(const RMessageID & messageID)
    :    RHeaderBody(messageID),
        localPart_(messageID.localPart_),
        domain_(messageID.domain_)
{
    rmmDebug("ctor");
}

RMessageID::RMessageID(const QCString & s)
    :    RHeaderBody(s)
{
}

RMessageID::~RMessageID()
{
    rmmDebug("dtor");
}

    bool
RMessageID::operator == (RMessageID & msgID)
{
    parse();
    msgID.parse();

    return (
        localPart_    == msgID.localPart_ &&
        domain_        == msgID.domain_);
}

    RMessageID &
RMessageID::operator = (const RMessageID & messageID)
{
    rmmDebug("operator =");
    if (this == &messageID) return *this; // Avoid a = a
    
    localPart_ = messageID.localPart_;
    domain_ = messageID.domain_;
    
    rmmDebug("operator = ...");
    rmmDebug("localPart_ == " + localPart_);
    rmmDebug("domain_ == " + domain_);
    
    RHeaderBody::operator = (messageID);
    
    assembled_ = false;
    return *this;
}

    RMessageID &
RMessageID::operator = (const QCString & s)
{
    rmmDebug("operator =");
    RHeaderBody::operator = (s);
    assembled_ = false;
    return *this;
}


    QDataStream &
RMM::operator >> (QDataStream & s, RMessageID & mid)
{
    s    >> mid.localPart_
        >> mid.domain_;
    mid.parsed_ = true;
    mid.assembled_ = false;
    return s;
}
        
    QDataStream &
RMM::operator << (QDataStream & s, RMessageID & mid)
{
    mid.parse();
    s    << mid.localPart_
        << mid.domain_;
    return s;
}
    
    QCString
RMessageID::localPart()
{
    parse();
    return localPart_;
}

    void
RMessageID::setLocalPart(const QCString & localPart)
{
    localPart_ = localPart;
    assembled_ = false;
}

    QCString
RMessageID::domain()
{
    parse();
    return domain_;
}

    void
RMessageID::setDomain(const QCString & domain)
{
    domain_ = domain;
    assembled_ = false;
}

    void
RMessageID::_parse()
{
    if (strRep_.isEmpty()) {
        rmmDebug("But there's nothing to parse !");
        return;
    }
    
    int atPos = strRep_.find('@');
    
    if (atPos == -1) {
        parsed_ = true;
        return;
    }
    
    localPart_    = strRep_.left(atPos);
    domain_        = strRep_.right(strRep_.length() - atPos - 1);
    
    if (localPart_.at(0) == '<')
        localPart_.remove(0, 1);
    
    if (domain_.right(1) == ">")
        domain_.remove(domain_.length() - 1, 1);
}

    void
RMessageID::_assemble()
{
    strRep_ = "<" + localPart_ + "@" + domain_ + ">";
}

    void
RMessageID::createDefault()
{
    rmmDebug("createDefault() called");

    struct timeval timeVal;
    struct timezone timeZone;
    
    gettimeofday(&timeVal, &timeZone);
    int t = timeVal.tv_sec;

    localPart_ =
        "Empath." +
        QCString().setNum(t)        + '.' +
        QCString().setNum(getpid())    + '.' +
        QCString().setNum(seq_++);
    
    struct utsname utsName;
    if (uname(&utsName) == 0)
        domain_ = utsName.nodename;
    else
        domain_ = "localhost.localdomain";

    rmmDebug("Created \"" + localPart_ + "." + domain_ + "\"");
    parsed_ = true;
    assembled_ = false;
}

// vim:ts=4:sw=4:tw=78
