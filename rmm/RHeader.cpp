#include <RMM_Enum.h>
#include <RMM_Header.h>
#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_ContentType.h>
#include <RMM_Cte.h>
#include <RMM_AddressList.h>
#include <RMM_DateTime.h>
#include <RMM_DispositionType.h>
#include <RMM_Mechanism.h>
#include <RMM_MessageID.h>
#include <RMM_Text.h>

using namespace RMM;

RHeader::RHeader()
    :   RMessageComponent(),
        headerType_(HeaderUnknown),
        headerBody_(0)
{
    // Empty.
}

RHeader::RHeader(const RHeader & h)
    :   RMessageComponent(h),
        headerName_(h.headerName_),
        headerType_(h.headerType_),
        headerBody_(0)
{
    // XXX necessary?
    // parse();
    headerBody_ = _copyHeaderBody(headerType_, h.headerBody_);   
}

RHeader::~RHeader()
{
    delete headerBody_;
    headerBody_ = 0;
}

RHeader::RHeader(const QCString & s)
    :   RMessageComponent(s),
        headerType_(HeaderUnknown),
        headerBody_(0)
{
    // Empty.
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

    headerName_ = h.headerName_;
    headerType_ = h.headerType_;

    delete headerBody_;
    headerBody_ = 0;
    headerBody_ = _copyHeaderBody(headerType_, h.headerBody_);
  
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
    headerName_ = name;
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

    delete headerBody_;
    headerBody_ = 0;
    headerType_ = HeaderUnknown;

    if (split == -1) {
        rmmDebug("No split ?");
        headerBody_ = new RText;
        return;
    }

    headerName_ = strRep_.left(split);
    headerName_ = headerName_.stripWhiteSpace();
    
    headerType_ = headerNameToEnum(headerName_);
    headerBody_ = _newHeaderBody(headerType_);

    *headerBody_ = strRep_.mid(split + 1).stripWhiteSpace();

    // XXX Is this necessary ?
    // headerBody_->parse();
}

    void
RHeader::_assemble()
{
    if ((int)headerType_ > 42)
        headerType_ = HeaderUnknown;
    
    if (headerType_ != HeaderUnknown) 
        headerName_ = headerNames[headerType_];

    strRep_ = headerName_;
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
        case Address:           b = new RAddress;           break;
        case AddressList:       b = new RAddressList;       break;
        case ContentType:       b = new RContentType;       break;
        case Cte:               b = new RCte;               break;
        case DateTime:          b = new RDateTime;          break;
        case DispositionType:   b = new RDispositionType;   break;
        case Mechanism:         b = new RMechanism;         break;
        case MessageID:         b = new RMessageID;         break;
        case Text: default:     b = new RText;              break;
    }

    return b;
}

    RHeaderBody *
RHeader::_copyHeaderBody(HeaderType headerType, RHeaderBody * headerBody)
{    
    if (headerBody == 0)
        return 0;

    RHeaderBody * b = 0;

    if (headerType > HeaderUnknown) {
        rmmDebug("You passed me an illegal header type !");
        headerType = HeaderUnknown;
    }

    switch (headerTypesTable[headerType]) {
        case Address:           
            b = new RAddress(*(RAddress *)headerBody);                 break;
        case AddressList:       
            b = new RAddressList(*(RAddressList *)headerBody);         break;
        case ContentType:       
            b = new RContentType(*(RContentType *)headerBody);         break;
        case Cte:               
            b = new RCte(*(RCte *)headerBody);                         break;
        case DateTime:          
            b = new RDateTime(*(RDateTime *)headerBody);               break;
        case DispositionType:   
            b = new RDispositionType(*(RDispositionType *)headerBody); break;
        case Mechanism:         
            b = new RMechanism(*(RMechanism *)headerBody);             break;
        case MessageID:         
            b = new RMessageID(*(RMessageID *)headerBody);             break;
        case Text: 
        default:     
            b = new RText(*(RText *)headerBody);                       break;
    }

    return b;
}

// vim:ts=4:sw=4:tw=78
