#ifndef KABC_ADDRESSEE_H
#define KABC_ADDRESSEE_H
// $Id$

#include <qvaluelist.h>
#include <qstring.h>

#include <ksharedptr.h>

#include "phonenumber.h"

namespace KABC {

struct AddresseeData : public KShared
{
  QString uid;
  QString name;
  QString formattedName;
  QString email;
  PhoneNumber::List phoneNumbers;
};


class Addressee
{
  public:
    typedef QValueList<Addressee> List;

    Addressee();
    ~Addressee();

    Addressee( const Addressee & );
    Addressee &operator=( const Addressee & );

    void setUid( const QString & );
    QString uid() const;

    void setName( const QString &name );
    QString name() const;
    
    void setFormattedName( const QString formattedName );
    QString formattedName() const;
    
    void setEmail( const QString &email );
    QString email() const;
    
    void setPhoneNumber( const PhoneNumber &phoneNumber );
    PhoneNumber phoneNumber( PhoneNumber::Type ) const;
    PhoneNumber::List phoneNumbers() const;

    void dump() const;
  
  private:
    Addressee copy();
    void detach();
  
    KSharedPtr<AddresseeData> mData;
};

}

#endif
