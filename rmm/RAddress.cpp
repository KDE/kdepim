/*
	Empath - Mailer for KDE

	Copyright (C) 1998 Rik Hemsley rik@kde.org

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

#include <qstring.h>

#include <RMM_Enum.h>
#include <RMM_Address.h>
#include <RMM_Group.h>
#include <RMM_Mailbox.h>

RAddress::RAddress()
	:	RHeaderBody(),
		mailbox_(0),
		group_(0)
{
	rmmDebug("ctor");
}

RAddress::RAddress(const RAddress & addr)
	:	RHeaderBody(),
		mailbox_(addr.mailbox_),
		group_(addr.group_)
{
	rmmDebug("ctor");
	assembled_	= false;
}

RAddress::RAddress(const QCString & addr)
	:	RHeaderBody(addr.data()),
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

	mailbox_	= 0;
	group_		= 0;
}

	RAddress &
RAddress::operator = (const RAddress & addr)
{
	rmmDebug("operator =");
    if (this == &addr) return *this; // Don't do a = a.

        delete mailbox_;
        delete group_;

	mailbox_	= 0;
	group_		= 0;

	if (addr.mailbox_ != 0)
		mailbox_ = new RMailbox(*(addr.mailbox_));
	else
		group_ = new RGroup(*(addr.group_));

	RHeaderBody::operator = (addr);

	assembled_	= false;
	return *this;
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
RAddress::parse()
{
	rmmDebug("parse() called");
	
	if (parsed_) return;
	
	delete mailbox_;
	mailbox_	= 0;
	delete group_;
	group_		= 0;

	QCString s = strRep_.stripWhiteSpace();

	// RFC822: group: phrase ":" [#mailbox] ";"
	// -> If a group, MUST end in ";".

	if (s.right(1) == ";") { // This is a group !

		rmmDebug("I'm a group.");

		group_ = new RGroup;
		CHECK_PTR(group_);
		group_->set(s);
		group_->parse();

	} else {

		rmmDebug("I'm a mailbox.");

		mailbox_ = new RMailbox;
		CHECK_PTR(mailbox_);
		rmmDebug(s);
		mailbox_->set(s);
		mailbox_->parse();
	}
	
	parsed_		= true;
	assembled_	= false;
}

	void
RAddress::assemble()
{
	parse();
	rmmDebug("assemble() called");

	if (mailbox_ != 0) {

		strRep_ = mailbox_->asString();

	} else if (group_ != 0) {

		strRep_ = group_->asString();

	} else {
			strRep_ = "foo@bar";
	}
	
	assembled_ = true;
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

