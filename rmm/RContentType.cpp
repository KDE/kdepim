#include <qstring.h>

#include <RMM_ContentType.h>
#include <RMM_Token.h>
#include <RMM_Defines.h>

using namespace RMM;

RContentType::RContentType()
    :    RHeaderBody()
{
    // Empty.
}

RContentType::RContentType(const RContentType & cte)
    :    RHeaderBody(cte),
        type_(cte.type_),
        subType_(cte.subType_),
        parameterList_(cte.parameterList_)
{
    // Empty.
}

RContentType::RContentType(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}

RContentType::~RContentType()
{
    // Empty.
}

    RContentType &
RContentType::operator = (const RContentType & ct)
{
    if (this == &ct) return *this; // Don't do a = a.

    type_            = ct.type_;
    subType_        = ct.subType_;
    parameterList_    = ct.parameterList_;
    
    RHeaderBody::operator = (ct);
    
    return *this;
}

    RContentType &
RContentType::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RContentType::operator == (RContentType & ct)
{
    parse();
    ct.parse();
    
    return (
        type_            == ct.type_        &&
        subType_        == ct.subType_    &&
        parameterList_    == ct.parameterList_);
}

    void
RContentType::_parse()
{
    QCString ts;
    int i = strRep_.find(";");
    
    if (i == -1)
    
        ts = strRep_;
    
    else {
    
        ts = strRep_.left(i);
        parameterList_ = strRep_.right(strRep_.length() - i - 1);
        parameterList_.parse();
    }
    
    int slash = ts.find('/');
    
    if (slash == -1) {
        rmmDebug("Invalid Content-Type");
        return;
    }
    
    type_        = ts.left(slash).stripWhiteSpace();
    subType_    = ts.right(ts.length() - slash - 1).stripWhiteSpace();
    
    rmmDebug("type_ == " + type_);
    rmmDebug("subType_ == " + subType_);
}

    void
RContentType::_assemble()
{
    strRep_ = type_ + "/" + subType_;
    
    parameterList_.assemble();
    
    if (parameterList_.list().count() == 0) return;
    
    strRep_ += QCString(";\n    ");
    
    strRep_ += parameterList_.asString();
}

    void
RContentType::createDefault()
{
    type_ = "text";
    subType_ = "plain";
    parsed_        = true;
    assembled_    = false;
}

    void
RContentType::setType(const QCString & t)
{
    parse();
    type_ = t;
}

    void
RContentType::setSubType(const QCString & t)
{
    parse();
    subType_ = t;
}

    void
RContentType::setParameterList(RParameterList & p)
{
    parse();
    parameterList_ = p;
}
    
    QCString
RContentType::type()
{
    parse();
    return type_;
}

    QCString
RContentType::subType()
{
    parse();
    return subType_;
}
    
    RParameterList &
RContentType::parameterList()    
{
    parse();
    return parameterList_;
}

// vim:ts=4:sw=4:tw=78
