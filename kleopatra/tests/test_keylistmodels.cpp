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

#include <config-kleopatra.h>

#include <models/keylistmodel.h>
#include <models/keylistsortfilterproxymodel.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <QTreeView>
#include <QLineEdit>
#include <QLayout>
#include <QTimer>
#include <QEventLoop>
#include <QDateTime>
#include <QDebug>

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
        qDebug( "next key" );
        mKeys.push_back( key );
        // push out keys in chunks of 1..16 keys
        if ( mKeys.size() > qrand() % 16U ) {
            emit nextKeys( mKeys );
            mKeys.clear();
        }
    }

    void slotOperationDoneEvent( GpgME::Context* , const GpgME::Error& error ) {
        qDebug( "listing done error: %d", error.encodedError() );
    }
 
Q_SIGNALS:
    void nextKeys( const std::vector<GpgME::Key> & keys );

private:
    std::vector<GpgME::Key> mKeys;
};

int main( int argc, char * argv[] ) {

    KAboutData aboutData( "test_flatkeylistmodel", 0, ki18n("FlatKeyListModel Test"), "0.2" );
    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add( "flat",         ki18n("Perform flat certificate listing") );
    options.add( "hierarchical", ki18n("Perform hierarchical certificate listing") );
    options.add( "disable-smime", ki18n("Do not list SMIME certificates") );

    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs * args = KCmdLineArgs::parsedArgs();

    const bool showFlat = args->isSet( "flat" ) || !args->isSet( "hierarchical" );
   const bool showHier = args->isSet( "hierarchical" ) || !args->isSet( "flat" );
    const bool disablesmime = args->isSet( "disable-smime" );

    qsrand( QDateTime::currentDateTime().toTime_t() );

    QWidget flatWidget, hierarchicalWidget;
    QVBoxLayout flatLay( &flatWidget ), hierarchicalLay( &hierarchicalWidget );
    QLineEdit flatLE( &flatWidget ), hierarchicalLE( &hierarchicalWidget );
    QTreeView flat( &flatWidget ), hierarchical( &hierarchicalWidget );

    flat.setSortingEnabled( true );
    flat.sortByColumn( Kleo::AbstractKeyListModel::Fingerprint, Qt::AscendingOrder );
    hierarchical.setSortingEnabled( true );
    hierarchical.sortByColumn( Kleo::AbstractKeyListModel::Fingerprint, Qt::AscendingOrder );

    flatLay.addWidget( &flatLE );
    flatLay.addWidget( &flat );

    hierarchicalLay.addWidget( &hierarchicalLE );
    hierarchicalLay.addWidget( &hierarchical );

    flatWidget.setWindowTitle( QLatin1String( "Flat Key Listing" ) );
    hierarchicalWidget.setWindowTitle( QLatin1String( "Hierarchical Key Listing" ) );

    Kleo::KeyListSortFilterProxyModel flatProxy, hierarchicalProxy;

    QObject::connect( &flatLE, SIGNAL(textChanged(QString)), &flatProxy, SLOT(setFilterFixedString(QString)) );
    QObject::connect( &hierarchicalLE, SIGNAL(textChanged(QString)), &hierarchicalProxy, SLOT(setFilterFixedString(QString)) );

    Relay relay;
    QObject::connect( QGpgME::EventLoopInteractor::instance(), SIGNAL(nextKeyEventSignal(GpgME::Context*,GpgME::Key)),
                      &relay, SLOT(slotNextKeyEvent(GpgME::Context*,GpgME::Key)) );
    QObject::connect( QGpgME::EventLoopInteractor::instance(), SIGNAL(operationDoneEventSignal(GpgME::Context*,GpgME::Error)),
                      &relay, SLOT(slotOperationDoneEvent(GpgME::Context*,GpgME::Error)) );


    if ( showFlat )
        if ( Kleo::AbstractKeyListModel * const model = Kleo::AbstractKeyListModel::createFlatKeyListModel( &flat ) ) {
            QObject::connect( &relay, SIGNAL(nextKeys(std::vector<GpgME::Key>)), model, SLOT(addKeys(std::vector<GpgME::Key>)) );
            flatProxy.setSourceModel( model );
            flat.setModel( &flatProxy );

            flatWidget.show();
        }

    if ( showHier )
        if ( Kleo::AbstractKeyListModel * const model = Kleo::AbstractKeyListModel::createHierarchicalKeyListModel( &hierarchical ) ) {
            QObject::connect( &relay, SIGNAL(nextKeys(std::vector<GpgME::Key>)), model, SLOT(addKeys(std::vector<GpgME::Key>)) );
            hierarchicalProxy.setSourceModel( model );
            hierarchical.setModel( &hierarchicalProxy );

            hierarchicalWidget.show();
        }

    const std::auto_ptr<GpgME::Context> pgp( GpgME::Context::createForProtocol( GpgME::OpenPGP ) );
    pgp->setManagedByEventLoopInteractor( true );
    pgp->setKeyListMode( GpgME::Local );

    if ( const GpgME::Error e = pgp->startKeyListing() )
        qDebug() << "pgp->startKeyListing() ->" << e.asString();


    if ( !disablesmime ) {
        const std::auto_ptr<GpgME::Context> cms( GpgME::Context::createForProtocol( GpgME::CMS ) );
        cms->setManagedByEventLoopInteractor( true );
        cms->setKeyListMode( GpgME::Local );

        if ( const GpgME::Error e = cms->startKeyListing() )
            qDebug() << "cms" << e.asString();


       QEventLoop loop;
       QTimer::singleShot( 2000, &loop, SLOT(quit()) );
       loop.exec();

       const std::auto_ptr<GpgME::Context> cms2( GpgME::Context::createForProtocol( GpgME::CMS ) );
       cms2->setManagedByEventLoopInteractor( true );
       cms2->setKeyListMode( GpgME::Local );

       if ( const GpgME::Error e = cms2->startKeyListing() )
          qDebug() << "cms2" << e.asString();
    }


    return app.exec();
}

#include "test_flatkeylistmodel.moc"
