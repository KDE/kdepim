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

/*
	RMailMessages -	Class library for Internet Mail Messages.
					This library relies on the Qt toolkit (http://www.troll.no).
					Compliant with various RFCs. See docs for details.
	
	Copyright (C) 1998 Rik Hemsley <rik@kde.org>
	
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
#include <RMM_BodyPart.h>
#include <RMM_Body.h>
#include <RMM_Message.h>
#include <RMM_MimeType.h>
#include <RMM_Envelope.h>

RBody::RBody()
{
	rmmDebug("ctor");
	partList_.setAutoDelete(true);
}


RBody::RBody(const RBody & body)
	:	RBodyPartList(),
		RMessageComponent()
{
	rmmDebug("ctor");
	partList_.setAutoDelete(true);
}

RBody::~RBody()
{
	rmmDebug("dtor");
}

	const RBody &
RBody::operator = (const RBody & body)
{
	rmmDebug("operator =");
	partList_.clear();
	QListIterator<RBodyPart> it(partList_);

	for (;it.current(); ++it)
		partList_.append(it.current());
	
	RMessageComponent::operator = (body);

	return *this;
}

	void
RBody::parse()
{
	rmmDebug("parse() called");
	
	// If we're not a multipart message, leave here.
	if (!isMultiPart_) return;
	
	// So, dear message, you are of multiple parts. Let's see what you're made
	// of.
	
	partList_.clear();
	
	
	
}

	void
RBody::assemble()
{
	rmmDebug("assemble() called");
}	

	int
RBody::numberOfParts() const
{
	return partList_.count();
}

	void
RBody::addPart(RBodyPart * bp)
{
	partList_.append(bp);
}

	void
RBody::removePart(RBodyPart * part)
{
	partList_.remove(part);
}

	RBodyPart *
RBody::part(int index)
{
	rmmDebug("part(" + QCString().setNum(index) + ") called");
	if (partList_.count() < (unsigned)index) return 0;
	return partList_.at(index);
}

	void
RBody::createDefault()
{
	rmmDebug("createDefault() called");
	strRep_ = "Empty message";
}

