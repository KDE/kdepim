#include <RMM_Defines.h>
#include <RMM_MessageComponent.h>

using namespace RMM;

RMessageComponent::RMessageComponent()
    :   parsed_     (false),
        assembled_  (false)
{
    // Empty.
}

RMessageComponent::RMessageComponent(const RMessageComponent & mc)
    :   strRep_     (mc.strRep_),
        parsed_     (mc.parsed_),
        assembled_  (mc.assembled_)
{
    // Empty.
}

RMessageComponent::RMessageComponent(const QCString & s)
    :   strRep_(s),
        parsed_(false),
        assembled_(false)
{
    // Empty.
}

RMessageComponent::~RMessageComponent()
{
    // Empty.
}

    RMessageComponent &
RMessageComponent::operator = (const RMessageComponent & m)
{
    if (this == &m) return *this;    // Avoid a = a.
    assembled_  = m.assembled_;
    parsed_     = m.parsed_;
    strRep_     = m.strRep_;
    return *this;
}

    RMessageComponent &
RMessageComponent::operator = (const QCString & s)
{
    strRep_     = s;
    parsed_     = false;
    assembled_  = false;
    return *this;
}

    bool
RMessageComponent::operator == (RMessageComponent & mc)
{
    assemble();
    return (strRep_ == mc.asString());
}

    bool
RMessageComponent::operator == (const QCString & s)
{
    parse();
    return (strRep_ == s);
}

// vim:ts=4:sw=4:tw=78
