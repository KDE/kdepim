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
#include <qregexp.h>

// Local includes
#include <RMM_Utility.h>
#include <RMM_Body.h>
#include <RMM_BodyPart.h>
#include <RMM_Envelope.h>
#include <RMM_Message.h>
#include <RMM_Enum.h>

RBodyPart::RBodyPart()
	:	REntity()
{
	rmmDebug("ctor");
	body_.setAutoDelete(true);
}

RBodyPart::RBodyPart(const RBodyPart & part)
	:	REntity(part),
		envelope_			(part.envelope_),
		data_				(part.data_),
		body_				(part.body_),
		encoding_			(part.encoding_),
		mimeType_			(part.mimeType_),
		mimeSubType_		(part.mimeSubType_),
		contentDescription_	(part.contentDescription_),
		disposition_		(part.disposition_),
		boundary_			(part.boundary_),
		type_				(part.type_),
		preamble_			(part.preamble_),
		epilogue_			(part.epilogue_)
{
	rmmDebug("ctor");
	body_.setAutoDelete(true);
	body_ = part.body_;
}

RBodyPart::RBodyPart(const QCString & s)
	:	REntity(s)
{
	rmmDebug("ctor");
	body_.setAutoDelete(true);
	parsed_ = false;
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
	
	envelope_			= part.envelope_;
	data_				= part.data_;
	body_				= part.body_;
	encoding_			= part.encoding_;
	mimeType_			= part.mimeType_;
	mimeSubType_		= part.mimeSubType_;
	contentDescription_	= part.contentDescription_;
	disposition_		= part.disposition_;
	boundary_			= part.boundary_;
	type_				= part.type_;
	preamble_			= part.preamble_;
	epilogue_			= part.epilogue_;

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
	rmmDebug("parse() called");
	rmmDebug("=== RBodyPart parse start =====================================");
	
	body_.clear();
	
	// A body part consists of an envelope and a body.
	// The body may, again, consist of multiple body parts.
	
	int endOfHeaders = strRep_.find(QRegExp("\n\n"));
	
	if (endOfHeaders == -1) {
		
		// The body is blank. We'll treat what there is as the envelope.
		rmmDebug("empty body");
		envelope_	= strRep_;
		data_		= "";
		
	} else {
		
		rmmDebug("Setting envelope to:");
		rmmDebug(strRep_.left(endOfHeaders));
		envelope_	= strRep_.left(endOfHeaders);
		data_		= strRep_.right(strRep_.length() - endOfHeaders);
	}
	

	rmmDebug("Looking to see if there's a Content-Type header");
	// Now see if there's a Content-Type header in the envelope.
	// If there is, we might be looking at a multipart message.
	if (!envelope_.has(RMM::HeaderContentType)) {
		
		parsed_		= true;
		assembled_	= false;
		rmmDebug("done parse");
		rmmDebug("=== RBodyPart parse end   =================================");
		return;
	}

	rmmDebug("There's a Content-Type header");
	
	RContentType contentType(envelope_.contentType());
	
	rmmDebug("contentType.type() == " + contentType.type());
	
	// If the header say multipart, we'll need to know the boundary.
	if (!stricmp(contentType.type(), "multipart")) {
		
		rmmDebug(" ==== This part is multipart ========================");
	
		RParameterListIterator it(contentType.parameterList());
		
		rmmDebug("Looking for boundary");
		for (; it.current(); ++it) {
		
			if (!stricmp(it.current()->attribute(), "boundary"))
				boundary_ = it.current()->value();
		}
		
		rmmDebug("boundary == \"" + boundary_ + "\"");
		
		if (boundary_.isEmpty()) {
			parsed_		= true;
			assembled_	= false;
			return;
		}
		
		if (boundary_.at(0) == '\"') {
			boundary_.remove(boundary_.length() - 1, 1);
			boundary_.remove(0, 1);
		}
		
		if (boundary_.isEmpty()) {
			rmmDebug("The boundary is empty ! Get out ! Run away !");
			parsed_		= true;
			assembled_	= false;
			rmmDebug("done parse");
			rmmDebug("=== RBodyPart parse end   =============================");
			return;
		}

		int i(strRep_.find(boundary_, endOfHeaders));

		if (i == -1) {
			// Let's just call it a plain text message.
			rmmDebug("No boundary found in message. Assume plain ?");
			parsed_		= true;
			assembled_	= false;
			rmmDebug("done parse");
			rmmDebug("=== RBodyPart parse end   =============================");
			return;
		}

		preamble_ = strRep_.mid(endOfHeaders + 2, i - endOfHeaders);
		rmmDebug("preamble == \"" + preamble_ + "\"");

		int oldi(i + boundary_.length());
		// Now do the rest of the parts.
		i = strRep_.find(boundary_, i + boundary_.length());

		while (i != -1) {

			rmmDebug(" ==== Creating new part ========================");
			RBodyPart * newPart = new RBodyPart(strRep_.mid(oldi, i - oldi));
			CHECK_PTR(newPart);
			body_.append(newPart);
			rmmDebug(" ==== Parsing new part =========================");
			newPart->parse();
			oldi = i + boundary_.length();
			i = strRep_.find(boundary_, i + boundary_.length());
			rmmDebug(" ==== Created new part OK ======================");
		}

		// No more body parts. Anything that's left is the epilogue.

		epilogue_ =
			strRep_.right(strRep_.length() - i + boundary_.length());
		
		data_ = "";
			
	}

	mimeType_		= RMM::mimeTypeStr2Enum(contentType.type());
	mimeSubType_	= RMM::mimeSubTypeStr2Enum(contentType.subType());

	parsed_		= true;
	assembled_	= false;
	rmmDebug("done parse");
	rmmDebug("=== RBodyPart parse end   =====================================");
}

	void
RBodyPart::assemble()
{
	parse();
	if (assembled_) return;

	rmmDebug("assemble() called");

	strRep_ = envelope_.asString();
	strRep_ += "\n";
	strRep_ += preamble_;
	strRep_ += data_;
	strRep_ += epilogue_;

	assembled_ = true;
}

	void
RBodyPart::createDefault()
{
	envelope_.createDefault();
}

	REnvelope &
RBodyPart::envelope()
{
	parse();
	return envelope_;
}

	RBodyPartList &
RBodyPart::body()
{
	parse();
	return body_;
}

	Q_UINT32
RBodyPart::size()
{
	return data_.length();
}

	void
RBodyPart::_update()
{
//	type_ = (0 == 1) ? Basic : Mime;
}

	void
RBodyPart::addPart(RBodyPart * bp)
{
	parse();
	_update();
}

	void
RBodyPart::removePart(RBodyPart * part)
{
	parse();
	_update();
}

	RBodyPart::PartType
RBodyPart::type()
{
	parse();
	return type_;
}

	QCString
RBodyPart::data()
{
	parse();
	return data_;
}

	RMM::DispType
RBodyPart::disposition()
{
	parse();
	return disposition_;
}

	void
RBodyPart::setEnvelope(REnvelope e)
{
	rmmDebug("setEnvelope() called");
	parse();
	envelope_ = e;
	assembled_ = false;
}	

	void
RBodyPart::setData(const QCString & s)
{
	rmmDebug("setData() called");
	parse();
	data_ = s;
	assembled_ = false;
}

	void
RBodyPart::setBody(QList<RBodyPart> & b)
{
	rmmDebug("setBody() called");
	parse();
	body_ = b;
	assembled_ = false;
}

	RBodyPart
RBodyPart::decode()
{
	rmmDebug("decode()");
	return *this;
	REnvelope e;
	RBodyPart x;
	
	if (envelope_.has(RMM::Cte)) {
	
		switch (envelope_.contentTransferEncoding().mechanism()) {
			
			case RMM::CteTypeBase64:
				e = envelope_;
				e.set(RMM::HeaderContentTransferEncoding, "");
				x = e.asString() + RDecodeBase64(data_);
				break;

			case RMM::CteTypeQuotedPrintable:
				e = envelope_;
				e.set(RMM::HeaderContentTransferEncoding, "");
				x = e.asString() + RDecodeQuotedPrintable(data_);
				break;
				
			case RMM::CteType7bit:
			case RMM::CteType8bit:
			case RMM::CteTypeBinary:
			case RMM::CteTypeXtension:
			default:
				return *this;
				break;
		}

	} else {
		return *this;
	}

	return x;
}

