#include <rmm/Enum.h>
#include <rmm/Header.h>
#include <rmm/Address.h>
#include <rmm/AddressList.h>
#include <rmm/ContentType.h>
#include <rmm/Cte.h>
#include <rmm/AddressList.h>
#include <rmm/DateTime.h>
#include <rmm/ContentDisposition.h>
#include <rmm/Mechanism.h>
#include <rmm/MessageID.h>
#include <rmm/Text.h>

using namespace RMM;

Header::Header()
    :   MessageComponent(),
        headerName_(""),
        headerType_(HeaderUnknown),
        headerBody_(0)
{
    // Empty.
}

Header::Header(const Header & h)
    :   MessageComponent(h),
        headerName_(h.headerName_.copy()),
        headerType_(h.headerType_),
        headerBody_(0)
{
    _replaceHeaderBody(headerType_, h.headerBody_);   
}

Header::Header(const QCString & s)
    :   MessageComponent(s),
        headerName_(""),
        headerType_(HeaderUnknown),
        headerBody_(0)
{
    // Empty.
}

Header::~Header()
{
    delete headerBody_;
    headerBody_ = 0;
}

    Header &
Header::operator = (const QCString & s)
{
    delete headerBody_;
    headerBody_ = 0;

    MessageComponent::operator = (s);
    return *this;
}

    Header &
Header::operator = (const Header & h)
{
    if (this == &h) return *this;

    headerName_ = h.headerName_.copy();
    headerType_ = h.headerType_;

    _replaceHeaderBody(headerType_, h.headerBody_);
  
    MessageComponent::operator = (h);
    return *this;
}

    bool
Header::operator == (Header & h)
{
    parse();
    h.parse();
    
    if (headerBody_ != 0 && h.headerBody_ != 0)
        return (
            *headerBody_    == *h.headerBody_   &&
            headerName_     == h.headerName_    &&
            headerType_     == h.headerType_);
    
    if (headerBody_ == 0 && h.headerBody_ == 0)
        return (
            headerName_ == h.headerName_ &&
            headerType_ == h.headerType_);
    
    return false;
}

    QCString
Header::headerName()
{
    parse();
    return headerName_;
}

    HeaderType
Header::headerType()
{
    parse();
    return headerType_;
}

    HeaderBody *
Header::headerBody()
{
    parse();
    return headerBody_;
}

    void
Header::setName(const QCString & name)
{
    parse();
    headerName_ = name.copy();
}

    void
Header::setType(HeaderType t)
{
    parse();
    headerType_ = t;
}

    void
Header::setBody(HeaderBody * b)
{
    parse();
    headerBody_ = b;
}

    void
Header::_parse()
{
    int split = strRep_.find(':');

    if (split == -1)
        headerName_ = "Error";
    else
        headerName_ = strRep_.left(split).stripWhiteSpace();
    
    headerType_ = headerNameToType(headerName_);
    
    delete headerBody_;

    headerBody_ = _newHeaderBody(headerType_);
    *headerBody_ = strRep_.mid(split + 1).stripWhiteSpace();
}

    void
Header::_assemble()
{
    if ((int)headerType_ > 42)
        headerType_ = HeaderUnknown;
    
    if (headerType_ != HeaderUnknown) 
        headerName_ = headerNames[headerType_];

    strRep_ = headerName_.copy();
    strRep_ += ':';
    strRep_ += ' ';

    if (headerBody_ != 0) {
        strRep_ += headerBody_->asString();
    } else {
        rmmDebug("headerBody is 0 !!!!");
    }
}

    void
Header::createDefault()
{
    // STUB
}

    HeaderBody *
Header::_newHeaderBody(HeaderType headerType)
{    
    HeaderBody * b = 0;

    if (headerType > HeaderUnknown) {
        rmmDebug("You passed me an illegal header type !");
        headerType = HeaderUnknown;
    }

    switch (headerTypesTable[headerType]) {
        case ClassAddress:              b = new Address;            break;
        case ClassAddressList:          b = new AddressList;        break;
        case ClassContentType:          b = new ContentType;        break;
        case ClassCte:                  b = new Cte;                break;
        case ClassDateTime:             b = new DateTime;           break;
        case ClassContentDisposition:   b = new ContentDisposition; break;
        case ClassMechanism:            b = new Mechanism;          break;
        case ClassMessageID:            b = new MessageID;          break;
        case ClassText: default:        b = new Text;               break;
    }

    return b;
}

    void
Header::_replaceHeaderBody(HeaderType headerType, HeaderBody * b)
{
    delete headerBody_;
    headerBody_ = 0;

    headerBody_ = _newHeaderBody(headerType);

    if (0 != b)
        *headerBody_ = *b;
}

// vim:ts=4:sw=4:tw=78
