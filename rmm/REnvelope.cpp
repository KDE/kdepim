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

#include <qlist.h>

#include <RMM_HeaderList.h>
#include <RMM_Envelope.h>
#include <RMM_Enum.h>

REnvelope::REnvelope()
{
	rmmDebug("ctor");
	headerList_.setAutoDelete(true);
}

REnvelope::REnvelope(const REnvelope & e)
	:	RMessageComponent()
{
	rmmDebug("ctor");
	headerList_.setAutoDelete(true);
}

	const REnvelope &
REnvelope::operator = (const REnvelope & e)
{
	rmmDebug("operator =");
    if (this == &e) return *this; // Don't do a = a.
	headerList_ = e.headerList_;
	rmmDebug(".");
	RMessageComponent::operator = (e);
	return *this;
}

REnvelope::~REnvelope()
{
	rmmDebug("dtor");
}

	void
REnvelope::parse()
{
	rmmDebug("parse() called");
	rmmDebug("strRep_ : " + strRep_);

	const char * c = strRep_.data();
	const char * start = c;
	const char * end = (char *)(c + strlen(c));
	char * r = new char[1024]; // rfc821 -> max length 1000 ? Can't remember.
	char * rstart = r;
	
	while (c < end) {
		
		// We don't want non-printable chars, apart from \n and \t.
		// The header's supposed to be 7 bit us-ascii. I'm not going to handle
		// backspaces, no matter what rfc822 says. That's just crazy. If you want
		// to do fancy chars in a header, you must quote them.
		 
		if ((*c != '\n' && *c != '\t' && *c < 32) || *c == 127) {
			rmmDebug("Invalid char in header");
			++c;
			continue;
		}

		if (*c == '\r') {
			++c;
			continue;
		}

		if ((*c == '\n' && (c != end - 1) && (c[1] != ' ') && (c[1] != '\t')) ||
			(c == end - 1) ||
			r - start == 1024) {
			
			if (c == end - 1) *r++ = *c++;
			*r = '\0'; // NUL-terminate buffer.
			
			if (r != rstart) { // Only use buffer if it's not empty.
			
				QCString s(rstart);
				
				rmmDebug("New header: \"" + s + "\"");
				RHeader * h = new RHeader(s);
				headerList_.append(h);
				rmmDebug(".");
			}
				
			r = rstart;
			++c;
			continue;
		}
		
		*r++ = *c++;
	}
	
	delete rstart;
	
	RHeaderListIterator it(headerList_);
	
	for (; it.current(); ++it)
		it.current()->parse();
}

	void
REnvelope::assemble()
{
	rmmDebug("assemble() called");

	strRep_ = "";

	if (!has(HeaderTo))			_createDefault(HeaderTo);
	if (!has(HeaderMessageID))	_createDefault(HeaderMessageID);
	if (!has(HeaderFrom))		_createDefault(HeaderFrom);
	if (!has(HeaderDate))		_createDefault(HeaderDate);

	RHeaderListIterator it(headerList_);

	for (; it.current(); ++it) {
		rmmDebug(it.current()->className());
		it.current()->assemble();
		strRep_ += it.current()->asString();
		strRep_ += '\r';
		strRep_ += '\n';
	}
}

	void
REnvelope::_createDefault(HeaderType t)
{
	rmmDebug("Creating default of type " + QCString(headerNames[t]));
	RHeader * h = new RHeader;
	h->setName(headerNames[t]);

	RHeaderBody * b;
	switch (headerTypesTable[t]) {
		
		case Address:
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
			b = new RText;
			break;
	}

	b->createDefault();
	h->setBody(b);
	headerList_.append(h);
}

	void
REnvelope::createDefault()
{
	rmmDebug("****** CREATING DEFAULT TO ******");
	_createDefault(HeaderTo);
	rmmDebug("****** CREATING DEFAULT MESSAGE ID ******");
	_createDefault(HeaderMessageID);
	rmmDebug("****** CREATING DEFAULT FROM ******");
	_createDefault(HeaderFrom);
	rmmDebug("****** CREATING DEFAULT DATE ******");
	_createDefault(HeaderDate);
}

	bool
REnvelope::has(HeaderType t) const
{
	RHeaderListIterator it(headerList_);

	for (; it.current(); ++it)
		if (it.current()->headerType() == t) return true;

	return false;
}

	bool
REnvelope::has(const QCString & headerName) const
{
	RHeaderListIterator it(headerList_);

	for (; it.current(); ++it)
		if (it.current()->headerName() == headerName) return true;

	return false;
}

template <class T>
	T
REnvelope::get(HeaderType h, T t)
{
	rmmDebug("get " + QCString(headerNames[h]));
	// See if we can find this header in the list.
	
	RHeaderListIterator it(headerList_);
	
	for (; it.current(); ++it)
		if (it.current()->headerType() == h) {
			rmmDebug("The header you asked for exists.");
			rmmDebug("headerbody: \"" + QCString(it.current()->asString()) + "\"");
			return (T)(it.current()->headerBody());
		}
	
	// else make a new one, set it to default values, and return that.
	
	rmmDebug("Creating a new item as there wasn't one existing.");
	HeaderDataType hdt = headerTypesTable[h];

	RHeaderBody * d;

	switch (hdt) {
		case Address:
			d = new RAddress;
			break;
		case AddressList:
			d = new RAddressList;
			break;
		case DateTime:
			d = new RDateTime;
			break;
		case DispositionType:
			d = new RDispositionType;
			break;
		case Mailbox:
			d = new RMailbox;
			break;
		case MailboxList:
			d = new RMailboxList;
			break;
		case Mechanism:
			d = new RMechanism;
			break;
		case MessageID:
			d = new RMessageID;
			break;
		case Text:
		default:
			d = new RText;
			break;
	}
	
	CHECK_PTR(d);

	rmmDebug("Making that item a default item");
	d->createDefault();
	
	RHeader * hdr = new RHeader;
	
	hdr->setType(h);
	hdr->setBody(d);
	
	headerList_.append(hdr);
	
	rmmDebug("..done creating new item");
	return (T)d;

}

	RText &
REnvelope::approved()
{
	RText * t = 0;
	return *get(HeaderApproved, t);
}

	RAddressList &
REnvelope::bcc()
{
	RAddressList * t = 0;
	return *get(HeaderBcc, t);
}

	RMailboxList &
REnvelope::cc() 
{
	RMailboxList * t = 0;
	return *get(HeaderCc, t);
}

	RText &
REnvelope::comments() 
{
	RText * t = 0;
	return *get(HeaderComments, t);
}

	RText &
REnvelope::contentDescription() 
{
	RText * t = 0;
	return *get(HeaderContentDescription, t);
}

	RDispositionType &
REnvelope::contentDisposition() 
{
	RDispositionType * t = 0;
	return *get(HeaderContentDisposition, t);
}

	RMessageID &
REnvelope::contentID() 
{
	RMessageID * t = 0;
	return *get(HeaderContentID, t);
}

	RText &
REnvelope::contentMD5() 
{
	RText * t = 0;
	return *get(HeaderContentMD5, t);
}

	RText &
REnvelope::contentType() 
{
	RText * t = 0;
	return *get(HeaderContentType, t);
}

	RText &
REnvelope::control() 
{
	RText * t = 0;
	return *get(HeaderControl, t);
}

	RText &
REnvelope::contentTransferEncoding() 
{
	RText * t = 0;
	return *get(HeaderContentTransferEncoding, t);
}

	RDateTime &
REnvelope::date() 
{
	RDateTime * t = 0;
	return *get(HeaderDate, t);
}

	RText &
REnvelope::distribution() 
{
	RText * t = 0;
	return *get(HeaderDistribution, t);
}

	RText &
REnvelope::encrypted() 
{
	RText * t = 0;
	return *get(HeaderEncrypted, t);
}

	RDateTime &
REnvelope::expires() 
{
	RDateTime * t = 0;
	return *get(HeaderExpires, t);
}

	RText &
REnvelope::followupTo() 
{
	RText * t = 0;
	return *get(HeaderFollowupTo, t);
}

	RMailboxList &
REnvelope::from() 
{
	RMailboxList * t = 0;
	return *get(HeaderFrom, t);
}

	RText &
REnvelope::inReplyTo() 
{
	rmmDebug("inReplyTo() called");
	RText * t = 0;
	t = get(HeaderInReplyTo, t);
	rmmDebug("Returning from inReplyTo()");
	return *t;
}

	RText &
REnvelope::keywords() 
{
	RText * t = 0;
	return *get(HeaderKeywords, t);
}

	RText &
REnvelope::lines() 
{
	RText * t = 0;
	return *get(HeaderLines, t);
}

	RMessageID &
REnvelope::messageID() 
{
	RMessageID * t = 0;
	return *get(HeaderMessageID, t);
}

	RText &
REnvelope::mimeVersion() 
{
	RText * t = 0;
	return *get(HeaderMimeVersion, t);
}

	RText &
REnvelope::newsgroups() 
{
	RText * t = 0;
	return *get(HeaderNewsgroups, t);
}

	RText &
REnvelope::organization() 
{
	RText * t = 0;
	return *get(HeaderOrganization, t);
}

	RText &
REnvelope::path() 
{
	RText * t = 0;
	return *get(HeaderPath, t);
}

	RText &
REnvelope::received() 
{
	RText * t = 0;
	return *get(HeaderReceived, t);
}

	RText &
REnvelope::references() 
{
	RText * t = 0;
	return *get(HeaderReferences, t);
}

	RAddressList &
REnvelope::replyTo() 
{
	RAddressList * t = 0;
	return *get(HeaderReplyTo, t);
}

	RAddressList &
REnvelope::resentBcc() 
{
	RAddressList * t = 0;
	return *get(HeaderResentBcc, t);
}

	RAddressList &
REnvelope::resentCc() 
{
	RAddressList * t = 0;
	return *get(HeaderResentCc, t);
}

	RDateTime &
REnvelope::resentDate() 
{
	RDateTime * t = 0;
	return *get(HeaderResentDate, t);
}

	RMailboxList &
REnvelope::resentFrom() 
{
	RMailboxList * t = 0;
	return *get(HeaderResentFrom, t);
}

	RMessageID &
REnvelope::resentMessageID() 
{
	RMessageID * t = 0;
	return *get(HeaderResentMessageID, t);
}

	RAddressList &
REnvelope::resentReplyTo() 
{
	RAddressList * t = 0;
	return *get(HeaderResentReplyTo, t);
}

	RMailbox &
REnvelope::resentSender() 
{
	RMailbox * t = 0;
	return *get(HeaderResentSender, t);
}

	RAddressList &
REnvelope::resentTo() 
{
	RAddressList * t = 0;
	return *get(HeaderResentTo, t);
}

	RText &
REnvelope::returnPath() 
{
	RText * t = 0;
	return *get(HeaderReturnPath, t);
}

	RMailbox &
REnvelope::sender() 
{
	RMailbox * t = 0;
	return *get(HeaderSender, t);
}

	RText &
REnvelope::subject() 
{
	RText * t = 0;
	return *get(HeaderSubject, t);
}

	RText &
REnvelope::summary() 
{
	RText * t = 0;
	return *get(HeaderSummary, t);
}

	RAddressList &
REnvelope::to() 
{
	RAddressList * t = 0;
	return *get(HeaderTo, t);
}

	RText &
REnvelope::xref() 
{
	RText * t = 0;
	return *get(HeaderXref, t);
}

	RText &
REnvelope::get(const QCString & headerName)
{
	RHeaderListIterator it(headerList_);
	
	for (; it.current(); ++it) {
		if (stricmp(it.current()->headerName(),headerName))
			return *((RText *)(it.current()->headerBody()));
	}

	RText * d = new RText;
	d->createDefault();
	
	RHeader * hdr = new RHeader;
	
	hdr->setName(headerName);
	hdr->setBody(d);
	
	headerList_.append(hdr);
	
	return *d;
}

	const RMailbox &
REnvelope::firstSender()
{
	rmmDebug("firstSender() called");
	
	if (has(HeaderFrom)) {
	
		RMailboxList & m = from();
		rmmDebug("Number of mailboxes in from field : " +
			QCString().setNum(m.count()));

		if (m.count() == 0) {
			
			rmmDebug("Asking the mailbox list to create a default mailbox");
			m.createDefault();
			
		}
			
		rmmDebug(QCString("Returning ") + m.at(0)->asString());
		m.at(0)->assemble();
		return *(m.at(0));
	
	
	} else {

		sender().assemble();
		return sender();
	}
}

	RMessageID
REnvelope::parentMessageId()
{
	// XXX If there's a references field, we use this over the InReplyTo: field.
	// This is a temporary policy decision and may change.
	
	rmmDebug("parentMessageId() called");
	
	RMessageID m;

	if (has(HeaderReferences)) {
		
		rmmDebug("Has header references");
				
		/// FIXME have to do set() as operator = seems to think operand is
		//RMessageID !
		QCString s = references().asString();
		s = s.right(s.length() - s.findRev('<'));
		m.set(s);
		
	} else if (has(HeaderInReplyTo)) {
		
		rmmDebug("Has header in reply to");
	
		RText t = inReplyTo();
		rmmDebug("..");
		m.set(t.asString());
		
	} else {
		
		m.setLocalPart("");
		m.setDomain("");
	}
	
	rmmDebug("DOING PARSE");
	m.parse();

	return m;
}

