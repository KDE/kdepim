#ifndef KABC_STDADDRESSBOOK_H
#define KABC_STDADDRESSBOOK_H

#include "addressbook.h"

namespace KABC {

class StdAddressBook : public AddressBook
{
  public:
    static AddressBook *self();
    static bool save();
    
  protected:
    StdAddressBook();
    ~StdAddressBook();
    
  private:
    static AddressBook *mSelf;
};

}

#endif
