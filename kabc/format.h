#ifndef KABC_FORMAT_H
#define KABC_FORMAT_H

#include <qstring.h>

namespace KABC {

class AddressBook;

class Format {
  public:
    virtual bool load( AddressBook *, const QString &fileName ) = 0;
    virtual bool save( AddressBook *, const QString &fileName ) = 0;
};

}

#endif
