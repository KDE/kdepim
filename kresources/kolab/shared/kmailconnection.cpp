/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

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

#include "kmailconnection.h"
#include "resourcekolabbase.h"

#include <kdebug.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdcopservicestarter.h>
#include <klocale.h>

#include "kmailicalIface_stub.h"


using namespace Kolab;


KMailConnection::KMailConnection( ResourceKolabBase* resource,
                                  const TQCString& objId )
  : DCOPObject( objId ), mResource( resource ), mKMailIcalIfaceStub( 0 )
{
  // Make the connection to KMail ready
  mDCOPClient = new DCOPClient();
  mDCOPClient->attach();
  mDCOPClient->registerAs( objId, true );

  kapp->dcopClient()->setNotifications( true );
  connect( kapp->dcopClient(), TQT_SIGNAL( applicationRemoved( const TQCString& ) ),
           this, TQT_SLOT( unregisteredFromDCOP( const TQCString& ) ) );
}

KMailConnection::~KMailConnection()
{
  kapp->dcopClient()->setNotifications( false );
  delete mKMailIcalIfaceStub;
  mKMailIcalIfaceStub = 0;
  delete mDCOPClient;
  mDCOPClient = 0;
}

static const TQCString dcopObjectId = "KMailICalIface";
bool KMailConnection::connectToKMail()
{
  if ( !mKMailIcalIfaceStub ) {
    TQCString dcopService;

    // if we are kmail (and probably kontact as well) ourselves, don't try to start us again
    // this prevents a DCOP deadlock when launching the kmail while kontact is the IMAP backend
    // provider (and probably vice versa)
    if ( kapp->instanceName() == "kmail" ) {
      // someone, probably ourselves, already offers the interface, if not stop here
      const QCStringList services = kapp->dcopClient()->registeredApplications();
      for ( uint i = 0; i < services.count(); ++i ) {
        if ( services[i].find( "anonymous" ) == 0 ) // querying anonymous-XXXXX deadlocks as well, what are those anyway?
          continue; 
        const QCStringList objs = kapp->dcopClient()->remoteObjects( services[i] );
        if ( objs.contains( dcopObjectId ) ) {
          dcopService = services[i];
          break;
        }
      }
      if ( dcopService.isEmpty() ) {
        kdError(5650) << k_funcinfo << "Not connecting to KMail to prevent DCOP deadlock" << endl;
        return false;
      }
    } else {
      TQString error;
      int result = KDCOPServiceStarter::self()->
        findServiceFor( "DCOP/ResourceBackend/IMAP", TQString::null,
                        TQString::null, &error, &dcopService );
      if ( result != 0 ) {
        kdError(5650) << "Couldn't connect to the IMAP resource backend\n";
        // TODO: You might want to show "error" (if not empty) here,
        // using e.g. KMessageBox
        return false;
      }
    }

    mKMailIcalIfaceStub = new KMailICalIface_stub( kapp->dcopClient(),
                                                   dcopService, dcopObjectId );

    // Attach to the KMail signals
    if ( !connectKMailSignal( "incidenceAdded(TQString,TQString,Q_UINT32,int,TQString)",
                              "fromKMailAddIncidence(TQString,TQString,Q_UINT32,int,TQString)" ) )
      kdError(5650) << "DCOP connection to incidenceAdded failed" << endl;
    if ( !connectKMailSignal( "incidenceDeleted(TQString,TQString,TQString)",
                              "fromKMailDelIncidence(TQString,TQString,TQString)" ) )
      kdError(5650) << "DCOP connection to incidenceDeleted failed" << endl;
    if ( !connectKMailSignal( "signalRefresh(TQString,TQString)",
                              "fromKMailRefresh(TQString,TQString)" ) )
      kdError(5650) << "DCOP connection to signalRefresh failed" << endl;
    if ( !connectKMailSignal( "subresourceAdded( TQString, TQString, TQString, bool, bool )",
                              "fromKMailAddSubresource( TQString, TQString, TQString, bool, bool )" ) )
      kdError(5650) << "DCOP connection to subresourceAdded failed" << endl;
    if ( !connectKMailSignal( "subresourceDeleted(TQString,TQString)",
                              "fromKMailDelSubresource(TQString,TQString)" ) )
      kdError(5650) << "DCOP connection to subresourceDeleted failed" << endl;
    if ( !connectKMailSignal( "asyncLoadResult(TQMap<Q_UINT32, TQString>, TQString, TQString)",
                              "fromKMailAsyncLoadResult(TQMap<Q_UINT32, TQString>, TQString, TQString)" ) )
      kdError(5650) << "DCOP connection to asyncLoadResult failed" << endl;
  }

  return ( mKMailIcalIfaceStub != 0 );
}

bool KMailConnection::fromKMailAddIncidence( const TQString& type,
                                             const TQString& folder,
                                             Q_UINT32 sernum,
                                             int format,
                                             const TQString& data )
{
  if ( format != KMailICalIface::StorageXML
      && format != KMailICalIface::StorageIcalVcard )
    return false;
//   kdDebug(5650) << "KMailConnection::fromKMailAddIncidence( " << type << ", "
//                 << folder << " ). iCal:\n" << ical << endl;
  return mResource->fromKMailAddIncidence( type, folder, sernum, format, data );
}

void KMailConnection::fromKMailDelIncidence( const TQString& type,
                                             const TQString& folder,
                                             const TQString& xml )
{
//   kdDebug(5650) << "KMailConnection::fromKMailDelIncidence( " << type << ", "
//                 << folder << ", " << uid << " )\n";
  mResource->fromKMailDelIncidence( type, folder, xml );
}

void KMailConnection::fromKMailRefresh( const TQString& type, const TQString& folder )
{
//   kdDebug(5650) << "KMailConnection::fromKMailRefresh( " << type << ", "
//                 << folder << " )\n";
  mResource->fromKMailRefresh( type, folder );
}

void KMailConnection::fromKMailAddSubresource( const TQString& type,
                                               const TQString& resource,
                                               const TQString& label,
                                               bool writable,
                                               bool alarmRelevant )
{
//   kdDebug(5650) << "KMailConnection::fromKMailAddSubresource( " << type << ", "
//                 << resource << " )\n";
  mResource->fromKMailAddSubresource( type, resource, label,
                                      writable, alarmRelevant );
}

void KMailConnection::fromKMailDelSubresource( const TQString& type,
                                               const TQString& resource )
{
//   kdDebug(5650) << "KMailConnection::fromKMailDelSubresource( " << type << ", "
//                 << resource << " )\n";
  mResource->fromKMailDelSubresource( type, resource );
}

void KMailConnection::fromKMailAsyncLoadResult( const TQMap<Q_UINT32, TQString>& map,
                                                const TQString& type,
                                                const TQString& folder )
{
  mResource->fromKMailAsyncLoadResult( map, type, folder );
}

bool KMailConnection::connectKMailSignal( const TQCString& signal,
                                          const TQCString& method )
{
  return connectDCOPSignal( "kmail", dcopObjectId, signal, method, false )
    && connectDCOPSignal( "kontact", dcopObjectId, signal, method, false );
}

bool KMailConnection::kmailSubresources( TQValueList<KMailICalIface::SubResource>& lst,
                                         const TQString& contentsType )
{
  if ( !connectToKMail() )
    return false;

  lst = mKMailIcalIfaceStub->subresourcesKolab( contentsType );
  return mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailIncidencesCount( int& count,
                                            const TQString& mimetype,
                                            const TQString& resource )
{
  if ( !connectToKMail() )
    return false;

  count = mKMailIcalIfaceStub->incidencesKolabCount( mimetype, resource );
  return mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailIncidences( TQMap<Q_UINT32, TQString>& lst,
                                       const TQString& mimetype,
                                       const TQString& resource,
                                       int startIndex,
                                       int nbMessages )
{
  if ( !connectToKMail() )
    return false;

  lst = mKMailIcalIfaceStub->incidencesKolab( mimetype, resource, startIndex, nbMessages );
  return mKMailIcalIfaceStub->ok();
}


bool KMailConnection::kmailGetAttachment( KURL& url,
                                          const TQString& resource,
                                          Q_UINT32 sernum,
                                          const TQString& filename )
{
  if ( !connectToKMail() )
    return false;

  url = mKMailIcalIfaceStub->getAttachment( resource, sernum, filename );
  return mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailAttachmentMimetype( TQString & mimeType,
                                               const TQString & resource,
                                               Q_UINT32 sernum,
                                               const TQString & filename )
{
  if ( !connectToKMail() )
    return false;
  mimeType = mKMailIcalIfaceStub->attachmentMimetype( resource, sernum, filename );
  return mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailListAttachments(TQStringList &list,
                                            const TQString & resource, Q_UINT32 sernum)
{
  if ( !connectToKMail() )
    return false;

  list = mKMailIcalIfaceStub->listAttachments( resource, sernum );
  return mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailDeleteIncidence( const TQString& resource,
                                            Q_UINT32 sernum )
{
  return connectToKMail()
    && mKMailIcalIfaceStub->deleteIncidenceKolab( resource, sernum )
    && mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailUpdate( const TQString& resource,
                                   Q_UINT32& sernum,
                                   const TQString& subject,
                                   const TQString& plainTextBody,
                                   const TQMap<TQCString, TQString>& customHeaders,
                                   const TQStringList& attachmentURLs,
                                   const TQStringList& attachmentMimetypes,
                                   const TQStringList& attachmentNames,
                                   const TQStringList& deletedAttachments )
{
  //kdDebug(5006) << kdBacktrace() << endl;
  if ( connectToKMail() ) {
    sernum = mKMailIcalIfaceStub->update( resource, sernum, subject, plainTextBody, customHeaders,
                                          attachmentURLs, attachmentMimetypes, attachmentNames,
                                          deletedAttachments );
    return sernum && mKMailIcalIfaceStub->ok();
  } else
    return false;
}

bool KMailConnection::kmailAddSubresource( const TQString& resource,
                                           const TQString& parent,
                                           const TQString& contentsType )
{
  return connectToKMail()
      && mKMailIcalIfaceStub->addSubresource( resource, parent, contentsType )
      && mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailRemoveSubresource( const TQString& resource )
{
  return connectToKMail()
      && mKMailIcalIfaceStub->removeSubresource( resource )
      && mKMailIcalIfaceStub->ok();
}


bool KMailConnection::kmailStorageFormat( KMailICalIface::StorageFormat& type,
                                          const TQString& folder )
{
  bool ok = connectToKMail();
  type = mKMailIcalIfaceStub->storageFormat( folder );
  return ok && mKMailIcalIfaceStub->ok();
}


bool KMailConnection::kmailTriggerSync( const TQString &contentsType )
{
  bool ok = connectToKMail();
  return ok && mKMailIcalIfaceStub->triggerSync( contentsType );
}

void KMailConnection::unregisteredFromDCOP( const TQCString& appId )
{
  if ( mKMailIcalIfaceStub && mKMailIcalIfaceStub->app() == appId ) {
    // Delete the stub so that the next time we need to talk to kmail,
    // we'll know that we need to start a new one.
    delete mKMailIcalIfaceStub;
    mKMailIcalIfaceStub = 0;
  }
}

#include "kmailconnection.moc"
