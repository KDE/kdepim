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
//	headerList_.setAutoDelete(true);
}

REnvelope::REnvelope(const REnvelope & e)
	:	RMessageComponent(e)
{
	rmmDebug("ctor");
//	headerList_.setAutoDelete(true);
}

	REnvelope &
REnvelope::operator = (const REnvelope & e)
{
	rmmDebug("operator =");
    if (this == &e) return *this; // Don't do a = a.
	headerList_ = e.headerList_;
	RMessageComponent::operator = (e);
	assembled_ = false;
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
				h->parse();
				headerList_.append(h);
				rmmDebug(".");
			}

			r = rstart;
			++c;
			continue;
		}

		*r++ = *c++;
	}

	delete [] rstart;

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

//	if (!has(RMM::HeaderTo))		_createDefault(RMM::HeaderTo);
//	if (!has(RMM::HeaderMessageID))	_createDefault(RMM::HeaderMessageID);
//	if (!has(RMM::HeaderFrom))		_createDefault(RMM::HeaderFrom);
//	if (!has(RMM::HeaderDate))		_createDefault(RMM::HeaderDate);

	RHeaderListIterator it(headerList_);

	for (; it.current(); ++it) {
		strRep_ += it.current()->asString();
		strRep_ += '\r';
		strRep_ += '\n';
	}
	assembled_ = true;
}

	void
REnvelope::_createDefault(RMM::HeaderType t)
{
	rmmDebug("Creating default of type " + QCString(RMM::headerNames[t]));
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
		if (!stricmp(it.current()->headerName(), headerName))
			return true;

	return false;
}

	RHeader *
REnvelope::get(const QCString & s)
{
	parse();
	RHeaderListIterator it(headerList_);

	for (; it.current(); ++it)
		if (!stricmp(it.current()->headerName(), s))
			return it.current();
	
	return 0;
}

	RHeaderBody *
REnvelope::get(RMM::HeaderType h)
{
	parse();
	rmmDebug("get " + QCString(RMM::headerNames[h]));
	// See if we can find this header in the list.

	RHeaderListIterator it(headerList_);

	for (; it.current(); ++it)
		if (it.current()->headerType() == h) {
			rmmDebug("The header you asked for exists.");
			rmmDebug("headerbody: \"" +
				QCString(it.current()->headerBody()->asString()) + "\"");
			return it.current()->headerBody();
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
	return d;
}

	RText
REnvelope::approved()
{
	return *(RText *)get(RMM::HeaderApproved);
}

	RAddressList 
REnvelope::bcc()
{
	return *(RAddressList *)get(RMM::HeaderBcc);
}

	RMailboxList
REnvelope::cc()
{
	return *(RMailboxList *)get(RMM::HeaderCc);
}

	RText 
REnvelope::comments()
{
	return *(RText *)get(RMM::HeaderComments);
}

	RText 
REnvelope::contentDescription()
{
	return *(RText *)get(RMM::HeaderContentDescription);
}

	RDispositionType 
REnvelope::contentDisposition()
{
	return *(RDispositionType *)(RMM::HeaderContentDisposition);
}

	RMessageID 
REnvelope::contentID()
{
	return *(RMessageID *)get(RMM::HeaderContentID);
}

	RText 
REnvelope::contentMD5()
{
	return *(RText *)get(RMM::HeaderContentMD5);
}

	RContentType 
REnvelope::contentType()
{
	return *(RContentType *)get(RMM::HeaderContentType);
}

	RText 
REnvelope::control()
{
	return *(RText *)get(RMM::HeaderControl);
}

	RCte 
REnvelope::contentTransferEncoding()
{
	return *(RCte *)get(RMM::HeaderContentTransferEncoding);
}

	RDateTime 
REnvelope::date()
{
	return *(RDateTime *)get(RMM::HeaderDate);
}

	RText 
REnvelope::distribution()
{
	return *(RText *)get(RMM::HeaderDistribution);
}

	RText 
REnvelope::encrypted()
{
	return *(RText *)get(RMM::HeaderEncrypted);
}

	RDateTime 
REnvelope::expires()
{
	return *(RDateTime *)get(RMM::HeaderExpires);
}

	RText 
REnvelope::followupTo()
{
	return *(RText *)get(RMM::HeaderFollowupTo);
}

	RMailboxList 
REnvelope::from()
{
	return *(RMailboxList *)get(RMM::HeaderFrom);
}

	RText 
REnvelope::inReplyTo()
{
	return *(RText *)get(RMM::HeaderInReplyTo);
}

	RText 
REnvelope::keywords()
{
	return *(RText *)get(RMM::HeaderKeywords);
}

	RText 
REnvelope::lines()
{
	return *(RText *)get(RMM::HeaderLines);
}

	RMessageID 
REnvelope::messageID()
{
	return *(RMessageID *)get(RMM::HeaderMessageID);
}

	RText 
REnvelope::mimeVersion()
{
	return *(RText *)get(RMM::HeaderMimeVersion);
}

	RText 
REnvelope::newsgroups()
{
	return *(RText *)get(RMM::HeaderNewsgroups);
}

	RText 
REnvelope::organization()
{
	return *(RText *)get(RMM::HeaderOrganization);
}

	RText 
REnvelope::path()
{
	return *(RText *)get(RMM::HeaderPath);
}

	RText 
REnvelope::received()
{
	return *(RText *)get(RMM::HeaderReceived);
}

	RText 
REnvelope::references()
{
	return *(RText *)get(RMM::HeaderReferences);
}

	RAddressList 
REnvelope::replyTo()
{
	return *(RAddressList *)get(RMM::HeaderReplyTo);
}

	RAddressList 
REnvelope::resentBcc()
{
	return *(RAddressList *)get(RMM::HeaderResentBcc);
}

	RAddressList 
REnvelope::resentCc()
{
	return *(RAddressList *)get(RMM::HeaderResentCc);
}

	RDateTime 
REnvelope::resentDate()
{
	return *(RDateTime *)get(RMM::HeaderResentDate);
}

	RMailboxList 
REnvelope::resentFrom()
{
	return *(RMailboxList *)get(RMM::HeaderResentFrom);
}

	RMessageID 
REnvelope::resentMessageID()
{
	return *(RMessageID *)get(RMM::HeaderResentMessageID);
}

	RAddressList 
REnvelope::resentReplyTo()
{
	return *(RAddressList *)get(RMM::HeaderResentReplyTo);
}

	RMailbox 
REnvelope::resentSender()
{
	return *(RMailbox *)get(RMM::HeaderResentSender);
}

	RAddressList 
REnvelope::resentTo()
{
	return *(RAddressList *)get(RMM::HeaderResentTo);
}

	RText 
REnvelope::returnPath()
{
	return *(RText *)get(RMM::HeaderReturnPath);
}

	RMailbox 
REnvelope::sender()
{
	return *(RMailbox *)get(RMM::HeaderSender);
}

	RText 
REnvelope::subject()
{
	return *(RText *)get(RMM::HeaderSubject);
}

	RText 
REnvelope::summary()
{
	return *(RText *)get(RMM::HeaderSummary);
}

	RAddressList 
REnvelope::to()
{
	return *(RAddressList *)get(RMM::HeaderTo);
}

	RText 
REnvelope::xref()
{
	return *(RText *)get(RMM::HeaderXref);
}

	RMailbox
REnvelope::firstSender()
{
	parse();
	rmmDebug("firstSender() called");

	if (!has(RMM::HeaderFrom))
		return sender();
	
	else {

		RMailboxList m(from());
		rmmDebug("Number of mailboxes in from field : " +
			QCString().setNum(m.count()));

		if (m.count() == 0) {

			rmmDebug("Asking the mailbox list to create a default mailbox");
			m.createDefault();

		}

		return *(m.at(0));
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
		m = s;

	} else if (has(RMM::HeaderInReplyTo)) {

		rmmDebug("Has header in reply to");

		RText t = inReplyTo();
		rmmDebug("..");
		m = t.asString();

	} else {

		m.setLocalPart("");
		m.setDomain("");
	}

	rmmDebug("DOING PARSE");
	m.parse();

	return m;
}

