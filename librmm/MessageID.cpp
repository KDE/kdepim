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


#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <qstring.h>
#include <qregexp.h>
#include <qstrlist.h>
#include <rmm/Defines.h>
#include <rmm/MessageID.h>
#include <rmm/Token.h>

using namespace RMM;

int MessageID::seq_ = 0;

MessageID::MessageID()
    :    HeaderBody()
{
    // Empty.
}

MessageID::MessageID(const MessageID & messageID)
    :   HeaderBody (messageID),
        localPart_  (messageID.localPart_.copy()),
        domain_     (messageID.domain_.copy())
{
    // Empty.
}

MessageID::MessageID(const QCString & s)
    :    HeaderBody(s)
{
    // Empty.
}

MessageID::~MessageID()
{
    // Empty.
}

    bool
MessageID::operator == (MessageID & msgID)
{
    parse();
    msgID.parse();

    return (
        localPart_  == msgID.localPart_ &&
        domain_     == msgID.domain_);
}

    MessageID &
MessageID::operator = (const MessageID & messageID)
{
    if (this == &messageID) return *this; // Avoid a = a
    
    localPart_  = messageID.localPart_.copy();
    domain_     = messageID.domain_.copy();
    
    HeaderBody::operator = (messageID);
    
    return *this;
}

    MessageID &
MessageID::operator = (const QCString & s)
{
    HeaderBody::operator = (s);
    return *this;
}


    QDataStream &
RMM::operator >> (QDataStream & s, MessageID & mid)
{
    s    >> mid.localPart_
        >> mid.domain_;
    mid.parsed_ = true;
    mid.assembled_ = false;
    return s;
}
        
    QDataStream &
RMM::operator << (QDataStream & s, MessageID & mid)
{
    mid.parse();
    s    << mid.localPart_
        << mid.domain_;
    return s;
}
    
    QCString
MessageID::localPart()
{
    parse();
    return localPart_;
}

    void
MessageID::setLocalPart(const QCString & s)
{
    parse();
    localPart_ = s.copy();
}

    QCString
MessageID::domain()
{
    parse();
    return domain_;
}

    void
MessageID::setDomain(const QCString & s)
{
    parse();
    domain_ = s.copy();
}

    void
MessageID::_parse()
{
    if (strRep_.isEmpty())
        return;
    
    int atPos = strRep_.find('@');
    
    if (atPos == -1)
        return;
    
    localPart_  = strRep_.left(atPos);
    domain_     = strRep_.right(strRep_.length() - atPos - 1);
    
    if (localPart_.at(0) == '<')
        localPart_.remove(0, 1);
    
    if (domain_.right(1) == ">")
        domain_.remove(domain_.length() - 1, 1);
}

    void
MessageID::_assemble()
{
    if (localPart_.isEmpty() || domain_.isEmpty())
        strRep_ = "";
    else
        strRep_ = "<" + localPart_ + "@" + domain_ + ">";
}

    void
MessageID::createDefault()
{
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

    parsed_ = true;
    assembled_ = false;
}

// vim:ts=4:sw=4:tw=78
