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

#include <qstring.h>

#include <RMM_Enum.h>
#include <RMM_Address.h>
#include <RMM_Group.h>
#include <RMM_Mailbox.h>

using namespace RMM;

RAddress::RAddress()
    :    RHeaderBody(),
        mailbox_(0),
        group_(0)
{
    rmmDebug("ctor");
}

RAddress::RAddress(const RAddress & addr)
    :    RHeaderBody(addr)
{
    rmmDebug("copy ctor");

    mailbox_    = 0;
    group_        = 0;
    
    rmmDebug("...");

    if (addr.mailbox_ != 0) {
        
        rmmDebug("mailbox");
        mailbox_ = new RMailbox(*(addr.mailbox_));
    
    } else if (addr.group_ != 0) {
    
        rmmDebug("group");
        group_ = new RGroup(*(addr.group_));
    
    } else {
        
        strRep_ = addr.strRep_;
        parsed_ = false;
    }

    rmmDebug("...");
    assembled_    = false;
}

RAddress::RAddress(const QCString & addr)
    :    RHeaderBody(addr),
        mailbox_(0),
        group_(0)
{
    rmmDebug("ctor");
}

RAddress::~RAddress()
{
    rmmDebug("dtor");
    
    delete mailbox_;
    delete group_;

    mailbox_    = 0;
    group_        = 0;
}

    RAddress &
RAddress::operator = (const RAddress & addr)
{
    rmmDebug("operator =");
    if (this == &addr) return *this; // Don't do a = a.

    delete mailbox_;
    mailbox_    = 0;
    delete group_;
    group_        = 0;
    
    RHeaderBody::operator = (addr);

    if (addr.mailbox_ != 0)
        mailbox_ = new RMailbox(*(addr.mailbox_));
    else if (addr.group_ != 0)
        group_ = new RGroup(*(addr.group_));
    else
        parsed_ = false;
    

    assembled_    = false;
    return *this;
}

    RAddress &
RAddress::operator = (const QCString & s)
{
    rmmDebug("operator = QCString("  + s + ")");

    delete mailbox_;
    mailbox_    = 0;
    delete group_;
    group_        = 0;

    RHeaderBody::operator = (s);

    assembled_    = false;
    return *this;
}

    bool
RAddress::operator == (RAddress & a)
{
    parse();

    a.parse();
    
    if (mailbox_ != 0 && a.mailbox_ != 0)
        return *mailbox_ == *a.mailbox_;
    
    else if    (group_ != 0 && a.group_ != 0)
        return *group_ == *a.group_;
    
    else
        return true;
}

    RGroup *
RAddress::group()
{
    parse();
    return group_;
}

    RMailbox *
RAddress::mailbox()
{
    parse();
    return mailbox_;
}

    void
RAddress::_parse()
{
    delete mailbox_;
    mailbox_    = 0;
    delete group_;
    group_        = 0;

    QCString s = strRep_.stripWhiteSpace();

    // RFC822: group: phrase ":" [#mailbox] ";"
    // -> If a group, MUST end in ";".

    if (s.right(1) == ";") { // This is a group !

        rmmDebug("I'm a group.");

        group_ = new RGroup(s);
        CHECK_PTR(group_);
        group_->parse();

    } else {

        rmmDebug("I'm a mailbox.");

        mailbox_ = new RMailbox(s);
        CHECK_PTR(mailbox_);
        mailbox_->parse();
    }
}

    void
RAddress::_assemble()
{
    if (mailbox_ != 0)
        strRep_ = mailbox_->asString();
    
    else if (group_ != 0)
        strRep_ = group_->asString();
    
    else {
        strRep_ = "foo@bar";
        rmmDebug("_assemble() assigns foo@bar!!!!!!!!!!!!");
    }
    
}

    void
RAddress::createDefault()
{
    rmmDebug("createDefault() called");
    if (mailbox_ == 0 && group_ == 0) {
        rmmDebug("I have no mailbox or group yet");
        mailbox_ = new RMailbox;
        mailbox_->createDefault();
    }
    else if (mailbox_ == 0) {
        rmmDebug("I have no mailbox");
        group_ = new RGroup;
        group_->createDefault();
    } else {
        rmmDebug("I have no group");
        mailbox_ = new RMailbox;
        mailbox_->createDefault();
    }
}

// vim:ts=4:sw=4:tw=78
