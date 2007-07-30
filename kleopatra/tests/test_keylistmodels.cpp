/* -*- mode: c++; c-basic-offset:4 -*-
    test_keylistmodels.cpp

    This file is part of Kleopatra's test suite.
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/
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
