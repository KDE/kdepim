#include <rmm/HeaderBody.h>

using namespace RMM;

HeaderBody::HeaderBody()
    :   MessageComponent()
{
    // Empty.
}

HeaderBody::~HeaderBody()
{
}

HeaderBody::HeaderBody(const HeaderBody & h)
    :   MessageComponent(h)
{
}

HeaderBody::HeaderBody(const QCString & s)
    :   MessageComponent(s)
{
    // Empty.
}

    HeaderBody &
HeaderBody::operator = (const QCString & s)
{
    MessageComponent::operator = (s);
    return *this;
}

    HeaderBody &
HeaderBody::operator = (const HeaderBody & h)
{
    if (this == &h) return *this;
    MessageComponent::operator = (h);
    return *this;
}

    bool
HeaderBody::operator == (HeaderBody &)
{
    return false;
}

    void
HeaderBody::_parse()
{
}

    void
HeaderBody::_assemble()
{
}

    void
HeaderBody::createDefault()
{
}

// vim:ts=4:sw=4:tw=78
