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
    void insertPhoneNumber( const PhoneNumber &phoneNumber );
    PhoneNumber phoneNumber( PhoneNumber::Type ) const;
    PhoneNumber::List phoneNumbers() const;
    
    void insertAddress( const Address &address );
    Address address( int type ) const;
    Address::List addresses() const;

    void dump() const;
  
  private:
    Addressee copy();
    void detach();
  
    KSharedPtr<AddresseeData> mData;
};

}

#endif
