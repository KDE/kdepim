#include <qlist.h>

#include <RMM_HeaderList.h>
#include <RMM_Envelope.h>
#include <RMM_Enum.h>

using namespace RMM;

REnvelope::REnvelope()
    :   RMessageComponent()
{
    // Empty.
}

REnvelope::REnvelope(const REnvelope & e)
    :    RMessageComponent(e)
{
    headerList_  = e.headerList_;
}

REnvelope::REnvelope(const QCString & s)
    :    RMessageComponent(s)
{
    // Empty.
}

    REnvelope &
REnvelope::operator = (const REnvelope & e)
{
    if (this == &e) return *this; // Don't do a = a.

    headerList_ = e.headerList_;

    RMessageComponent::operator = (e);
    return *this;
}

    REnvelope &
REnvelope::operator = (const QCString & s)
{
    headerList_.clear();
    RMessageComponent::operator = (s);
    return *this;
}

    bool
REnvelope::operator == (REnvelope & e)
{
    parse();
    e.parse();

    return false; // TODO
}

REnvelope::~REnvelope()
{
    // Empty.
}

    void
REnvelope::_parse()
{
    const char * c = strRep_.data();
    const char * end = c + strlen(c);
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

            if (r != rstart) // Only use buffer if it's not empty.
                headerList_.append(RHeader(QCString(rstart)));

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

    RHeaderListIterator it(headerList_.begin());

    for (; it != headerList_.end(); ++it)
        strRep_ += (*it).asString() + '\n';
}

    void
REnvelope::_createDefault(HeaderType t)
{
    if (t <= HeaderUnknown)
        headerList_.append(RHeader(QCString(headerNames[t]) + ":"));
}

    void
REnvelope::createDefault()
{
    // STUB
}

    bool
REnvelope::has(HeaderType t)
{
    parse();

    RHeaderListIterator it(headerList_.begin());

    for (; it != headerList_.end(); ++it) {

        if ((*it).headerType() == t) {
            return true;
        }
    }

    return false;
}

    bool
REnvelope::has(const QCString & headerName)
{
    parse();

    RHeaderListIterator it(headerList_.begin());

    for (; it != headerList_.end(); ++it) {

        if (0 == stricmp((*it).headerName(), headerName)) {
            return true;
        }
    }

    return false;
}

    RHeader
REnvelope::get(const QCString & s)
{
    parse();

    RHeaderListIterator it(headerList_.begin());

    for (; it != headerList_.end(); ++it)
        if (0 == stricmp((*it).headerName(), s))
            return (*it);

    RHeader h(s + ":");

    headerList_.append(h);

    return h;
}

    RHeaderBody *
REnvelope::get(HeaderType t)
{
    parse();

    RHeaderListIterator it(headerList_.begin());

    for (; it != headerList_.end(); ++it)
        if ((*it).headerType() == t)
            return (*it).headerBody();

    RHeader h(headerNames[t] + ":");

    headerList_.append(h);

    return h.headerBody();
}

    RMessageID
REnvelope::parentMessageId()
{
    parse();
    // Note: If there's a references field, we use this over the InReplyTo:
    // field. This is a temporary policy decision and may change.

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

    RHeader h(s);

    RHeaderListIterator it(headerList_.begin());

    for (; it != headerList_.end(); ++it)
        if (0 == stricmp((*it).headerName(), h.headerName())) {
            (*it) = s;
            break;
        }
}

    void
REnvelope::set(const QCString & headerName, const QCString & s)
{
    parse();

    RHeaderListIterator it(headerList_.begin());

    for (; it != headerList_.end(); ++it)
        if (0 == stricmp((*it).headerName(), headerName)) {
            (*it) = headerName + ": " + s;
            break;
        }
}

    void
REnvelope::addHeader(RHeader h)
{
    parse();
    headerList_.append(h);
}

    void
REnvelope::addHeader(const QCString & s)
{
    parse();
    headerList_.append(RHeader(s));
}

     RAddress
REnvelope::firstSender()
{
    parse();
    return (has(HeaderFrom) ? from().at(0) : sender());
}

   RText
REnvelope::approved()
{ return *static_cast<RText *>(get(HeaderApproved)); }

    RAddressList 
REnvelope::bcc()
{ return *static_cast<RAddressList *>(get(HeaderBcc)); }

    RAddressList
REnvelope::cc()
{ return *static_cast<RAddressList *>(get(HeaderCc)); }

    RText 
REnvelope::comments()
{ return *static_cast<RText *>(get(HeaderComments)); }

    RText 
REnvelope::contentDescription()
{ return *static_cast<RText *>(get(HeaderContentDescription)); }

    RContentDisposition
REnvelope::contentDisposition()
{ return *static_cast<RContentDisposition *>(get(HeaderContentDisposition)); }

    RMessageID 
REnvelope::contentID()
{ return *static_cast<RMessageID *>(get(HeaderContentID)); }

    RText 
REnvelope::contentMD5()
{ return *static_cast<RText *>(get(HeaderContentMD5)); }

    RContentType 
REnvelope::contentType()
{ return *static_cast<RContentType *>(get(HeaderContentType)); }

    RText 
REnvelope::control()
{ return *static_cast<RText *>(get(HeaderControl)); }

    RCte 
REnvelope::contentTransferEncoding()
{ return *static_cast<RCte *>(get(HeaderContentTransferEncoding)); }

    RDateTime 
REnvelope::date()
{ return *static_cast<RDateTime *>(get(HeaderDate)); }

    RText 
REnvelope::distribution()
{ return *static_cast<RText *>(get(HeaderDistribution)); }

    RText 
REnvelope::encrypted()
{ return *static_cast<RText *>(get(HeaderEncrypted)); }

    RDateTime 
REnvelope::expires()
{ return *static_cast<RDateTime *>(get(HeaderExpires)); }

    RText 
REnvelope::followupTo()
{ return *static_cast<RText *>(get(HeaderFollowupTo)); }

    RAddressList 
REnvelope::from()
{ return *static_cast<RAddressList *>(get(HeaderFrom)); }

    RText 
REnvelope::inReplyTo()
{ return *static_cast<RText *>(get(HeaderInReplyTo)); }

    RText 
REnvelope::keywords()
{ return *static_cast<RText *>(get(HeaderKeywords)); }

    RText 
REnvelope::lines()
{ return *static_cast<RText *>(get(HeaderLines)); }

    RMessageID 
REnvelope::messageID()
{ return *static_cast<RMessageID *>(get(HeaderMessageID)); }

    RText 
REnvelope::mimeVersion()
{ return *static_cast<RText *>(get(HeaderMimeVersion)); }

    RText 
REnvelope::newsgroups()
{ return *static_cast<RText *>(get(HeaderNewsgroups)); }

    RText 
REnvelope::organization()
{ return *static_cast<RText *>(get(HeaderOrganization)); }

    RText 
REnvelope::path()
{ return *static_cast<RText *>(get(HeaderPath)); }

    RText 
REnvelope::received()
{ return *static_cast<RText *>(get(HeaderReceived)); }

    RText 
REnvelope::references()
{ return *static_cast<RText *>(get(HeaderReferences)); }

    RAddressList 
REnvelope::replyTo()
{ return *static_cast<RAddressList *>(get(HeaderReplyTo)); }

    RAddressList 
REnvelope::resentBcc()
{ return *static_cast<RAddressList *>(get(HeaderResentBcc)); }

    RAddressList 
REnvelope::resentCc()
{ return *static_cast<RAddressList *>(get(HeaderResentCc)); }

    RDateTime 
REnvelope::resentDate()
{ return *static_cast<RDateTime *>(get(HeaderResentDate)); }

    RAddressList 
REnvelope::resentFrom()
{ return *static_cast<RAddressList *>(get(HeaderResentFrom)); }

    RMessageID 
REnvelope::resentMessageID()
{ return *static_cast<RMessageID *>(get(HeaderResentMessageID)); }

    RAddressList 
REnvelope::resentReplyTo()
{ return *static_cast<RAddressList *>(get(HeaderResentReplyTo)); }

    RAddress
REnvelope::resentSender()
{ return *static_cast<RAddress *>(get(HeaderResentSender)); }

    RAddressList 
REnvelope::resentTo()
{ return *static_cast<RAddressList *>(get(HeaderResentTo)); }

    RText 
REnvelope::returnPath()
{ return *static_cast<RText *>(get(HeaderReturnPath)); }

    RAddress
REnvelope::sender()
{ return *static_cast<RAddress *>(get(HeaderSender)); }

    RText 
REnvelope::subject()
{ return *static_cast<RText *>(get(HeaderSubject)); }

    RText 
REnvelope::summary()
{ return *static_cast<RText *>(get(HeaderSummary)); }

    RAddressList 
REnvelope::to()
{ return *static_cast<RAddressList *>(get(HeaderTo)); }

    RText 
REnvelope::xref()
{ return *static_cast<RText *>(get(HeaderXref)); }


// vim:ts=4:sw=4:tw=78
