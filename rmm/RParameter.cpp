#include <qcstring.h>

#include <RMM_Defines.h>
#include <RMM_Token.h>
#include <RMM_Parameter.h>

using namespace RMM;

RParameter::RParameter()
    :    RMessageComponent()
{
    // Empty.
}

RParameter::RParameter(const RParameter & p)
    :   RMessageComponent(p),
        attribute_(p.attribute_),
        value_(p.value_)
{
    // Empty.
}

RParameter::RParameter(const QCString & s)
    :    RMessageComponent(s)
{
    // Empty.
}

RParameter::~RParameter()
{
    // Empty.
}

    RParameter &
RParameter::operator = (const QCString & s)
{
    RMessageComponent::operator = (s);
    return *this;
}

    RParameter &
RParameter::operator = (const RParameter & p)
{
    if (this == &p) return *this; // Don't do a = a.
    
    attribute_ = p.attribute_;
    value_ = p.value_;

    RMessageComponent::operator = (p);
    return *this;
}

    bool
RParameter::operator == (RParameter & p)
{
    parse();
    p.parse();

    return (
        attribute_    == p.attribute_ &&
        value_        == p.value_);
}
    
    void
RParameter::_parse()   
{
    int split = strRep_.find('=');
    
    if (split == -1) {
        rmmDebug("Invalid parameter");
        return;
    }
    
    attribute_    = strRep_.left(split).stripWhiteSpace();
    value_        = strRep_.right(strRep_.length() - split - 1).stripWhiteSpace();
    
    rmmDebug("attribute == \"" + attribute_ + "\"");
    rmmDebug("value     == \"" + value_ + "\"");
}

    void
RParameter::_assemble()
{
    strRep_ = attribute_ + "=" + value_;
}

    void
RParameter::createDefault()
{
    attribute_ = value_ = "";
}

    QCString
RParameter::attribute()
{
    parse();
    return attribute_;
}

    QCString
RParameter::value()
{
    parse();
    return value_;
}

    void
RParameter::setAttribute(const QCString & attribute)
{
    parse();
    attribute_ = attribute;
}
    void
RParameter::setValue(const QCString & value)    
{
    parse();
    value_ = value;
}

// vim:ts=4:sw=4:tw=78
