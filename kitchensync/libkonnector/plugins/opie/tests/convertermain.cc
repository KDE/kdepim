
#include <stdlib.h>
#include <stdio.h>

#include <qvaluelist.h>

#include <klocale.h>

#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kapplication.h>

#include <kontainer.h>
#include <idhelper.h>


//#include  "../categoryedit.h"
//#include "../todo.h"

#include <plugins/opie/categoryedit.h>

static KCmdLineOptions options[] =
{
    { "id <path>",  I18N_NOOP("Path to the dir with ids" ),  0 },
    { "category <path>",  I18N_NOOP("Path to the Category file"),  0 },
    { "todo <path>",  I18N_NOOP("Path to the todo xml"),  0 },
    { "datebook <path>",  I18N_NOOP("Path to the datebook xml"),  0 },
    { "addressbook <path>",  I18N_NOOP("Path to the addressbook"),  0 },
    { 0, 0, 0 }
};

int main( int argc,  char *argv[] )
{
    KCmdLineArgs::init( argc,  argv,  "Walla", "test", "0.1" );
    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    KApplication::addCmdLineOptions();
    KApplication app;
    QString category;
    if ( args->isSet("category") )
         category = QString::fromLocal8Bit( args->getOption( "category" ) );
    else
         exit(0 );
    QString id;
    if (args->isSet("id") ) {
        id = QString::fromLocal8Bit( args->getOption("id") );
        qWarning("id %s",  args->getOption("id").data() );
    }else
        exit( 0 );

    QString todo;
    if ( args->isSet("todo") )
        todo = QString::fromLocal8Bit( args->getOption("todo") );
    QString datebook;
    if ( args->isSet("datebook") )
       datebook = QString::fromLocal8Bit( args->getOption("datebook") );
    QString address;
    if ( args->isSet("addressbook") )
         address = QString::fromLocal8Bit( args->getOption("addressbook") );


    kdDebug() << "path " << id << " " << category << endl;
    KonnectorUIDHelper helper(id );
    qWarning("Helper done" );
//    helper.addId( "test-123",  "-12346",  "Test-12346"); // enable dissable to test helper loading and al
    kdDebug() << "KdeId for test-123 and -12345 is " << helper.kdeId("test-123",  "-12345") << endl;
    kdDebug() << "KdeId for test-123 and -12346 is " << helper.kdeId("test-123",  "-12346") << endl;
    kdDebug() << "Konnector Id test-123 and Test-12345 " << helper.konnectorId( "test-123",  "Test-12345") << endl;
    kdDebug() << "Illegal group " << helper.konnectorId( "test-1234",  "ping-pong") << endl;
    QValueList<Kontainer> kont;
    //kont.append( Kontainer( "-12345",  "12345-Test") );
    //kont.append( Kontainer( "-12346",  "12346-Test") );
    //helper.replaceIds( "test-123",  kont );

//    OpieHelper::CategoryEdit *edit = new OpieHelper::CategoryEdit( );
//    edit->parse( category);
//    OpieHelper::ToDo todo2( 0,  &helper,  true );
//    OpieHelperClass helpClass;
//    Test test;
    OpieCategories cats;
    OpieHelper::CategoryEdit edit;
    helper.save();
    args->clear();
    return app.exec();
}
