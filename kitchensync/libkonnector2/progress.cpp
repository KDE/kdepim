#include "progress.h"

using namespace KSync;

Progress::Progress( int code,  const QString& text )
    : Notify( code, text ) {

}
Progress::Progress( const QString& text )
    : Notify( Undefined, text )
{
}
bool Progress::operator==( const Progress& rhs ) {
    return Notify::operator==( rhs );
}
