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
	body_ = new RBody();
}

RBodyPart::RBodyPart(const RBodyPart & part)
	:	REntity(part)
{
	rmmDebug("ctor");
	body_ = new RBody();
}

RBodyPart::~RBodyPart()
{
	rmmDebug("dtor");
	delete body_;
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
	
	// A body part consists of an envelope and a body.
	// The body may, again, consist of multiple body parts.
	
	int endOfHeaders = strRep_.find(QRegExp("\n\n"));
	
	if (endOfHeaders == -1) {
		rmmDebug("No end of headers ! - message is " +
			QString().setNum(strRep_.length()) + " bytes long");
		return;
	}
	
	envelope_.set(strRep_.left(endOfHeaders));

	rmmDebug("Looking to see if there's a Content-Type header");
	// Now see if there's a Content-Type header in the envelope.
	// If there is, we might be looking at a multipart message.
	if (envelope_.has(RMM::HeaderContentType)) {
		
		rmmDebug("There's a Content-Type header");
		
		// It has the header.
		RContentType contentType(envelope_.contentType());
		
		// If the header say multipart, we'll need to know the boundary.
		if (contentType.type() == "multipart") {
			
			rmmDebug("This message is multipart");
		
			RParameterListIterator it(contentType.parameterList());
			
			for (; it.current(); ++it) {
			
				if (it.current()->attribute() == "boundary") {
				
					// We've found the boundary.
					// The body needs the boundary, so it knows where to split.
					// It also needs the content type header, so it knows what
					// subtype the contentType has.
					// It also needs the content-transfer-encoding, so that it
					// can decode where necessary.
					body_->setBoundary(it.current()->value());
					body_->setContentType(contentType);
					RCte cte(envelope_.contentTransferEncoding());
					body_->setCTE(cte);
					body_->setMultiPart(true);
					break;
				}
			}
		}
	}

	body_->set(strRep_.right(strRep_.length() - endOfHeaders));
	body_->parse();
	

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

