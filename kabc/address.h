#ifndef KABC_ADDRESS_H
#define KABC_ADDRESS_H

#include <qstring.h>
#include <qvaluelist.h>

namespace KABC {

class Address
{
  public:
    typedef QValueList<Address> List;
  
    Address();
  
    enum { Dom = 1, Intl = 2, Postal = 4, Parcel = 8, Home = 16, Work = 32,
           Pref = 64 };

    void setType( int );
    int type() const;

    void setPostOfficeBox( const QString & );
    QString postOfficeBox() const;

    void setExtended( const QString & );
    QString extended() const;

    void setStreet( const QString & );
    QString street() const;

    void setLocality( const QString & );
    QString locality() const;

    void setRegion( const QString & );
    QString region() const;
 
    void setPostalCode( const QString & );
    QString postalCode() const;

    void setCountry( const QString & );
    QString country() const;

    void setLabel( const QString & );
    QString label() const;
  
  private:
    int mType;
  
    QString mPostOfficeBox;
    QString mExtended;
    QString mStreet;
    QString mLocality;
    QString mRegion;
    QString mPostalCode;
    QString mCountry;
    QString mLabel;
};

}

#endif
