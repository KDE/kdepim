#ifndef KABC_SIMPLEFORMAT_H
#define KABC_SIMPLEFORMAT_H

#include <qstring.h>

namespace KABC {

class AddressBook;

class Format {
  public:
    virtual bool load( AddressBook *, const QString &fileName ) = 0;
    virtual bool save( AddressBook *, const QString &fileName ) = 0;
};

class SimpleFormat : public Format {
  public:
    bool load( AddressBook *, const QString &fileName );
    bool save( AddressBook *, const QString &fileName );
};

}

#endif
