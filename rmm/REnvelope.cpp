/*
    Empath - Mailer for KDE

    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org

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

#ifdef __GNUG__
# pragma implementation "RMM_Envelope.h"
#endif

#include <qlist.h>

#include <RMM_HeaderList.h>
#include <RMM_Envelope.h>
#include <RMM_Enum.h>

using namespace RMM;

REnvelope::REnvelope()
    :   RMessageComponent()
{
    rmmDebug("ctor");
//    headerList_.setAutoDelete(true);
}

REnvelope::REnvelope(const REnvelope & e)
    :    RMessageComponent(e),
        headerList_(e.headerList_)
{
    rmmDebug("copy ctor");
//    headerList_.setAutoDelete(true);
}

REnvelope::REnvelope(const QCString & s)
    :    RMessageComponent(s)
{
    rmmDebug("ctor with QCString(" + s + ")");
//    headerList_.setAutoDelete(true);
}

    REnvelope &
REnvelope::operator = (const REnvelope & e)
{
    rmmDebug("operator = (const REnvelope &e)");
    if (this == &e) return *this; // Don't do a = a.
    headerList_ = e.headerList_;
    RMessageComponent::operator = (e);
    return *this;
}

    REnvelope &
REnvelope::operator = (const QCString & s)
{
    rmmDebug("operator = (const QCString&)");
    RMessageComponent::operator = (s);
    parsed_ = false;
    assembled_ = false;
    return *this;
}

    bool
REnvelope::operator == (REnvelope & e)
{
    parse();
    e.parse();

    return false; // XXX: Need to write this...
}

REnvelope::~REnvelope()
{
    rmmDebug("dtor");
}

    void
REnvelope::_parse()
{
    const char * c = strRep_.data();
    const char * end = (char *)(c + strlen(c));
    char * r = new char[1024]; // rfc821 -> max length 1000 ? Can't remember.
    char * rstart = r;

    while (c < end) {

        // We don't want non-printable chars, apart from \n and \t.
        // The header's supposed to be 7 bit us-ascii. I'm not going to handle
        // backspaces, no matter what rfc822 says. That's just crazy.
        // If you want to do fancy chars in a header, you must quote them.

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
            r - rstart == 1020) {

            if (c == end - 1) *r++ = *c++;
            *r = '\0'; // NUL-terminate buffer.

            if (r != rstart) { // Only use buffer if it's not empty.

                QCString s(rstart);

                rmmDebug("New header: \"" + s + "\"");
                RHeader * h = new RHeader(s);
                h->parse();
                headerList_.append(h);
            }

            r = rstart;
            ++c;
            continue;
        }

        *r++ = *c++;
    }

    delete [] rstart;
}

    void
REnvelope::_assemble()
{
    strRep_ = "";

    RHeaderListIterator it(headerList_);

    for (; it.current(); ++it) {
        strRep_ += it.current()->asString();
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

        case Address:           b = new RAddress;           break;
        case AddressList:       b = new RAddressList;       break;
        case ContentType:       b = new RContentType;       break;
        case Cte:               b = new RCte;               break;
        case DateTime:          b = new RDateTime;          break;
        case DispositionType:   b = new RDispositionType;   break;
        case Mailbox:           b = new RMailbox;           break;
        case MailboxList:       b = new RMailboxList;       break;
        case Mechanism:         b = new RMechanism;         break;
        case MessageID:         b = new RMessageID;         break;
        case Text: default:     b = new RText;              break;
    }

    b->createDefault();
    h->setBody(b);
    headerList_.append(h);
}

    void
REnvelope::createDefault()
{
    parsed_        = false; 
    assembled_    = false;
}

    bool
REnvelope::has(HeaderType t)
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
        if (stricmp(it.current()->headerName(), s) == 0)
            return it.current();
    
    return 0;
}

    RHeaderBody *
REnvelope::get(HeaderType h)
{
    parse();
    rmmDebug("get " + QCString(headerNames[h]));
    // See if we can find this header in the list.

    RHeaderListIterator it(headerList_);

    for (; it.current(); ++it)
        if (it.current()->headerType() == h)
            return it.current()->headerBody();

    // else make a new one, set it to default values, and return that.

    rmmDebug("Creating a new item as there wasn't one existing.");

    RHeader * hdr = new RHeader(headerNames[h] + ":");
    headerList_.append(hdr);
    return hdr->headerBody();
}

    RText
REnvelope::approved()
{ return *(RText *)get(HeaderApproved); }

    RAddressList 
REnvelope::bcc()
{ return *(RAddressList *)get(HeaderBcc); }

    RMailboxList
REnvelope::cc()
{ return *(RMailboxList *)get(HeaderCc); }

    RText 
REnvelope::comments()
{ return *(RText *)get(HeaderComments); }

    RText 
REnvelope::contentDescription()
{ return *(RText *)get(HeaderContentDescription); }

    RDispositionType 
REnvelope::contentDisposition()
{ return *(RDispositionType *)(HeaderContentDisposition); }

    RMessageID 
REnvelope::contentID()
{ return *(RMessageID *)get(HeaderContentID); }

    RText 
REnvelope::contentMD5()
{ return *(RText *)get(HeaderContentMD5); }

    RContentType 
REnvelope::contentType()
{ return *(RContentType *)get(HeaderContentType); }

    RText 
REnvelope::control()
{ return *(RText *)get(HeaderControl); }

    RCte 
REnvelope::contentTransferEncoding()
{ return *(RCte *)get(HeaderContentTransferEncoding); }

    RDateTime 
REnvelope::date()
{ return *(RDateTime *)get(HeaderDate); }

    RText 
REnvelope::distribution()
{ return *(RText *)get(HeaderDistribution); }

    RText 
REnvelope::encrypted()
{ return *(RText *)get(HeaderEncrypted); }

    RDateTime 
REnvelope::expires()
{ return *(RDateTime *)get(HeaderExpires); }

    RText 
REnvelope::followupTo()
{ return *(RText *)get(HeaderFollowupTo); }

    RMailboxList 
REnvelope::from()
{ return *(RMailboxList *)get(HeaderFrom); }

    RText 
REnvelope::inReplyTo()
{ return *(RText *)get(HeaderInReplyTo); }

    RText 
REnvelope::keywords()
{ return *(RText *)get(HeaderKeywords); }

    RText 
REnvelope::lines()
{ return *(RText *)get(HeaderLines); }

    RMessageID 
REnvelope::messageID()
{ return *(RMessageID *)get(HeaderMessageID); }

    RText 
REnvelope::mimeVersion()
{ return *(RText *)get(HeaderMimeVersion); }

    RText 
REnvelope::newsgroups()
{ return *(RText *)get(HeaderNewsgroups); }

    RText 
REnvelope::organization()
{ return *(RText *)get(HeaderOrganization); }

    RText 
REnvelope::path()
{ return *(RText *)get(HeaderPath); }

    RText 
REnvelope::received()
{ return *(RText *)get(HeaderReceived); }

    RText 
REnvelope::references()
{ return *(RText *)get(HeaderReferences); }

    RAddressList 
REnvelope::replyTo()
{ return *(RAddressList *)get(HeaderReplyTo); }

    RAddressList 
REnvelope::resentBcc()
{ return *(RAddressList *)get(HeaderResentBcc); }

    RAddressList 
REnvelope::resentCc()
{ return *(RAddressList *)get(HeaderResentCc); }

    RDateTime 
REnvelope::resentDate()
{ return *(RDateTime *)get(HeaderResentDate); }

    RMailboxList 
REnvelope::resentFrom()
{ return *(RMailboxList *)get(HeaderResentFrom); }

    RMessageID 
REnvelope::resentMessageID()
{ return *(RMessageID *)get(HeaderResentMessageID); }

    RAddressList 
REnvelope::resentReplyTo()
{ return *(RAddressList *)get(HeaderResentReplyTo); }

    RMailbox 
REnvelope::resentSender()
{ return *(RMailbox *)get(HeaderResentSender); }

    RAddressList 
REnvelope::resentTo()
{ return *(RAddressList *)get(HeaderResentTo); }

    RText 
REnvelope::returnPath()
{ return *(RText *)get(HeaderReturnPath); }

    RMailbox 
REnvelope::sender()
{ return *(RMailbox *)get(HeaderSender); }

    RText 
REnvelope::subject()
{ return *(RText *)get(HeaderSubject); }

    RText 
REnvelope::summary()
{ return *(RText *)get(HeaderSummary); }

    RAddressList 
REnvelope::to()
{ return *(RAddressList *)get(HeaderTo); }

    RText 
REnvelope::xref()
{ return *(RText *)get(HeaderXref); }

    RMailbox
REnvelope::firstSender()
{
    parse();
    rmmDebug("firstSender() called");

    if (!has(HeaderFrom))
        return sender();
    
    RMailbox m;
    
    rmmDebug("firstSender: Checking if from count is 0");
    if (from().count() == 0) {
        rmmDebug("firstSender: count is 0");
        return m;
    }
    
    return *(from().at(0));
}

    RMessageID
REnvelope::parentMessageId()
{
    parse();
    // XXX If there's a references field, we use this over the InReplyTo: field.
    // This is a temporary policy decision and may change.

    RMessageID m;

    if (has(HeaderReferences)) {

        QCString s = references().asString();
        s = s.right(s.length() - s.findRev('<'));
        m = s;

    } else if (has(HeaderInReplyTo)) {

        RText t = inReplyTo();
        m = t.asString();

    } else {

        m.setLocalPart("");
        m.setDomain("");
    }

    return m;
}

    void
REnvelope::set(HeaderType t, const QCString & s)
{
    parse();
    RHeaderListIterator it(headerList_);

    for (; it.current(); ++it)
        if (it.current()->headerType() == t)
            headerList_.remove(it.current());

    HeaderDataType hdt = headerTypesTable[t];

    RHeaderBody * d;

    switch (hdt) {

        case Address:            d = new RAddress;            break;
        case AddressList:        d = new RAddressList;        break;
        case DateTime:            d = new RDateTime;            break;
        case DispositionType:    d = new RDispositionType;    break;
        case Mailbox:            d = new RMailbox;            break;
        case MailboxList:        d = new RMailboxList;        break;
        case Mechanism:            d = new RMechanism;            break;
        case MessageID:            d = new RMessageID;            break;
        case Text: default:        d = new RText;                break;
    }
    
    CHECK_PTR(d);
    
    *d = s;

    RHeader * hdr = new RHeader;

    hdr->setType(t);
    hdr->setBody(d);

    headerList_.append(hdr);
    assembled_ = false;
}

#if 0
    void
REnvelope::set(const QCString & headerName, const QCString & s)
{
    RHeaderListIterator it(headerList_);

    for (; it.current(); ++it)
        if (!stricmp(it.current()->headerName(), headerName))
            headerList_.remove(it.current());
    

}
#endif

    void
REnvelope::addHeader(RHeader h)
{
    parse();
    RHeader * newHeader = new RHeader(h);
    headerList_.append(newHeader);
    assembled_ = false;
}

    void
REnvelope::addHeader(const QCString & s)
{
    parse();
    RHeader * newHeader = new RHeader(s);
    headerList_.append(newHeader);
    assembled_ = false;
}

// vim:ts=4:sw=4:tw=78
