#ifndef KABC_VCARDFORMATIMPL_H
#define KABC_VCARDFORMATIMPL_H

#include <qstring.h>

#include "format.h"

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
};

}

#endif
