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

// Qt includes
#include <qstring.h>

// Local includes
#include <RMM_Body.h>
#include <RMM_BodyPart.h>
#include <RMM_Envelope.h>
#include <RMM_Message.h>
#include <RMM_Enum.h>

RBodyPart::RBodyPart()
	:	REntity()
{
	rmmDebug("ctor");
}

RBodyPart::RBodyPart(const RBodyPart & part)
	:	REntity(part)
{
	rmmDebug("ctor");
}

RBodyPart::~RBodyPart()
{
	rmmDebug("dtor");
}

	RBodyPart &
RBodyPart::operator = (const RBodyPart & part)
{
	rmmDebug("operator =");
	if (this == &part) return *this;	// Avoid a = a.
	REntity::operator = (part);
	return *this;
}

	RMM::MimeType
RBodyPart::mimeType()
{
	parse();
	return mimeType_;
}

	RMM::MimeSubType
RBodyPart::mimeSubType()
{
	parse();
	return mimeSubType_;
}

	void
RBodyPart::setMimeType(RMM::MimeType t)
{
	mimeType_ = t;
	assembled_ = false;
}

	void
RBodyPart::setMimeSubType(RMM::MimeSubType st)
{
	mimeSubType_ = st;
	assembled_ = false;
}

	void
RBodyPart::setMimeType(const QCString & s)
{
	mimeType_ = RMM::mimeTypeStr2Enum(s);
	assembled_ = false;
}

	void
RBodyPart::setMimeSubType(const QCString & s)
{
	mimeSubType_ = RMM::mimeSubTypeStr2Enum(s);
	assembled_ = false;
}

	void
RBodyPart::parse()
{
	if (parsed_) return;
	
	parsed_		= true;
	assembled_	= false;
}

	void
RBodyPart::assemble()
{
	parse();
	if (assembled_) return;

	assembled_ = true;
}

	void
RBodyPart::createDefault()
{
}

