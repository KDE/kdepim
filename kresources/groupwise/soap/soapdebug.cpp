#include "groupwiseserver.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <ktemporaryfile.h>

#include <kabc/resourcecached.h>

#include <kcal/icalformat.h>
#include <kcal/resourcelocal.h>
#include <kcal/calendarresources.h>

#include <iostream>

namespace KABC {

class ResourceMemory : public ResourceCached
{
  public:
    ResourceMemory() : ResourceCached() {}
    
    Ticket *requestSaveTicket() { return 0; }
    bool load() { return true; }
    bool save( Ticket * ) { return true; }
    void releaseSaveTicket( Ticket * ) {}
};

}

int main( int argc, char **argv )
{
  KAboutData aboutData( "soapdebug", 0, ki18n("Groupwise Soap Debug"), "0.1" );
  aboutData.addAuthor( ki18n("Cornelius Schumacher"), KLocalizedString(), "schumacher@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add("s");
  options.add("server <hostname>", ki18n("Server"));
  options.add("u");
  options.add("user <string>", ki18n("User"));
  options.add("p");
  options.add("password <string>", ki18n("Password"));
  options.add("f");
  options.add("freebusy-user <string>", ki18n("Free/Busy user name"));
  options.add("addressbook-id <string>", ki18n("Address book identifier"));
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString user = args->getOption( "user" );
  QString pass = args->getOption( "password" );
  QString url = args->getOption( "server" );

#if 1
  if ( user.isEmpty() ) {
    kError() <<"Need user.";
    return 1; 
  }
  if ( pass.isEmpty() ) {
    kError() <<"Need password.";
    return 1; 
  }
  if ( url.isEmpty() ) {
    kError() <<"Need server.";
    return 1; 
  }
#endif
  KDateTime::Spec spec = KDateTime::Spec::LocalZone();
  GroupwiseServer server( url, user, pass, spec, 0 );

#if 1
  if ( !server.login() ) {
    kError() <<"Unable to login to server" << url;
    return 1;
  }
#endif

#if 0
  server.dumpData();
#endif

#if 0
  server.getCategoryList();
#endif

#if 0
  server.dumpFolderList();
#endif

#if 0
  QString fbUser = args->getOption( "freebusy-user" );
  if ( fbUser.isEmpty() ) {
    kError() <<"Need user for which the freebusy data should be retrieved.";
  } else {
    KCal::FreeBusy *fb = new KCal::FreeBusy;

    server.readFreeBusy( "user1",
      QDate( 2004, 9, 1 ), QDate( 2004, 10, 31 ), fb );
  }
#endif

#if 0
  KTemporaryFile temp;
  temp.setautoRemove(false);
  temp.open();
  KCal::ResourceLocal resource( temp.fileName() );
  resource.setActive( true );
  KCal::CalendarResources calendar;
  calendar.resourceManager()->add( &resource );
  kDebug() <<"Login";

  if ( !server.login() ) {
    kDebug() <<"Unable to login.";
  } else {
    kDebug() <<"Read calendar";
    if ( !server.readCalendarSynchronous( &resource ) ) {
      kDebug() <<"Unable to read calendar data.";
    }
    kDebug() <<"Logout";
    server.logout();
  }
  KCal::ICalFormat format;

  QString ical = format.toString( &calendar );

  kDebug() <<"ICALENDAR:" << ical;
#endif

#if 0
  QString id = args->getOption( "addressbook-id" );

  kDebug() <<"ADDRESSBOOK ID:" << id;

  QStringList ids;
  ids.append( id );

  KABC::ResourceMemory resource;

  kDebug() <<"Login";
  if ( !server.login() ) {
    kError() <<"Unable to login.";
  } else {
    kDebug() <<"Read Addressbook";
    if ( !server.readAddressBooksSynchronous( ids, &resource ) ) {
      kError() <<"Unable to read addressbook data.";
    }
    kDebug() <<"Logout";
    server.logout();
  }

  KABC::Addressee::List addressees;
  KABC::Resource::Iterator it2;
  for( it2 = resource.begin(); it2 != resource.end(); ++it2 ) {
    kDebug() <<"ADDRESSEE:" << (*it2).fullEmail();
    addressees.append( *it2 );
  }
#endif

#if 0    
  server.getDelta();
#endif

  server.logout();

  return 0;
}

