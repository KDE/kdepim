#include <kaboutdata.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "addressbook.h"

using namespace KABC;

int main(int argc,char **argv)
{
  KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;
  
  AddressBook ab;
  
  ab.load( "/home/cs/kdecvs/kdepim/kabc/my.kabc" );
  ab.dump();
  
  ab.clear();
  
  Addressee a;
  a.setName( "Hans Speck" );
  a.setEmail( "hw@abc.de" );
  ab.insertAddressee( a );

  ab.dump();

  Addressee b;
  b = a;
  b.setName( "Hilde Wurst" );
  b.insertPhoneNumber( PhoneNumber( PhoneNumber::Mobile, "12345" ) );
  ab.insertAddressee( b );

  ab.dump();
  
  Addressee c( a );
  c.setName( "Klara Klossbruehe" );
  c.insertPhoneNumber( PhoneNumber( PhoneNumber::Mobile, "00000" ) );
  c.insertPhoneNumber( PhoneNumber( PhoneNumber::Fax, "4711" ) );
  ab.insertAddressee( c );
  
  ab.dump();
  
  AddressBook::Iterator it = ab.find( a );
  (*it).setEmail( "neueemail@woauchimmer" );
  
  ab.dump();
  
  AddressBook::Ticket *t = ab.requestSave( "/home/cs/kdecvs/kdepim/kabc/my.kabc" );
  if ( t ) {
    ab.save( t );
  } else {
    kdDebug() << "No ticket for save." << endl;
  }
}
