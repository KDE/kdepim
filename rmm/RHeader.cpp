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

#ifdef __GNUG__
# pragma implementation "RMM_Header.h"
#endif

#include <RMM_Enum.h>
#include <RMM_Header.h>
#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_ContentType.h>
#include <RMM_Cte.h>
#include <RMM_AddressList.h>
#include <RMM_DateTime.h>
#include <RMM_DispositionType.h>
#include <RMM_Mailbox.h>
#include <RMM_MailboxList.h>
#include <RMM_Mechanism.h>
#include <RMM_MessageID.h>
#include <RMM_Text.h>

using namespace RMM;

RHeader::RHeader()
	:	RMessageComponent(),
		headerType_(HeaderUnknown),
		headerBody_(0)
{
	rmmDebug("ctor");
}

RHeader::RHeader(const RHeader & h)
	:	RMessageComponent(h),
		headerBody_(0)
{
	rmmDebug("copy ctor");
}

RHeader::~RHeader()
{
	rmmDebug("dtor");
	delete headerBody_;
	headerBody_ = 0;
}

RHeader::RHeader(const QCString & s)
	:	RMessageComponent(s),
		headerBody_(0)
{
	rmmDebug("ctor");
}

	RHeader &
RHeader::operator = (const QCString & s)
{
	rmmDebug("operator =");
	delete headerBody_;
	headerBody_ = 0;

	RMessageComponent::operator = (s);
	parsed_ = false;
	assembled_	= false;
	return *this;
}


	RHeader &
RHeader::operator = (const RHeader & h)
{
	rmmDebug("operator =");
	if (this == &h) return *this;

	headerName_ = h.headerName_;
	headerType_ = h.headerType_;

	delete headerBody_;
	headerBody_ = 0;
	
	headerBody_ = new RHeaderBody(*h.headerBody_);
	CHECK_PTR(headerBody_);
	
	RMessageComponent::operator = (h);
	parsed_ = true;
	assembled_	= false;
	return *this;
}

	bool
RHeader::operator == (RHeader & h)
{
	parse();
	h.parse();
	
	if (headerBody_ != 0 && h.headerBody_ != 0)
		return (
			*headerBody_	== *h.headerBody_	&&
			headerName_		== h.headerName_	&&
			headerType_		== h.headerType_);
	
	if (headerBody_ == 0 && h.headerBody_ == 0)
		return (
			headerName_ == h.headerName_ &&
			headerType_ == h.headerType_);
	
	return false;
}

	QCString
RHeader::headerName()
{
	parse();
	return headerName_;
}

	HeaderType
RHeader::headerType()
{
	parse();
	return headerType_;
}

	RHeaderBody *
RHeader::headerBody()
{
	parse();
	return headerBody_;
}

	void
RHeader::setName(const QCString & name)
{
	headerName_ = name;
	assembled_ = false;
}

	void
RHeader::setType(HeaderType t)
{
	headerType_ = t;
	assembled_ = false;
}

	void
RHeader::setBody(RHeaderBody * b)
{
	headerBody_ = b;
	assembled_ = false;
}

	void
RHeader::_parse()
{
	int split = strRep_.find(':');

	ASSERT(headerBody_ == 0);
	delete headerBody_;
	headerBody_ = 0;
	headerType_ = HeaderUnknown;

	if (split == -1) {
		rmmDebug("No split ?");
		headerBody_ = new RText;
		CHECK_PTR(headerBody_);
		return;
	}

	headerName_ = strRep_.left(split);
	headerName_ = headerName_.stripWhiteSpace();
	
	headerType_ = headerNameToEnum(headerName_);

	if (headerType_ == HeaderUnknown) {
		rmmDebug("I'm an unknown header, \"" + headerName_ + "\"");
	}

	RHeaderBody * b;
	switch (headerTypesTable[headerType_]) {
		case Address:			b = new RAddress;			break;
		case AddressList:		b = new RAddressList;		break;
		case ContentType:		b = new RContentType;		break;
		case Cte:				b = new RCte;				break;
		case DateTime:			b = new RDateTime;			break;
		case DispositionType:	b = new RDispositionType;	break;
		case Mailbox:			b = new RMailbox;			break;
		case MailboxList:		b = new RMailboxList;		break;
		case Mechanism:			b = new RMechanism;			break;
		case MessageID:			b = new RMessageID;			break;
		case Text: default:		b = new RText;				break;
	}
	CHECK_PTR(b);

	QCString hb = strRep_.right(strRep_.length() - split - 1);
	hb = hb.stripWhiteSpace();
	*b = hb;
	headerBody_ = b;
	headerBody_->parse();
}

	void
RHeader::_assemble()
{
	if ((int)headerType_ > 42)
		headerType_ = HeaderUnknown;
	
	if (headerType_ != HeaderUnknown) {
		headerName_ = headerNames[headerType_];
	}

	strRep_ = headerName_;
	strRep_ += ':';
	strRep_ += ' ';

	if (headerBody_ != 0) {
		strRep_ += headerBody_->asString();
	} else {
		rmmDebug("headerBody is 0 !!!!");
	}
}

	void
RHeader::createDefault()
{
}

