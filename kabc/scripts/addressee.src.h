#ifndef KABC_ADDRESSEE_H
#define KABC_ADDRESSEE_H
// $Id$

#include <qvaluelist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>

#include <ksharedptr.h>
#include <kurl.h>

#include "phonenumber.h"
#include "address.h"
#include "geo.h"
#include "timezone.h"

namespace KABC {

struct AddresseeData : public KShared
{
  --VARIABLES--

  PhoneNumber::List phoneNumbers;
  Address::List addresses;
  QStringList emails;
  QStringList categories;
};


class Addressee
{
  public:
    typedef QValueList<Addressee> List;

    Addressee();
    ~Addressee();

    Addressee( const Addressee & );
    Addressee &operator=( const Addressee & );

    --DECLARATIONS--
    void insertEmail( const QString &email, bool prefered=false );
    void removeEmail( const QString &email );
    QString preferredEmail() const;
    QStringList emails() const;
    
    void insertPhoneNumber( const PhoneNumber &phoneNumber );
    void removePhoneNumber( const PhoneNumber &phoneNumber );
    PhoneNumber phoneNumber( int type ) const;
    PhoneNumber::List phoneNumbers() const;
    PhoneNumber findPhoneNumber( const QString &id ) const;
    
    void insertAddress( const Address &address );
    void removeAddress( const Address &address );
    Address address( int type ) const;
    Address::List addresses() const;
    Address findAddress( const QString &id ) const;

    void insertCategory( const QString & );
    void removeCategory( const QString & );
    bool hasCategory( const QString & ) const;
    void setCategories( const QStringList & );
    QStringList categories() const;

    void dump() const;
  
  private:
    Addressee copy();
    void detach();
  
    KSharedPtr<AddresseeData> mData;
};

}

#endif
