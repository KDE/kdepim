#include <qstring.h>

#include <RMM_Text.h>

using namespace RMM;

RText::RText()
    :    RHeaderBody()
{
    parsed_ = assembled_ = true;
}

RText::RText(const RText & r)
    :    RHeaderBody(r)
{
    parsed_ = assembled_ = true;
}

RText::RText(const QCString & s)
       :    RHeaderBody(s)
{
    parsed_ = assembled_ = true;
}

RText::~RText()
{
    // Empty.
}

    RText &
RText::operator = (const RText & r)
{
    if (this == &r) return *this; // Avoid a = a
    RHeaderBody::operator = (r);
    return *this;
}

    RText &
RText::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RText::operator == (RText & t)
{
    return (RHeaderBody::operator == (t));
}

    void
RText::_parse()
{
    // Empty.
}

    void
RText::_assemble()
{
    // Empty.
}

    void
RText::createDefault()
{
    // Empty.
}

// vim:ts=4:sw=4:tw=78
