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

EmpathCachedMessage::EmpathCachedMessage(RMM::RMessage * m, const QString & r)
    :   message_(m)
{
    ref(r);
}

EmpathCachedMessage::~EmpathCachedMessage()
{
    delete message_;
    message_ = 0;
}

    RMM::RMessage *
EmpathCachedMessage::message(const QString & r)
{
    references_.remove(r);
    return message_;
}

    unsigned int
EmpathCachedMessage::refCount() const
{
    return references_.count();
}

    void
EmpathCachedMessage::ref(const QString & r)
{
    empathDebug("Adding `" + r + "' to refs");
    references_.append(r);
}

    void
EmpathCachedMessage::deref(const QString & r)
{
    QString notConst(r);
    QStringList::Iterator firstRefByCaller = references_.find(notConst);
    if (firstRefByCaller == references_.end()) {
        empathDebug("Not dereferencing this - can't find ref by `" + r + "'");
        return;
    }
    empathDebug("Removing reference `" + *firstRefByCaller + "'");
    references_.remove(firstRefByCaller);
}

    bool
EmpathCachedMessage::referencedBy(const QString & s)
{
    QStringList::ConstIterator it;
    for (it = references_.begin(); it != references_.end(); ++it)
        empathDebug("Reference: `" + *it + "'");
    return (references_.contains(s) > 0);
}

