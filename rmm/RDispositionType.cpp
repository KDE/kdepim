#include <RMM_DispositionType.h>
#include <RMM_Token.h>

using namespace RMM;

RDispositionType::RDispositionType()
    :    RHeaderBody()
{
    // Empty.
}

RDispositionType::RDispositionType(const RDispositionType & t)
    :    RHeaderBody(t)
{
    // Empty.
}

RDispositionType::RDispositionType(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}

    RDispositionType &
RDispositionType::operator = (const RDispositionType & t)
{
    if (this == &t) return *this; // Don't do a = a.
    
    parameterList_    = t.parameterList_;
    dispType_        = t.dispType_;
    filename_        = t.filename_;
    
    RHeaderBody::operator = (t);
    
    return *this;
}

    RDispositionType &
RDispositionType::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RDispositionType::operator == (RDispositionType & dt)
{
    parse();
    dt.parse();
    
    return (
        parameterList_    == dt.parameterList_    &&
        dispType_        == dt.dispType_            &&
        filename_        == dt.filename_);
}

RDispositionType::~RDispositionType()
{
    // Empty.
}

    RMM::DispType
RDispositionType::type()
{
    parse();
    return dispType_;
}

    QCString
RDispositionType::filename()
{
    parse();
    return filename_;
}


    void
RDispositionType::setFilename(const QCString & s)
{
    parse();
    filename_ = s;
}

    void
RDispositionType::_parse()
{
    // STUB
}

    void
RDispositionType::_assemble()
{
    // STUB
}
    
    void
RDispositionType::createDefault()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
