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
#include <RMM_Utility.h>
#include <RMM_Enum.h>

RBody::RBody()
	:	RMessageComponent()
{
	rmmDebug("ctor");
	partList_.setAutoDelete(true);
}


RBody::RBody(const RBody & body)
	:	RMessageComponent()
{
	rmmDebug("ctor");
	partList_.setAutoDelete(true);
	partList_	= body.partList_;
	assembled_	= false;
}

RBody::~RBody()
{
	rmmDebug("dtor");
}

	RBody &
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
	if (parsed_) return;
	
	partList_.clear();
	
	// We need to know what type of encoding we're looking at.

	RMM::CteType t(RMM::RCteStr2Enum(cte_.mechanism()));
	
	QCString decoded;
	
	switch (t) {
		
		case RMM::CteTypeQuotedPrintable:
			decoded = RDecodeQuotedPrintable(strRep_);
			break;
		
		case RMM::CteTypeBase64:
			decoded = RDecodeBase64(strRep_);
			break;
			
		case RMM::CteType7bit:
		case RMM::CteType8bit:
		case RMM::CteTypeBinary:
		default:
			decoded = strRep_;
			break;
	}
	
	
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
	
	parsed_		= true;
	assembled_	= false;
}

	void
RBody::assemble()
{
	rmmDebug("assemble() called");
	
	assembled_ = true;
}	

	int
RBody::numberOfParts()
{
	parse();
	return partList_.count();
}

	void
RBody::addPart(RBodyPart * bp)
{
	partList_.append(bp);
	assembled_ = false;
}

	void
RBody::removePart(RBodyPart * part)
{
	partList_.remove(part);
	assembled_ = false;
}

	RBodyPart
RBody::part(int index)
{
	return *(partList_.at(index));
}

	void
RBody::createDefault()
{
	rmmDebug("createDefault() called");
	strRep_ = "Empty message";
	parsed_		= true;
	assembled_	= true;
}

	void
RBody::setBoundary(const QCString & s)
{
	boundary_ = s;
	assembled_ = false;
}

	void
RBody::setContentType(RContentType & t)
{
	contentType_ = t;
	assembled_ = false;
}
	
	void
RBody::setCTE(RCte & t)
{
	cte_ = t; 
	assembled_ = false;
}

	void
RBody::setMultiPart(bool b)
{
	isMultiPart_ = b;
	assembled_ = false;
}

