#include <RMM_Mechanism.h>
#include <RMM_Enum.h>
#include <RMM_Defines.h>

using namespace RMM;

RMechanism::RMechanism()
{
    // Empty.
}

RMechanism::RMechanism(const RMechanism & m)
    :    RHeaderBody(m)
{
    // Empty.
}

RMechanism::RMechanism(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}

RMechanism::~RMechanism()
{
    // Empty.
}

    RMechanism &
RMechanism::operator = (const RMechanism & m)
{
    if (this == &m) return *this; // Don't do a = a.
    
    RHeaderBody::operator = (m);
    return *this;
}

    RMechanism &
RMechanism::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RMechanism::operator == (RMechanism & m)
{
    parse();
    m.parse();

    return (RHeaderBody::operator == (m));
}

    void
RMechanism::_parse()
{
    // STUB
}

    void
RMechanism::_assemble()
{
    // STUB
}

    void
RMechanism::createDefault()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
