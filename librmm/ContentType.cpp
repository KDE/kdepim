#include <qstring.h>

#include <rmm/ContentType.h>
#include <rmm/Token.h>
#include <rmm/Defines.h>

using namespace RMM;

ContentType::ContentType()
    :    HeaderBody()
{
    // Empty.
}

ContentType::ContentType(const ContentType & cte)
    :   HeaderBody(cte),
        type_(cte.type_.copy()),
        subType_(cte.subType_.copy()),
        parameterList_(cte.parameterList_)
{
    // Empty.
}

ContentType::ContentType(const QCString & s)
    :    HeaderBody(s)
{
    // Empty.
}

ContentType::~ContentType()
{
    // Empty.
}

    ContentType &
ContentType::operator = (const ContentType & ct)
{
    if (this == &ct) return *this; // Don't do a = a.

    type_           = ct.type_.copy();
    subType_        = ct.subType_.copy();
    parameterList_  = ct.parameterList_;
    
    HeaderBody::operator = (ct);
    
    return *this;
}

    ContentType &
ContentType::operator = (const QCString & s)
{
    HeaderBody::operator = (s);
    return *this;
}

    bool
ContentType::operator == (ContentType & ct)
{
    parse();
    ct.parse();
    
    return (
        type_            == ct.type_        &&
        subType_        == ct.subType_    &&
        parameterList_    == ct.parameterList_);
}

    void
ContentType::_parse()
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
//        rmmDebug("Invalid Content-Type `" + ts + "'");
        return;
    }
    
    type_        = ts.left(slash).stripWhiteSpace();
    subType_    = ts.right(ts.length() - slash - 1).stripWhiteSpace();
    
    //rmmDebug("type_ == " + type_);
    //rmmDebug("subType_ == " + subType_);
}

    void
ContentType::_assemble()
{
    strRep_ = type_ + "/" + subType_;
    
    if (parameterList_.list().count() == 0) return;
    
    strRep_ += QCString(";\n    ");
    
    strRep_ += parameterList_.asString();
}

    void
ContentType::createDefault()
{
    type_ = "text";
    subType_ = "plain";
    parsed_        = true;
    assembled_    = false;
}

    void
ContentType::setType(const QCString & t)
{
    parse();
    type_ = t.copy();
}

    void
ContentType::setSubType(const QCString & t)
{
    parse();
    subType_ = t.copy();
}

    void
ContentType::setParameterList(ParameterList & p)
{
    parse();
    parameterList_ = p;
}
    
    QCString
ContentType::type()
{
    parse();
    return type_;
}

    QCString
ContentType::subType()
{
    parse();
    return subType_;
}
    
    RMM::ParameterList &
ContentType::parameterList()    
{
    parse();
    return parameterList_;
}

// vim:ts=4:sw=4:tw=78
