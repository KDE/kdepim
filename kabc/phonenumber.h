#ifndef KABC_PHONENUMBER_H
#define KABC_PHONENUMBER_H

#include <qvaluelist.h>
#include <qstring.h>

namespace KABC {

class PhoneNumber
{
  public:
    typedef QValueList<PhoneNumber> List;
  
    enum Type { Home, Office, Mobile, Fax };
  
    PhoneNumber();
    PhoneNumber( const QString &, Type type = Home );
    ~PhoneNumber();
    
    void setNumber( const QString & );
    QString number() const;
    
    void setType( Type );
    Type type() const;
    
  private:
    Type mType;
    QString mNumber;
};

}

#endif
