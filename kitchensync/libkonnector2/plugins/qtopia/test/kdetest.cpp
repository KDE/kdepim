#include <qstring.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kmdcodec.h>


static const char *description =
	I18N_NOOP("Testapp");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

void md5( const QString& str ) {
    KMD5 test(str );
    qWarning(str);
    qWarning("%s", test.hexDigest().data() );
}

int main(int argc, char *argv[] )
{
  KAboutData aboutData( "md5sumtest", I18N_NOOP("Testapp"),
			"0.01", description, KAboutData::License_GPL,
			"(c) 2001, Holger  Freyther", 0, 0, "freyther@kde.org");
  aboutData.addAuthor("Holger  Freyther",0, "freyther@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KApplication a;

  md5( QString::fromLatin1("Test it") );
  md5( QString::fromLatin1("Test it ") );
  md5( QString::fromLatin1("Holger Freyther") );

}
