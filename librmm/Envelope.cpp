#include <qlist.h>

#include <rmm/Envelope.h>
#include <rmm/Enum.h>

using namespace RMM;

Envelope::Envelope()
    :   MessageComponent()
{
    _init();
}

Envelope::Envelope(const Envelope & e)
    :    MessageComponent(e)
{
    _init();
    _replaceHeaderList(e.headerList_);
}

Envelope::Envelope(const QCString & s)
    :    MessageComponent(s)
{
    _init();
}

    Envelope &
Envelope::operator = (const Envelope & e)
{
    if (this == &e) return *this; // Don't do a = a.

    _replaceHeaderList(e.headerList_);

    MessageComponent::operator = (e);
    return *this;
}

    Envelope &
Envelope::operator = (const QCString & s)
{
    MessageComponent::operator = (s);
    return *this;
}

    bool
Envelope::operator == (Envelope & e)
{
    parse();
    e.parse();

    return false; // TODO
}

Envelope::~Envelope()
{
    // Empty.
}

    void
Envelope::_init()
{
    headerList_.setAutoDelete(true);
}

    void
Envelope::_replaceHeaderList(const HeaderList & l)
{
    headerList_.clear();

    for (HeaderListIterator it(l); it.current(); ++it)
        headerList_.append(new Header(*it.current()));
}

    void
Envelope::_parse()
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
                headerList_.append(new Header(QCString(rstart)));

            r = rstart;
            ++c;
            continue;
        }

        *r++ = *c++;
    }

    delete [] rstart;
}

    void
Envelope::_assemble()
{
    strRep_ = "";

    for (HeaderListIterator it(headerList_); it.current(); ++it)
        strRep_ += it.current()->asString() + '\n';
}

    void
Envelope::_createDefault(HeaderType t)
{
    if (t <= HeaderUnknown)
        headerList_.append(new Header(QCString(headerNames[t]) + ":"));
}

    void
Envelope::createDefault()
{
    // STUB
}

    bool
Envelope::has(HeaderType t)
{
    parse();

    for (HeaderListIterator it(headerList_); it.current(); ++it)
        if (it.current()->headerType() == t)
            return true;

    return false;
}

    bool
Envelope::has(const QCString & headerName)
{
    parse();

    for (HeaderListIterator it(headerList_); it.current(); ++it)
        if (0 == qstricmp(it.current()->headerName(), headerName))
            return true;

    return false;
}

    RMM::Header
Envelope::get(const QCString & s)
{
    parse();

    for (HeaderListIterator it(headerList_); it.current(); ++it)
        if (0 == qstricmp(it.current()->headerName(), s))
            return *it.current();

    Header * h = new Header(s + ":");

    headerList_.append(h);

    return *h;
}

    HeaderBody *
Envelope::get(HeaderType t)
{
    parse();

    for (HeaderListIterator it(headerList_); it.current(); ++it)
        if (it.current()->headerType() == t)
            return it.current()->headerBody();

    Header * h = new Header(headerNames[t] + ":");

    headerList_.append(h);

    return h->headerBody();
}

    MessageID
Envelope::parentMessageId()
{
    parse();
    // Note: If there's a references field, we use this over the InReplyTo:
    // field. This is a temporary policy decision and may change.

    MessageID m;

    if (has(HeaderReferences)) {

        QCString s = references().asString();
        s = s.right(s.length() - s.findRev('<'));
        m = s;

    } else if (has(HeaderInReplyTo)) {

        Text t = inReplyTo();
        m = t.asString();

    } else {

        m.setLocalPart("");
        m.setDomain("");
    }

    return m;
}

    void
Envelope::set(HeaderType t, const QCString & s)
{
    parse();

    for (HeaderListIterator it(headerList_); it.current(); ++it)
        if (it.current()->headerType() == t) {
            *it.current() = s;
            break;
        }
}

    void
Envelope::set(const QCString & headerName, const QCString & s)
{
    parse();

    for (HeaderListIterator it(headerList_); it.current(); ++it)
        if (0 == qstricmp(it.current()->headerName(), headerName)) {
            *it.current() = headerName + ": " + s;
            break;
        }
}

    void
Envelope::addHeader(Header h)
{
    parse();
    headerList_.append(new Header(h));
}

    void
Envelope::addHeader(const QCString & s)
{
    parse();
    headerList_.append(new Header(s));
}

    RMM::Address
Envelope::firstSender()
{
    parse();
    return (has(HeaderFrom) ? from().at(0) : sender());
}

    RMM::Text
Envelope::approved()
{ return *static_cast<Text *>(get(HeaderApproved)); }

    RMM::AddressList 
Envelope::bcc()
{ return *static_cast<AddressList *>(get(HeaderBcc)); }

    RMM::AddressList
Envelope::cc()
{ return *static_cast<AddressList *>(get(HeaderCc)); }

    RMM::Text 
Envelope::comments()
{ return *static_cast<Text *>(get(HeaderComments)); }

    RMM::Text 
Envelope::contentDescription()
{ return *static_cast<Text *>(get(HeaderContentDescription)); }

    RMM::ContentDisposition
Envelope::contentDisposition()
{ return *static_cast<ContentDisposition *>(get(HeaderContentDisposition)); }

    RMM::MessageID 
Envelope::contentID()
{ return *static_cast<MessageID *>(get(HeaderContentID)); }

    RMM::Text 
Envelope::contentMD5()
{ return *static_cast<Text *>(get(HeaderContentMD5)); }

    RMM::ContentType 
Envelope::contentType()
{ return *static_cast<ContentType *>(get(HeaderContentType)); }

    RMM::Text 
Envelope::control()
{ return *static_cast<Text *>(get(HeaderControl)); }

    RMM::Cte 
Envelope::contentTransferEncoding()
{ return *static_cast<Cte *>(get(HeaderContentTransferEncoding)); }

    RMM::DateTime 
Envelope::date()
{ return *static_cast<DateTime *>(get(HeaderDate)); }

    RMM::Text 
Envelope::distribution()
{ return *static_cast<Text *>(get(HeaderDistribution)); }

    RMM::Text 
Envelope::encrypted()
{ return *static_cast<Text *>(get(HeaderEncrypted)); }

    RMM::DateTime 
Envelope::expires()
{ return *static_cast<DateTime *>(get(HeaderExpires)); }

    RMM::Text 
Envelope::followupTo()
{ return *static_cast<Text *>(get(HeaderFollowupTo)); }

    RMM::AddressList 
Envelope::from()
{ return *static_cast<AddressList *>(get(HeaderFrom)); }

    RMM::Text 
Envelope::inReplyTo()
{ return *static_cast<Text *>(get(HeaderInReplyTo)); }

    RMM::Text 
Envelope::keywords()
{ return *static_cast<Text *>(get(HeaderKeywords)); }

    RMM::Text 
Envelope::lines()
{ return *static_cast<Text *>(get(HeaderLines)); }

    RMM::MessageID 
Envelope::messageID()
{ return *static_cast<MessageID *>(get(HeaderMessageID)); }

    RMM::Text 
Envelope::mimeVersion()
{ return *static_cast<Text *>(get(HeaderMimeVersion)); }

    RMM::Text 
Envelope::newsgroups()
{ return *static_cast<Text *>(get(HeaderNewsgroups)); }

    RMM::Text 
Envelope::organization()
{ return *static_cast<Text *>(get(HeaderOrganization)); }

    RMM::Text 
Envelope::path()
{ return *static_cast<Text *>(get(HeaderPath)); }

    RMM::Text 
Envelope::received()
{ return *static_cast<Text *>(get(HeaderReceived)); }

    RMM::Text 
Envelope::references()
{ return *static_cast<Text *>(get(HeaderReferences)); }

    RMM::AddressList 
Envelope::replyTo()
{ return *static_cast<AddressList *>(get(HeaderReplyTo)); }

    RMM::AddressList 
Envelope::resentBcc()
{ return *static_cast<AddressList *>(get(HeaderResentBcc)); }

    RMM::AddressList 
Envelope::resentCc()
{ return *static_cast<AddressList *>(get(HeaderResentCc)); }

    RMM::DateTime 
Envelope::resentDate()
{ return *static_cast<DateTime *>(get(HeaderResentDate)); }

    RMM::AddressList 
Envelope::resentFrom()
{ return *static_cast<AddressList *>(get(HeaderResentFrom)); }

    RMM::MessageID 
Envelope::resentMessageID()
{ return *static_cast<MessageID *>(get(HeaderResentMessageID)); }

    RMM::AddressList 
Envelope::resentReplyTo()
{ return *static_cast<AddressList *>(get(HeaderResentReplyTo)); }

    RMM::Address
Envelope::resentSender()
{ return *static_cast<Address *>(get(HeaderResentSender)); }

    RMM::AddressList 
Envelope::resentTo()
{ return *static_cast<AddressList *>(get(HeaderResentTo)); }

    RMM::Text 
Envelope::returnPath()
{ return *static_cast<Text *>(get(HeaderReturnPath)); }

    RMM::Address
Envelope::sender()
{ return *static_cast<Address *>(get(HeaderSender)); }

    RMM::Text 
Envelope::subject()
{ return *static_cast<Text *>(get(HeaderSubject)); }

    RMM::Text 
Envelope::summary()
{ return *static_cast<Text *>(get(HeaderSummary)); }

    RMM::AddressList 
Envelope::to()
{ return *static_cast<AddressList *>(get(HeaderTo)); }

    RMM::Text 
Envelope::xref()
{ return *static_cast<Text *>(get(HeaderXref)); }


// vim:ts=4:sw=4:tw=78
