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

	REnvelope &
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
	if (parsed_) return;
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
	
	parsed_		= true;
	assembled_	= false;
}

	void
REnvelope::assemble()
{
	parse();
	if (assembled_) return;
	rmmDebug("assemble() called");

	strRep_ = "";

	if (!has(RMM::HeaderTo))		_createDefault(RMM::HeaderTo);
	if (!has(RMM::HeaderMessageID))	_createDefault(RMM::HeaderMessageID);
	if (!has(RMM::HeaderFrom))		_createDefault(RMM::HeaderFrom);
	if (!has(RMM::HeaderDate))		_createDefault(RMM::HeaderDate);

	RHeaderListIterator it(headerList_);

	for (; it.current(); ++it) {
		rmmDebug(it.current()->className());
		it.current()->assemble();
		strRep_ += it.current()->asString();
		strRep_ += '\r';
		strRep_ += '\n';
	}
	assembled_ = true;
}

	void
REnvelope::_createDefault(RMM::HeaderType t)
{
	rmmDebug("Creating default of type " + QString(RMM::headerNames[t]));
	RHeader * h = new RHeader;
	h->setName(RMM::headerNames[t]);

	RHeaderBody * b;
	switch (RMM::headerTypesTable[t]) {

		case RMM::Address:
			b = new RAddress;
			break;

		case RMM::AddressList:
			b = new RAddressList;
			break;

		case RMM::DateTime:
			b = new RDateTime;
			break;

		case RMM::DispositionType:
			b = new RDispositionType;
			break;

		case RMM::Mailbox:
			b = new RMailbox;
			break;

		case RMM::MailboxList:
			b = new RMailboxList;
			break;

		case RMM::Mechanism:
			b = new RMechanism;
			break;

		case RMM::MessageID:
			b = new RMessageID;
			break;

		case RMM::Text:
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
	_createDefault(RMM::HeaderTo);
	rmmDebug("****** CREATING DEFAULT MESSAGE ID ******");
	_createDefault(RMM::HeaderMessageID);
	rmmDebug("****** CREATING DEFAULT FROM ******");
	_createDefault(RMM::HeaderFrom);
	rmmDebug("****** CREATING DEFAULT DATE ******");
	_createDefault(RMM::HeaderDate);
	
	parsed_		= true;
	assembled_	= true;
}

	bool
REnvelope::has(RMM::HeaderType t)
{
	parse();
	RHeaderListIterator it(headerList_);

	for (; it.current(); ++it)
		if (it.current()->headerType() == t) return true;

	return false;
}

	bool
REnvelope::has(const QCString & headerName)
{
	parse();
	RHeaderListIterator it(headerList_);

	for (; it.current(); ++it)
		if (it.current()->headerName() == headerName) return true;

	return false;
}

template <class T>
	T
REnvelope::get(RMM::HeaderType h, T t)
{
	parse();
	rmmDebug("get " + QCString(RMM::headerNames[h]));
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
	RMM::HeaderDataType hdt = RMM::headerTypesTable[h];

	RHeaderBody * d;

	switch (hdt) {
		case RMM::Address:
			d = new RAddress;
			break;
		case RMM::AddressList:
			d = new RAddressList;
			break;
		case RMM::DateTime:
			d = new RDateTime;
			break;
		case RMM::DispositionType:
			d = new RDispositionType;
			break;
		case RMM::Mailbox:
			d = new RMailbox;
			break;
		case RMM::MailboxList:
			d = new RMailboxList;
			break;
		case RMM::Mechanism:
			d = new RMechanism;
			break;
		case RMM::MessageID:
			d = new RMessageID;
			break;
		case RMM::Text:
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
	return *get(RMM::HeaderApproved, t);
}

	RAddressList &
REnvelope::bcc()
{
	RAddressList * t = 0;
	return *get(RMM::HeaderBcc, t);
}

	RMailboxList &
REnvelope::cc()
{
	RMailboxList * t = 0;
	return *get(RMM::HeaderCc, t);
}

	RText &
REnvelope::comments()
{
	RText * t = 0;
	return *get(RMM::HeaderComments, t);
}

	RText &
REnvelope::contentDescription()
{
	RText * t = 0;
	return *get(RMM::HeaderContentDescription, t);
}

	RDispositionType &
REnvelope::contentDisposition()
{
	RDispositionType * t = 0;
	return *get(RMM::HeaderContentDisposition, t);
}

	RMessageID &
REnvelope::contentID()
{
	RMessageID * t = 0;
	return *get(RMM::HeaderContentID, t);
}

	RText &
REnvelope::contentMD5()
{
	RText * t = 0;
	return *get(RMM::HeaderContentMD5, t);
}

	RContentType &
REnvelope::contentType()
{
	RContentType * t = 0;
	return *get(RMM::HeaderContentType, t);
}

	RText &
REnvelope::control()
{
	RText * t = 0;
	return *get(RMM::HeaderControl, t);
}

	RCte &
REnvelope::contentTransferEncoding()
{
	RCte * t = 0;
	return *get(RMM::HeaderContentTransferEncoding, t);
}

	RDateTime &
REnvelope::date()
{
	RDateTime * t = 0;
	return *get(RMM::HeaderDate, t);
}

	RText &
REnvelope::distribution()
{
	RText * t = 0;
	return *get(RMM::HeaderDistribution, t);
}

	RText &
REnvelope::encrypted()
{
	RText * t = 0;
	return *get(RMM::HeaderEncrypted, t);
}

	RDateTime &
REnvelope::expires()
{
	RDateTime * t = 0;
	return *get(RMM::HeaderExpires, t);
}

	RText &
REnvelope::followupTo()
{
	RText * t = 0;
	return *get(RMM::HeaderFollowupTo, t);
}

	RMailboxList &
REnvelope::from()
{
	RMailboxList * t = 0;
	return *get(RMM::HeaderFrom, t);
}

	RText &
REnvelope::inReplyTo()
{
	rmmDebug("inReplyTo() called");
	RText * t = 0;
	t = get(RMM::HeaderInReplyTo, t);
	rmmDebug("Returning from inReplyTo()");
	return *t;
}

	RText &
REnvelope::keywords()
{
	RText * t = 0;
	return *get(RMM::HeaderKeywords, t);
}

	RText &
REnvelope::lines()
{
	RText * t = 0;
	return *get(RMM::HeaderLines, t);
}

	RMessageID &
REnvelope::messageID()
{
	RMessageID * t = 0;
	return *get(RMM::HeaderMessageID, t);
}

	RText &
REnvelope::mimeVersion()
{
	RText * t = 0;
	return *get(RMM::HeaderMimeVersion, t);
}

	RText &
REnvelope::newsgroups()
{
	RText * t = 0;
	return *get(RMM::HeaderNewsgroups, t);
}

	RText &
REnvelope::organization()
{
	RText * t = 0;
	return *get(RMM::HeaderOrganization, t);
}

	RText &
REnvelope::path()
{
	RText * t = 0;
	return *get(RMM::HeaderPath, t);
}

	RText &
REnvelope::received()
{
	RText * t = 0;
	return *get(RMM::HeaderReceived, t);
}

	RText &
REnvelope::references()
{
	RText * t = 0;
	return *get(RMM::HeaderReferences, t);
}

	RAddressList &
REnvelope::replyTo()
{
	RAddressList * t = 0;
	return *get(RMM::HeaderReplyTo, t);
}

	RAddressList &
REnvelope::resentBcc()
{
	RAddressList * t = 0;
	return *get(RMM::HeaderResentBcc, t);
}

	RAddressList &
REnvelope::resentCc()
{
	RAddressList * t = 0;
	return *get(RMM::HeaderResentCc, t);
}

	RDateTime &
REnvelope::resentDate()
{
	RDateTime * t = 0;
	return *get(RMM::HeaderResentDate, t);
}

	RMailboxList &
REnvelope::resentFrom()
{
	RMailboxList * t = 0;
	return *get(RMM::HeaderResentFrom, t);
}

	RMessageID &
REnvelope::resentMessageID()
{
	RMessageID * t = 0;
	return *get(RMM::HeaderResentMessageID, t);
}

	RAddressList &
REnvelope::resentReplyTo()
{
	RAddressList * t = 0;
	return *get(RMM::HeaderResentReplyTo, t);
}

	RMailbox &
REnvelope::resentSender()
{
	RMailbox * t = 0;
	return *get(RMM::HeaderResentSender, t);
}

	RAddressList &
REnvelope::resentTo()
{
	RAddressList * t = 0;
	return *get(RMM::HeaderResentTo, t);
}

	RText &
REnvelope::returnPath()
{
	RText * t = 0;
	return *get(RMM::HeaderReturnPath, t);
}

	RMailbox &
REnvelope::sender()
{
	RMailbox * t = 0;
	return *get(RMM::HeaderSender, t);
}

	RText &
REnvelope::subject()
{
	RText * t = 0;
	return *get(RMM::HeaderSubject, t);
}

	RText &
REnvelope::summary()
{
	RText * t = 0;
	return *get(RMM::HeaderSummary, t);
}

	RAddressList &
REnvelope::to()
{
	RAddressList * t = 0;
	return *get(RMM::HeaderTo, t);
}

	RText &
REnvelope::xref()
{
	RText * t = 0;
	return *get(RMM::HeaderXref, t);
}

	RText &
REnvelope::get(const QCString & headerName)
{
	parse();
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

	RMailbox &
REnvelope::firstSender()
{
	parse();
	rmmDebug("firstSender() called");

	if (has(RMM::HeaderFrom)) {

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
	parse();
	// XXX If there's a references field, we use this over the InReplyTo: field.
	// This is a temporary policy decision and may change.

	rmmDebug("parentMessageId() called");

	RMessageID m;

	if (has(RMM::HeaderReferences)) {

		rmmDebug("Has header references");

		/// FIXME have to do set() as operator = seems to think operand is
		//RMessageID !
		QCString s = references().asString();
		s = s.right(s.length() - s.findRev('<'));
		m.set(s);

	} else if (has(RMM::HeaderInReplyTo)) {

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

