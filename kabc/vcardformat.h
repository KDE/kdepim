#ifndef KABC_VCARDFORMAT_H
#define KABC_VCARDFORMAT_H

#include <qstring.h>

#include "format.h"

namespace KABC {

class AddressBook;

class VCardFormat : public Format {
  public:
    bool load( AddressBook *, const QString &fileName );
    bool save( AddressBook *, const QString &fileName );
};

}

#endif
