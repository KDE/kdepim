
#include "addresseeconfig.h"

using namespace KABC;

AddresseeConfig::AddresseeConfig()
{
  mAddressee = Addressee();
}

AddresseeConfig::AddresseeConfig( const Addressee &addr )
{
  mAddressee = addr;
}

void AddresseeConfig::setAddressee( const Addressee &addr )
{
  mAddressee = addr;
}

Addressee AddresseeConfig::addressee()
{
  return mAddressee;
}

void AddresseeConfig::setAutomaticNameParsing( bool value )
{
  KConfig config( "kaddressbook_addrconfig" );
  config.setGroup( mAddressee.uid() );
  config.writeEntry( "AutomaticNameParsing", value );
  config.sync();
}

bool AddresseeConfig::automaticNameParsing()
{
  KConfig config( "kaddressbook_addrconfig" );
  config.setGroup( mAddressee.uid() );
  return config.readBoolEntry( "AutomaticNameParsing", true );
}

void AddresseeConfig::remove()
{
  KConfig config( "kaddressbook_addrconfig" );
  config.deleteGroup( mAddressee.uid() );
}
