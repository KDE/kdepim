#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <qstring.h>
#include <qregexp.h>
#include <qstrlist.h>
#include <RMM_Defines.h>
#include <RMM_MessageID.h>
#include <RMM_Token.h>

using namespace RMM;

int RMessageID::seq_ = 0;

RMessageID::RMessageID()
    :    RHeaderBody()
{
    // Empty.
}

RMessageID::RMessageID(const RMessageID & messageID)
    :   RHeaderBody (messageID),
        localPart_  (messageID.localPart_.copy()),
        domain_     (messageID.domain_.copy())
{
    // Empty.
}

RMessageID::RMessageID(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}

RMessageID::~RMessageID()
{
    // Empty.
}

    bool
RMessageID::operator == (RMessageID & msgID)
{
    parse();
    msgID.parse();

    return (
        localPart_  == msgID.localPart_ &&
        domain_     == msgID.domain_);
}

    RMessageID &
RMessageID::operator = (const RMessageID & messageID)
{
    if (this == &messageID) return *this; // Avoid a = a
    
    localPart_  = messageID.localPart_.copy();
    domain_     = messageID.domain_.copy();
    
    RHeaderBody::operator = (messageID);
    
    return *this;
}

    RMessageID &
RMessageID::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}


    QDataStream &
RMM::operator >> (QDataStream & s, RMessageID & mid)
{
    s    >> mid.localPart_
        >> mid.domain_;
    mid.parsed_ = true;
    mid.assembled_ = false;
    return s;
}
        
    QDataStream &
RMM::operator << (QDataStream & s, RMessageID & mid)
{
    mid.parse();
    s    << mid.localPart_
        << mid.domain_;
    return s;
}
    
    QCString
RMessageID::localPart()
{
    parse();
    return localPart_;
}

    void
RMessageID::setLocalPart(const QCString & s)
{
    parse();
    localPart_ = s.copy();
}

    QCString
RMessageID::domain()
{
    parse();
    return domain_;
}

    void
RMessageID::setDomain(const QCString & s)
{
    parse();
    domain_ = s.copy();
}

    void
RMessageID::_parse()
{
    if (strRep_.isEmpty())
        return;
    
    int atPos = strRep_.find('@');
    
    if (atPos == -1)
        return;
    
    localPart_  = strRep_.left(atPos);
    domain_     = strRep_.right(strRep_.length() - atPos - 1);
    
    if (localPart_.at(0) == '<')
        localPart_.remove(0, 1);
    
    if (domain_.right(1) == ">")
        domain_.remove(domain_.length() - 1, 1);
}

    void
RMessageID::_assemble()
{
    if (localPart_.isEmpty() || domain_.isEmpty())
        strRep_ = "";
    else
        strRep_ = "<" + localPart_ + "@" + domain_ + ">";
}

    void
RMessageID::createDefault()
{
    struct timeval timeVal;
    struct timezone timeZone;
    
    gettimeofday(&timeVal, &timeZone);
    int t = timeVal.tv_sec;

    localPart_ =
        "Empath." +
        QCString().setNum(t)        + '.' +
        QCString().setNum(getpid())    + '.' +
        QCString().setNum(seq_++);
    
    struct utsname utsName;
    if (uname(&utsName) == 0)
        domain_ = utsName.nodename;
    else
        domain_ = "localhost.localdomain";

    parsed_ = true;
    assembled_ = false;
}

// vim:ts=4:sw=4:tw=78
