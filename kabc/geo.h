#ifndef KABC_GEO_H
#define KABC_GEO_H
//$Id$

#include <qstring.h>

namespace KABC {

class Geo
{
  public:
    Geo();
    Geo( float latitude, float longitude );
    
    void setLatitude( float );
    float latitude() const;
    
    void setLongitude( float );
    float longitude() const;
  
    bool isValid() const;
    
    bool operator==( const Geo & ) const;

    QString asString() const;
      
  private:
    float mLatitude;
    float mLongitude;
    
    bool mValid;
    bool mValidLat;
    bool mValidLong;
};

}

#endif
