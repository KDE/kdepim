#ifndef KABC_PHONENUMBER_H
#define KABC_PHONENUMBER_H

#include <qvaluelist.h>
#include <qstring.h>

namespace KABC {

class PhoneNumber
{
  public:
    typedef QValueList<PhoneNumber> List;
  
    enum { Home = 1, Work = 2, Msg = 4, Pref = 8, Voice = 16, Fax = 32,
           Cell = 64, Video = 128, Bbs = 256, Modem = 512, Car = 1024,
           Isdn = 2048, Pcs = 4096, Pager = 8192 };
  
    PhoneNumber();
    PhoneNumber( const QString &, int type = Home );
    ~PhoneNumber();
    
    void setId( const QString &id );
    QString id() const;
    
    void setNumber( const QString & );
    QString number() const;
    
    void setType( int );
    int type() const;
    
  private:
    void init();
  
    QString mId;
  
    int mType;
    QString mNumber;
};

}

#endif
