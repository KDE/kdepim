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
	:	RMessageComponent()
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
	RBodyPartListIterator it(partList_);

	for (;it.current(); ++it)
		partList_.append(it.current());
	
	RMessageComponent::operator = (body);

	return *this;
}

	void
RBody::parse()
{
	rmmDebug("parse() called");
	partList_.clear();
	
	if (!isMultiPart_) {
		
		RBodyPart * newPart = new RBodyPart(strRep_);
		CHECK_PTR(newPart);
/*		newPart->setEncoding(cte_);
		newPart->setMimeType(mimeType_);
		newPart->setMimeSubType(mimeSubType_);
		newPart->setDescription(description_);
		newPart->setDisposition(disposition_);
		newPart->parse();
*/		
		return;
	}
	
	// So, dear message, you are of multiple parts. Let's see what you're made
	// of.
	
	// Start by looking for the first boundary. If there's data before it, then
	// that's the preamble.
	
	int i(strRep_.find(boundary_));
	
	if (i == -1) {
		// Argh ! This is supposed to be a multipart message, but there's not
		// even one boundary !
		// Let's just call it a plain text message.
		// 
		return;
	}
	
	preamble_ = strRep_.left(i);
	
	int oldi(i + boundary_.length());
	// Now do the rest of the parts.
	i = strRep_.find(boundary_, i + boundary_.length());
	
	while (i != -1) {
	
		// Looks like there's only one body part.
		RBodyPart * newPart = new RBodyPart(strRep_.mid(oldi, i));
		CHECK_PTR(newPart);
/*		newPart->setEncoding(cte_);
		newPart->setMimeType(mimeType_);
		newPart->setMimeSubType(mimeSubType_);
		newPart->setDescription(description_);
		newPart->setDisposition(disposition_);
*/
		oldi = i + boundary_.length();
		i = strRep_.find(boundary_, i + boundary_.length());
	}
	
	// No more body parts. Anything that's left is the epilogue.
	
	epilogue_ = strRep_.right(strRep_.length() - i + boundary_.length());
		
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

