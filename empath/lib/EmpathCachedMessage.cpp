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

#include "EmpathDefines.h"
#include "EmpathCachedMessage.h"

EmpathCachedMessage::EmpathCachedMessage(RMM::RMessage & m)
    :   message_(m)
{
    ref();
}

EmpathCachedMessage::~EmpathCachedMessage()
{
}

    RMM::RMessage
EmpathCachedMessage::message()
{
    deref();
    return message_;
}

    unsigned int
EmpathCachedMessage::refCount() const
{
    return refCount_;
}

    void
EmpathCachedMessage::ref()
{
    ++refCount_;
}

    void
EmpathCachedMessage::deref()
{
    --refCount_;
}

