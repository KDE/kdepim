#ifndef KABC_VCARDFORMATIMPL_H
#define KABC_VCARDFORMATIMPL_H

#include <qstring.h>

#include "format.h"
#include "address.h"
#include "addressee.h"

#include <VCard.h>

namespace KABC {

class AddressBook;

class VCardFormatImpl {
  public:
    bool load( AddressBook *, const QString &fileName );
    bool save( AddressBook *, const QString &fileName );

  protected:
    void addTextValue (VCARD::VCard *, VCARD::EntityType, const QString & );
    QString readTextValue( VCARD::ContentLine * );
    
    void addAddressValue( VCARD::VCard *, const Address & );
    Address readAddressValue( VCARD::ContentLine * );

    void addTelephoneValue( VCARD::VCard *, const PhoneNumber & );
    PhoneNumber readTelephoneValue( VCARD::ContentLine * );

    void addNValue( VCARD::VCard *, const Addressee & );
    void readNValue( VCARD::ContentLine *, Addressee & );
};

}

#endif
