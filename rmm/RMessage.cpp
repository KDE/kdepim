#include <RMM_Defines.h>
#include <RMM_Message.h>

using namespace RMM;

RMessage::RMessage()
    :    RBodyPart()
{
    // Empty.
}

RMessage::RMessage(const RMessage & m)
    :    RBodyPart(m)
{
    // Empty.
}

RMessage::RMessage(const QCString & s)
    :    RBodyPart(s)
{
    // Empty.
}

RMessage::~RMessage()
{
    // Empty.
}

    MessageStatus
RMessage::status()
{
    return status_;
}

    void
RMessage::setStatus(MessageStatus s)
{
    status_ = s;
}

    QDataStream &
operator << (QDataStream & str, RMessage & m)
{
    str << m.asString(); return str;
}

    RMessage &
RMessage::operator = (const RMessage & m)
{
    if (this == &m) return *this;    // Avoid a = a.
    RBodyPart::operator = (m);
    return *this;
}

    RMessage &
RMessage::operator = (const QCString & s)
{
    RBodyPart::operator = (s);
    return *this;
}

    bool
RMessage::operator == (RMessage & m)
{
    parse();
    m.parse();

    return (RBodyPart::operator == (m));
}

    void
RMessage::_parse()
{
    RBodyPart::_parse();
}

    void
RMessage::_assemble()
{
    RBodyPart::_assemble();
}

    void
RMessage::createDefault()
{
    RBodyPart::createDefault();
}

    void
RMessage::addPart(RBodyPart &)
{
    // STUB
}

    void
RMessage::removePart(RBodyPart &)
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
