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

/*
	RMailMessages -	Class library for Internet Mail Messages.
					This library relies on the Qt toolkit (http://www.troll.no).
					Compliant with various RFCs. See docs for details.
	
	Copyright (C) 1998 Rik Hemsley <rikkus@postmaster.co.uk>
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
 
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.
 
	You should have received a copy of the GNU General Public License
	along with this library; see the file COPYING.  If not, write to
	the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA. 
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
{
	rmmDebug("ctor");
}

RBodyPart::RBodyPart(const RBodyPart & part)
	:	REntity()
{
	rmmDebug("ctor");
}

RBodyPart::~RBodyPart()
{
	rmmDebug("dtor");
}

	const RBodyPart &
RBodyPart::operator = (const RBodyPart & part)
{
	if (this == &part) return *this;	// Avoid a = a.
	REntity::operator = (part);
	return *this;
}

	MimeType
RBodyPart::mimeType() const
{
	return mimeType_;
}

	MimeSubType
RBodyPart::mimeSubType() const
{
	return mimeSubType_;
}

	void
RBodyPart::setMimeType(MimeType t)
{
	mimeType_ = t;
}

	void
RBodyPart::setMimeSubType(MimeSubType st)
{
	mimeSubType_ = st;
}

	void
RBodyPart::setMimeType(const QString & s)
{
	mimeType_ = mimeTypeStr2Enum(s);
}

	void
RBodyPart::setMimeSubType(const QString & s)
{
	mimeSubType_ = mimeSubTypeStr2Enum(s);
}

	void
RBodyPart::parse()
{
}

	void
RBodyPart::assemble()
{
}

	void
RBodyPart::createDefault()
{
}

