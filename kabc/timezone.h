#ifndef KABC_TIMEZONE_H
#define KABC_TIMEZONE_H

#include <qstring.h>

namespace KABC {

class TimeZone
{
  public:
    TimeZone();
    TimeZone( int offset );
    
    void setOffset( int );
    int offset() const;

    bool isValid() const;
    
    bool operator==( const TimeZone & ) const;
    
    QString asString() const;
    
  private:
    int mOffset;  // Offset in minutes

    bool mValid;
};

}

#endif
