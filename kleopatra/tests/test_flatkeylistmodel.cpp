#include <models/keylistmodel.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <QTableView>

#include <gpgme++/context.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>

#include <memory>
#include <vector>

int main( int argc, char * argv[] ) {

    KAboutData aboutData( "test_flatkeylistmodel", 0, ki18n("FlatKeyListModel Test"), "0.1" );
    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;

    QTableView view;
    
    Kleo::AbstractKeyListModel * const model = Kleo::AbstractKeyListModel::createFlatKeyListModel( &view );

    view.setModel( model );
    view.show();

    std::vector<GpgME::Key> keys;

    {    
	std::auto_ptr<GpgME::Context> pgp( GpgME::Context::createForProtocol( GpgME::OpenPGP ) );
	pgp->setKeyListMode( GpgME::Local );

	if ( GpgME::Error e = pgp->startKeyListing() ) {
	    qDebug() << "pgp" << e.asString();
	    return 1;
	}

	int pgpKeys = 0;
	for (;;) {
	    GpgME::Error e;
	    const GpgME::Key key = pgp->nextKey( e );
	    if ( key.isNull() ) {
		qDebug() << "pgp null key" << e.asString();
		break;
	    }
	    keys.push_back( key );
	    ++pgpKeys;
	}
	qDebug() << "pgpKeys" << pgpKeys;
    }

    {
	std::auto_ptr<GpgME::Context> cms( GpgME::Context::createForProtocol( GpgME::CMS ) );
	cms->setKeyListMode( GpgME::Local );

	if ( GpgME::Error e = cms->startKeyListing() ) {
	    qDebug() << "cms" << e.asString();
	    return 1;
	}

	int cmsKeys = 0;
	for (;;) {
	    GpgME::Error e;
	    const GpgME::Key key = cms->nextKey( e );
	    if ( key.isNull() ) {
		qDebug() << "cms null key" << e.asString();
		break;
	    }
	    keys.push_back( key );
	    ++cmsKeys;
	}
	qDebug() << "cmsKeys" << cmsKeys;
    }
	
    model->addKeys( keys );

    return app.exec();
}
