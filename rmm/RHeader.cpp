#include <RMM_Enum.h>
#include <RMM_Header.h>
#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_ContentType.h>
#include <RMM_Cte.h>
#include <RMM_AddressList.h>
#include <RMM_DateTime.h>
#include <RMM_ContentDisposition.h>
#include <RMM_Mechanism.h>
#include <RMM_MessageID.h>
#include <RMM_Text.h>

using namespace RMM;

RHeader::RHeader()
    :   RMessageComponent(),
        headerName_(""),
        headerType_(HeaderUnknown),
        headerBody_(0)
{
    // Empty.
}

RHeader::RHeader(const RHeader & h)
    :   RMessageComponent(h),
        headerName_(h.headerName_.copy()),
        headerType_(h.headerType_),
        headerBody_(0)
{
    _replaceHeaderBody(headerType_, h.headerBody_);   
}

RHeader::RHeader(const QCString & s)
    :   RMessageComponent(s),
        headerName_(""),
        headerType_(HeaderUnknown),
        headerBody_(0)
{
    // Empty.
}

RHeader::~RHeader()
{
    delete headerBody_;
    headerBody_ = 0;
}

    RHeader &
RHeader::operator = (const QCString & s)
{
    delete headerBody_;
    headerBody_ = 0;

    RMessageComponent::operator = (s);
    return *this;
}

    RHeader &
RHeader::operator = (const RHeader & h)
{
    if (this == &h) return *this;

    headerName_ = h.headerName_.copy();
    headerType_ = h.headerType_;

    _replaceHeaderBody(headerType_, h.headerBody_);
  
    RMessageComponent::operator = (h);
    return *this;
}

    bool
RHeader::operator == (RHeader & h)
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
RHeader::headerName()
{
    parse();
    return headerName_;
}

    HeaderType
RHeader::headerType()
{
    parse();
    return headerType_;
}

    RHeaderBody *
RHeader::headerBody()
{
    parse();
    return headerBody_;
}

    void
RHeader::setName(const QCString & name)
{
    parse();
    headerName_ = name.copy();
}

    void
RHeader::setType(HeaderType t)
{
    parse();
    headerType_ = t;
}

    void
RHeader::setBody(RHeaderBody * b)
{
    parse();
    headerBody_ = b;
}

    void
RHeader::_parse()
{
    int split = strRep_.find(':');

    if (split == -1)
        headerName_ = "Error";
    else
        headerName_ = strRep_.left(split).stripWhiteSpace();
    
    headerType_ = headerNameToEnum(headerName_);
    
    delete headerBody_;

    headerBody_ = _newHeaderBody(headerType_);
    *headerBody_ = strRep_.mid(split + 1).stripWhiteSpace();
}

    void
RHeader::_assemble()
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
RHeader::createDefault()
{
    // STUB
}

    RHeaderBody *
RHeader::_newHeaderBody(HeaderType headerType)
{    
    RHeaderBody * b = 0;

    if (headerType > HeaderUnknown) {
        rmmDebug("You passed me an illegal header type !");
        headerType = HeaderUnknown;
    }

    switch (headerTypesTable[headerType]) {
        case Address:               b = new RAddress;               break;
        case AddressList:           b = new RAddressList;           break;
        case ContentType:           b = new RContentType;           break;
        case Cte:                   b = new RCte;                   break;
        case DateTime:              b = new RDateTime;              break;
        case ContentDisposition:    b = new RContentDisposition;    break;
        case Mechanism:             b = new RMechanism;             break;
        case MessageID:             b = new RMessageID;             break;
        case Text: default:         b = new RText;                  break;
    }

    return b;
}

    void
RHeader::_replaceHeaderBody(HeaderType headerType, RHeaderBody * b)
{
    delete headerBody_;
    headerBody_ = 0;

    headerBody_ = _newHeaderBody(headerType);

    if (0 != b)
        *headerBody_ = *b;
}

// vim:ts=4:sw=4:tw=78
