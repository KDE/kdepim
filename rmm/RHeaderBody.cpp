#include <qstring.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>

using namespace RMM;

RHeaderBody::RHeaderBody()
    :    RMessageComponent()
{
    // Empty.
}

RHeaderBody::RHeaderBody(const RHeaderBody & headerBody)
    :    RMessageComponent(headerBody)
{
    // Empty.
}

RHeaderBody::RHeaderBody(const QCString & s)
    :    RMessageComponent(s)
{
    // Empty.
}

    RHeaderBody &
RHeaderBody::operator = (const RHeaderBody & hb)
{
    if (this == &hb) return *this;
    
    strRep_ = hb.strRep_;    
    
    RMessageComponent::operator = (hb);
    return *this;
}

    RHeaderBody &
RHeaderBody::operator = (const QCString & s)
{
    RMessageComponent::operator = (s);
    return *this;
}

    bool
RHeaderBody::operator == (RHeaderBody & hb)
{
    return (RMessageComponent::operator == (hb));
}


RHeaderBody::~RHeaderBody()
{
    // Empty.
}

    void
RHeaderBody::_parse()
{
    rmmDebug("WARNING PARSE CALLED");
}

    void
RHeaderBody::_assemble()
{
    rmmDebug("WARNING ASSEMBLE CALLED");
}

    void
RHeaderBody::createDefault()
{
    rmmDebug("WARNING CREATEDEFAULT CALLED");
}
// vim:ts=4:sw=4:tw=78
