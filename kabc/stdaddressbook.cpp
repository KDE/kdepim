#include <kstandarddirs.h>
#include <kdebug.h>

#include "stdaddressbook.h"

using namespace KABC;

AddressBook *StdAddressBook::mSelf = 0;

AddressBook *StdAddressBook::self()
{
  if ( !mSelf ) {
    mSelf = new StdAddressBook;
  }
  return mSelf;
}

bool StdAddressBook::save()
{
  Ticket *ticket = self()->requestSave( locateLocal( "data", "kabc/std.vcf" ) );
  if ( !ticket ) {
    kdError() << "Can't save to standard addressbook. It's locked." << endl;
    return false;
  }
  return self()->save( ticket );
}


StdAddressBook::StdAddressBook()
{
  self()->load( locateLocal( "data", "kabc/std.vcf" ) );
}

StdAddressBook::~StdAddressBook()
{
  save();
}
