#include "error.h"

using namespace KSync;

Error::Error( int number,  const QString& text )
    : Notify( number,  text )
{
}
Error::Error( const QString& text )
    : Notify( UserDefined,  text )
{
}
bool Error::operator==( const Error& rhs ) {
    return Notify::operator==( rhs );
}
