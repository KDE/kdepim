#include <qstring.h>

#include <rmm/Text.h>

using namespace RMM;

Text::Text()
    :    HeaderBody()
{
    parsed_ = assembled_ = true;
}

Text::Text(const Text & r)
    :    HeaderBody(r)
{
    parsed_ = assembled_ = true;
}

Text::Text(const QCString & s)
    :    HeaderBody(s)
{
    parsed_ = assembled_ = true;
}

Text::~Text()
{
    // Empty.
}

    Text &
Text::operator = (const Text & r)
{
    if (this == &r) return *this; // Avoid a = a
    HeaderBody::operator = (r);
    return *this;
}

    Text &
Text::operator = (const QCString & s)
{
    HeaderBody::operator = (s);
    return *this;
}

    bool
Text::operator == (Text & t)
{
    return (HeaderBody::operator == (t));
}

    void
Text::_parse()
{
    // Empty.
}

    void
Text::_assemble()
{
    // Empty.
}

    void
Text::createDefault()
{
    // Empty.
}

// vim:ts=4:sw=4:tw=78
