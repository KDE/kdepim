#include <models/keylistmodel.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <QTreeView>

#include <gpgme++/context.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>

#include <memory>
#include <vector>
#include <string>
#include <cassert>

int main( int argc, char * argv[] ) {

    KAboutData aboutData( "test_flatkeylistmodel", 0, ki18n("FlatKeyListModel Test"), "0.1" );
    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;

    QTreeView flat, hierarchical;

    flat.setWindowTitle( QLatin1String( "Flat Key Listing" ) );
    hierarchical.setWindowTitle( QLatin1String( "Hierarchical Key Listing" ) );
    
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
	
    if ( Kleo::AbstractKeyListModel * const model = Kleo::AbstractKeyListModel::createFlatKeyListModel( &flat ) ) {
	model->addKeys( keys );
	flat.setModel( model );
    }

    if ( Kleo::AbstractKeyListModel * const model = Kleo::AbstractKeyListModel::createHierarchicalKeyListModel( &hierarchical ) ) {
	model->addKeys( keys );
	hierarchical.setModel( model );
    }

    flat.show();
    hierarchical.show();

    return app.exec();
}
