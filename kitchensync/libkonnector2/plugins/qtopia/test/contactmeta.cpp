#include <qfile.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qstring.h>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kdebug.h>

#include <addressbooksyncee.h>

#include "../categoryedit.h"
#include "../addressbook.h"
#include "../metaaddressbook.h"


static const char *description =
	I18N_NOOP("Testapp");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static KCmdLineOptions options[] =
{
    { "cat <path>" , I18N_NOOP("Path to the Category file"), 0},
    { "con <path>" , I18N_NOOP("Path to the addressbook.xml"),  0},
    { "out <path>" , I18N_NOOP("Path to where this app should write") ,  0 },
    { "c", I18N_NOOP("Only dump data"), 0 },
    { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

void dump( KSync::AddressBookSyncEntry* entry) {
    KABC::Addressee adr = entry->addressee();

    QString str;
    kdDebug() << "GivenName " << adr.givenName()  << endl;
    kdDebug() << "AdditionalName " <<  adr.additionalName() << endl;
    kdDebug() << "FamilyName " <<  adr.familyName() << endl;
    kdDebug() << "Suffix " << adr.suffix() << endl;
    kdDebug() << "Role " <<  adr.role() << endl;
    kdDebug() << "Department " << adr.custom( "opie", "Department" ) << endl;


    /*
     * busines numbers
     */
    KABC::PhoneNumber number = adr.phoneNumber( KABC::PhoneNumber::Work );
    kdDebug() <<  "Phone WOrk " << number.number() << endl;

    number = adr.phoneNumber( KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
    kdDebug() <<  "Work Fax " << number.number() << endl;

    number = adr.phoneNumber( KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
    kdDebug() <<  "Work Cell " << number.number() << endl;

    kdDebug() <<  "Emails " << adr.emails().join(";") << endl;

    /*
     * home/private numbers
     */
    number = adr.phoneNumber( KABC::PhoneNumber::Home );
    kdDebug() <<  "Tel Home " << number.number() << endl;

    number = adr.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
    kdDebug() <<  "Fax home " << number.number() << endl;

    number = adr.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell );
    kdDebug() <<  "Cel home " << number.number() << endl;

    /*
     * address bits
     */
    KABC::Address ad = adr.address( KABC::Address::Work );
    kdDebug() <<  "Work street " << ad.street() << endl;
    kdDebug() <<  "local "<< ad.locality() << endl;
    kdDebug() <<  "postal " << ad.postalCode() << endl;
    kdDebug() <<  "country " << ad.country() << endl;
    kdDebug() <<  "region " << ad.region() << endl;


    /*
     * home address
     */
    ad = adr.address( KABC::Address::Home );
    kdDebug() <<  "Home street " << ad.street() << endl;
    kdDebug() <<  "local " << ad.locality() << endl;
    kdDebug() <<  "region " <<ad.region() << endl;
    kdDebug() <<  "postal " <<ad.postalCode() << endl;
    kdDebug() <<  "country " << ad.country() << endl;

    kdDebug() <<  "office " << adr.custom( "opie", "Office" ) << endl;
    kdDebug() <<  "profession " <<adr.custom( "opie", "Profession" ) << endl;
    kdDebug() <<  "assistant " << adr.custom( "opie", "Assistant" ) << endl;
    kdDebug() <<  "manager " << adr.custom( "opie", "Manager" ) << endl;
    kdDebug() <<  "children " << adr.custom( "opie", "Children" ) << endl;
    kdDebug() <<  "homeweb " << adr.custom( "opie", "HomeWebPage" ) << endl;
    kdDebug() <<  "spouse " << adr.custom( "opie", "Spouse" ) << endl;
    kdDebug() <<  "gender " <<adr.custom( "opie", "Gender" ) << endl;
    kdDebug() <<  "birthday " << adr.birthday().date().toString("dd.M.yyyy") << endl;
    kdDebug() <<  "anniversary " <<adr.custom( "opie", "Anniversary" ) << endl;

    kdDebug() <<  "note "<<adr.note() << endl;
    kdDebug() <<  "nick "<<adr.nickName() << endl;
    kdDebug() <<  "cat "<< adr.categories().join(";") << endl;
}

void dump( const QString& path, const QString& cat ) {
    OpieHelper::CategoryEdit edit( cat );
    OpieHelper::AddressBook ab( &edit );

    KSync::AddressBookSyncee* syncee = ab.toKDE( path );

    KSync::AddressBookSyncEntry* entry;
    for ( entry = (KSync::AddressBookSyncEntry*) syncee->firstEntry();
          entry != 0;
          entry = (KSync::AddressBookSyncEntry*) syncee->nextEntry() ) {
        dump( entry );
    }
}
void output( QPtrList<KSync::SyncEntry> li) {
    KSync::SyncEntry* entry;
    for ( entry = li.first(); entry != 0; entry = li.next() ) {
        kdDebug() << "Id " << entry->id() << endl;
        kdDebug() << "Name " << entry->name() << endl;
        kdDebug() << "State " << entry->state() << endl;
        kdDebug() << "----" << endl;
    }
}
void meta( const QString& path, const QString& cat ) {
    OpieHelper::CategoryEdit edit( cat );
    OpieHelper::AddressBook ab( &edit );

    KSync::AddressBookSyncee* syncee = ab.toKDE( path );

    KSync::AddressBookSyncEntry* entry;

    OpieHelper::MD5Map map( "/home/ich/contacts.md5.qtopia-socket" );

    OpieHelper::MetaAddressbook abook;
    abook.doMeta( syncee, map );

    QPtrList<KSync::SyncEntry> changed = syncee->added();
    kdDebug() << "added " << endl;
    output( changed );
    kdDebug() << "modified " << endl;
    output( syncee->modified() );

    kdDebug() << "removed " << endl;
    output( syncee->removed() );

    abook.saveMeta( syncee, map );

    map.save();
}



int main(int argc, char *argv[] )
{
    KAboutData aboutData( "dcopclient", I18N_NOOP("Testapp"),
                          "0.01", description, KAboutData::License_GPL,
                          "(c) 2001, Holger  Freyther", 0, 0, "freyther@kde.org");
    aboutData.addAuthor("Holger  Freyther",0, "freyther@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    KApplication::addCmdLineOptions();
    KApplication a;


    if ( !args->isSet("cat") || !args->isSet("con") ) {
        kdDebug() << "enter paths" << endl;
        return 0;
    }

    QString cat = QString::fromLocal8Bit( args->getOption("cat") );
    QString con = QString::fromLocal8Bit( args->getOption("con") );


    if ( args->isSet("c" ) ) {
        dump( con,  cat );
    }else{
        meta(con, cat);

    }
}
