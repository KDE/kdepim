/*
    This file is part of the IMAP resources.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdcopservicestarter.h>
#include <klocale.h>

#include "kmailicalIface_stub.h"

#include "resourceimapshared.h"
#include "kmailconnection.h"

using namespace ResourceIMAPBase;


KMailConnection::KMailConnection( ResourceIMAPShared* resource,
                                  const QCString& objId )
  : DCOPObject( objId ), mResource( resource ), mKMailIcalIfaceStub( 0 )
{
  // Make the connection to KMail ready
  mDCOPClient = new DCOPClient();
  mDCOPClient->attach();
  mDCOPClient->registerAs( objId, true );

  kapp->dcopClient()->setNotifications( true );
  connect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QCString& ) ),
           this, SLOT( unregisteredFromDCOP( const QCString& ) ) );
}

KMailConnection::~KMailConnection()
{
  kapp->dcopClient()->setNotifications( false );
  delete mKMailIcalIfaceStub;
  mKMailIcalIfaceStub = 0;
  delete mDCOPClient;
  mDCOPClient = 0;
}

static const QCString dcopObjectId = "KMailICalIface";
bool KMailConnection::connectToKMail()
{
  if ( !mKMailIcalIfaceStub ) {
    QString error;
    QCString dcopService;
    int result = KDCOPServiceStarter::self()->
      findServiceFor( "DCOP/ResourceBackend/IMAP", QString::null,
                      QString::null, &error, &dcopService );
    if ( result != 0 ) {
      kdDebug() << "Couldn't connect to the IMAP resource backend\n";
      // TODO: You might want to show "error" (if not empty) here,
      // using e.g. KMessageBox
      return false;
    }

    mKMailIcalIfaceStub = new KMailICalIface_stub( kapp->dcopClient(),
                                                   dcopService, dcopObjectId );

    // Attach to the KMail signals
    if ( !connectKMailSignal( "incidenceAdded(QString,QString)",
                              "addIncidence(QString,QString)" ) )
      kdError() << "DCOP connection to incidenceAdded failed" << endl;
    if ( !connectKMailSignal( "incidenceDeleted(QString,QString)",
                              "deleteIncidence(QString,QString)" ) )
      kdError() << "DCOP connection to incidenceDeleted failed" << endl;
    if ( !connectKMailSignal( "signalRefresh(QString)",
                              "slotRefresh(QString)" ) )
      kdError() << "DCOP connection to signalRefresh failed" << endl;
  }

  return ( mKMailIcalIfaceStub != 0 );
}

bool KMailConnection::addIncidence( const QString& type, const QString& ical )
{
  return mResource->addIncidence( type, ical );
}

void KMailConnection::deleteIncidence( const QString& type,
                                       const QString& uid )
{
  mResource->deleteIncidence( type, uid );
}

void KMailConnection::slotRefresh( const QString& type )
{
  mResource->slotRefresh( type );
}

bool KMailConnection::connectKMailSignal( const QCString& signal,
                                          const QCString& method )
{
  return connectDCOPSignal( "kmail", dcopObjectId, signal, method, false );
}

bool KMailConnection::kmailIncidences( QStringList& lst, const QString& type )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error: Can't connect to KMail\n";
    return false;
  }

  lst = mKMailIcalIfaceStub->incidences( type );
  return mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailAddIncidence( const QString& type,
                                         const QString& uid,
                                         const QString& incidence )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error: Can't connect to KMail\n";
    return false;
  }

  return mKMailIcalIfaceStub->addIncidence( type, uid, incidence )
    && mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailDeleteIncidence( const QString& type,
                                            const QString& uid )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error: Can't connect to KMail\n";
    return false;
  }

  return mKMailIcalIfaceStub->deleteIncidence( type, uid )
    && mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailUpdate( const QString& type,
                                   const QStringList& lst )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error: Can't connect to KMail\n";
    return false;
  }

  return mKMailIcalIfaceStub->update( type, lst )
    && mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailUpdate( const QString& type, const QString& uid,
                                   const QString& incidence )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error: Can't connect to KMail\n";
    return false;
  }

  return mKMailIcalIfaceStub->update( type, uid, incidence )
    && mKMailIcalIfaceStub->ok();
}


void KMailConnection::unregisteredFromDCOP( const QCString& appId )
{
  if ( mKMailIcalIfaceStub && mKMailIcalIfaceStub->app() == appId ) {
    // Delete the stub so that the next time we need the addressbook,
    // we'll know that we need to start a new one.
    delete mKMailIcalIfaceStub;
    mKMailIcalIfaceStub = 0;
  }
}


#include "kmailconnection.moc"
