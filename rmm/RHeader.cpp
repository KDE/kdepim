/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rikkus@postmaster.co.uk
	
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

#include <RMM_Enum.h>
#include <RMM_Header.h>
#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_DateTime.h>
#include <RMM_DispositionType.h>
#include <RMM_Mailbox.h>
#include <RMM_MailboxList.h>
#include <RMM_Mechanism.h>
#include <RMM_MessageID.h>
#include <RMM_Text.h>

RHeader::RHeader()
	:	headerType_(HeaderUnknown),
		headerBody_(0)
{
	rmmDebug("ctor");
}

RHeader::RHeader(const QString & name, RHeaderBody * b)
{
	rmmDebug("ctor");
}

RHeader::RHeader(HeaderType t, RHeaderBody * b)
{
	rmmDebug("ctor");
}

RHeader::~RHeader()
{
	rmmDebug("dtor");
	delete headerBody_;
}

	const RHeader &
RHeader::operator = (const RHeader & h)
{
	rmmDebug("operator =");
	if (this == &h) return *this;
	
	headerName_ = h.headerName_;
	headerType_ = h.headerType_;

	if (headerBody_ != 0) delete headerBody_;
	headerBody_ = new RHeaderBody(*h.headerBody_);
	
	RMessageComponent::operator = (h);
	return *this;
}

	const QString &
RHeader::headerName() const
{
	return headerName_;
}

	HeaderType
RHeader::headerType() const
{
	return headerType_;
}

	RHeaderBody *
RHeader::headerBody() const
{
	return headerBody_;
}

	void
RHeader::setName(const QString & name)
{
	headerName_ = name;
}

	void
RHeader::setType(HeaderType t)
{
	headerType_ = t;
}

	void
RHeader::setBody(RHeaderBody * b)
{
	headerBody_ = b;
}

	void
RHeader::parse()
{
	rmmDebug("parse() called");
	int split = strRep_.find(':');
	
	if (headerBody_ != 0) delete headerBody_;
	headerType_ = HeaderUnknown;
	
	if (split == -1) return;

	headerName_ = strRep_.left(split);
	headerName_ = headerName_.stripWhiteSpace();

	for (int i = 0; i < 42; i++) {
		if (!stricmp((headerName_), headerNames[i])) {
			headerType_ = (HeaderType)i;
			rmmDebug("I'm of type " + QString(headerNames[i]));
		}
	}

	if (headerType_ == HeaderUnknown) {
		rmmDebug("I'm an unknown header, \"" + headerName_ + "\"");
	}

	RHeaderBody * b;
	switch (headerTypesTable[headerType_]) {
		
		case Address:
			rmmDebug("b = new RAddress");
			b = new RAddress;
			break;
		
		case AddressList: 
			b = new RAddressList;
			break;

		case DateTime:
			b = new RDateTime;
			break;

		case DispositionType:
			b = new RDispositionType;
			break;

		case Mailbox:
			b = new RMailbox;
			break;

		case MailboxList:
			b = new RMailboxList;
			break;

		case Mechanism:
			b = new RMechanism;
			break;

		case MessageID:
			b = new RMessageID;
			break;

		case Text:
		default:
			rmmDebug("b = new RText");
			b = new RText;
			break;
	}

	QString hb = strRep_.right(strRep_.length() - split - 1);
	hb = hb.stripWhiteSpace();
	b->set(hb);
	b->parse();
	headerBody_ = b;

	rmmDebug("strRep == " + strRep_);
}

	void
RHeader::assemble()
{
	rmmDebug("assemble() called");
	
	if (headerType_ != HeaderUnknown)
		headerName_ = headerNames[headerType_];

	strRep_ = headerName_;
	strRep_ += ':';
	strRep_ += ' ';
	
	if (headerBody_ != 0) {
		headerBody_->assemble();
		strRep_ += headerBody_->asString();
	}

	rmmDebug("assembled to: \"" + strRep_ + "\"");
}

	void
RHeader::createDefault()
{
}

