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
  ab.setAddressee( a );

  ab.dump();

  Addressee b;
  b = a;
  b.setName( "Hilde Wurst" );
  PhoneNumber p;
  p.setType( PhoneNumber::Mobile );
  p.setNumber( "12345" );
  b.setPhoneNumber( p );
  ab.setAddressee( b );

  ab.dump();
  
  Addressee c( a );
  c.setName( "Klara Klossbruehe" );
  PhoneNumber p1;
  p1.setType( PhoneNumber::Mobile );
  p1.setNumber( "000000" );
  c.setPhoneNumber( p1 );
  PhoneNumber p2;
  p2.setType( PhoneNumber::Fax );
  p2.setNumber( "4711" );
  c.setPhoneNumber( p2 );
  ab.setAddressee( c );
  
  ab.dump();
  
  Addressee d = ab.addressee( a );
  d.setEmail( "neueemail@woauchimmer" );
  ab.setAddressee( d );
  
  ab.dump();
  
  AddressBook::Ticket *t = ab.requestSave( "/home/cs/kdecvs/kdepim/kabc/my.kabc" );
  if ( t ) {
    ab.save( t );
  } else {
    kdDebug() << "No ticket for save." << endl;
  }
}
