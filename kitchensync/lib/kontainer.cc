
#include "kontainer.h"

Kontainer::Kontainer(const QString& first,  const QString& second)
{
    m_first = first;
    m_second = second;
}
Kontainer::Kontainer( const Kontainer &tain )
{
    (*this) = tain;
}
Kontainer::~Kontainer()
{

}
QString Kontainer::first() const
{
    return m_first;
}
QString Kontainer::second() const
{
    return m_second;
}
Kontainer &Kontainer::operator=( const Kontainer &con )
{
    m_first = con.m_first;
    m_second = con.m_second;
    return *this;
}

bool operator== ( const Kontainer &a ,  const Kontainer &b ) {
    if ( a.first() == b.first() &&  a.second() == b.second() )
        return true;

    return false;
}
