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

#include <qgpgme/eventloopinteractor.h>

#include <gpgme++/context.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>

#include <memory>
#include <vector>
#include <string>
#include <cassert>

class Relay : public QObject {
    Q_OBJECT
public:
    explicit Relay( QObject * p=0 ) : QObject( p ) {}

public Q_SLOTS:
    void slotNextKeyEvent( GpgME::Context *, const GpgME::Key & key ) {
	mKeys.push_back( key );
	// push out keys in chunks of 1..16 keys
	if ( mKeys.size() > qrand() % 16 ) {
	    emit nextKeys( mKeys );
	    mKeys.clear();
	}
    }

Q_SIGNALS:
    void nextKeys( const std::vector<GpgME::Key> & keys );

private:
    std::vector<GpgME::Key> mKeys;
};

int main( int argc, char * argv[] ) {

    KAboutData aboutData( "test_flatkeylistmodel", 0, ki18n("FlatKeyListModel Test"), "0.1" );
    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;

    qsrand( QDateTime::currentDateTime().toTime_t() );

    QTreeView flat, hierarchical;

    flat.setWindowTitle( QLatin1String( "Flat Key Listing" ) );
    hierarchical.setWindowTitle( QLatin1String( "Hierarchical Key Listing" ) );

    Relay relay;
    QObject::connect( QGpgME::EventLoopInteractor::instance(), SIGNAL(nextKeyEventSignal(GpgME::Context*,GpgME::Key)),
                      &relay, SLOT(slotNextKeyEvent(GpgME::Context*,GpgME::Key)) );
    
    if ( Kleo::AbstractKeyListModel * const model = Kleo::AbstractKeyListModel::createFlatKeyListModel( &flat ) ) {
        QObject::connect( &relay, SIGNAL(nextKeys(std::vector<GpgME::Key>)), model, SLOT(addKeys(std::vector<GpgME::Key>)) );
        flat.setModel( model );
    }

    if ( Kleo::AbstractKeyListModel * const model = Kleo::AbstractKeyListModel::createHierarchicalKeyListModel( &hierarchical ) ) {
        QObject::connect( &relay, SIGNAL(nextKeys(std::vector<GpgME::Key>)), model, SLOT(addKeys(std::vector<GpgME::Key>)) );
        hierarchical.setModel( model );
    }

    flat.show();
    hierarchical.show();


    const std::auto_ptr<GpgME::Context> pgp( GpgME::Context::createForProtocol( GpgME::OpenPGP ) );
    pgp->setManagedByEventLoopInteractor( true );
    pgp->setKeyListMode( GpgME::Local );

    if ( const GpgME::Error e = pgp->startKeyListing() )
        qDebug() << "pgp->startKeyListing() ->" << e.asString();


    const std::auto_ptr<GpgME::Context> cms( GpgME::Context::createForProtocol( GpgME::CMS ) );
    cms->setManagedByEventLoopInteractor( true );
    cms->setKeyListMode( GpgME::Local );

    if ( const GpgME::Error e = cms->startKeyListing() )
        qDebug() << "cms" << e.asString();


    return app.exec();
}

#include "test_flatkeylistmodel.moc"
