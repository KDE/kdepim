#include "vcardformatimpl.h"

#include "vcardformat.h"

using namespace KABC;

VCardFormat::VCardFormat()
{
  mImpl = new VCardFormatImpl;
}

VCardFormat::~VCardFormat()
{
  delete mImpl;
}

bool VCardFormat::load( AddressBook *addressBook, const QString &fileName )
{
  return mImpl->load( addressBook, fileName );
}

bool VCardFormat::save( AddressBook *addressBook, const QString &fileName )
{
  return mImpl->save( addressBook, fileName );
}
