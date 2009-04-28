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
#include <kurl.h>
#include <kdbusservicestarter.h>
#include <kmail/groupware_types.h>
#include <klocale.h>
#include <QDBusAbstractInterface>
#include <QMap>

using namespace Kolab;

KMailConnection::KMailConnection( ResourceKolabBase* resource )
  : mResource( resource )
  , mKmailGroupwareInterface( 0 )
{
  QObject::connect(QDBusConnection::sessionBus().interface(),
                   SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                   this, SLOT(dbusServiceOwnerChanged(QString,QString,QString)));

  KMail::registerGroupwareTypes();
}


KMailConnection::~KMailConnection()
{
  delete mKmailGroupwareInterface;
  mKmailGroupwareInterface = 0;
}

bool KMailConnection::connectToKMail()
{
  if ( !mKmailGroupwareInterface ) {
    QString error;
    QString dbusService;
    int result = KDBusServiceStarter::self()->
      findServiceFor( "DBUS/ResourceBackend/IMAP", QString(),
                      &error, &dbusService );
    if ( result != 0 ) {
      kError(5650) <<"Couldn't connect to the IMAP resource backend";
      // TODO: You might want to show "error" (if not empty) here,
      // using e.g. KMessageBox
      return false;
    }
    kDebug(5650) << "Connected to the KMail DBus interface.";
    mKmailGroupwareInterface = new OrgKdeKmailGroupwareInterface( dbusService, KMAIL_DBUS_GROUPWARE_PATH,
                                                                  QDBusConnection::sessionBus() );
    if ( !mKmailGroupwareInterface->isValid() )
      kWarning(5650) << "The groupware interface is not valid, race condition!?";

    mOldServiceName = mKmailGroupwareInterface->service();

    connect( mKmailGroupwareInterface, SIGNAL(incidenceAdded(QString,QString,uint,int,QString)),
             SLOT(fromKMailAddIncidence(QString,QString,uint,int,QString)) );
    connect( mKmailGroupwareInterface, SIGNAL(incidenceDeleted(QString,QString,QString)),
             SLOT(fromKMailDelIncidence(QString,QString,QString)) );
    connect( mKmailGroupwareInterface, SIGNAL(signalRefresh(QString,QString)),
             SLOT( fromKMailRefresh(QString,QString)) );
    connect( mKmailGroupwareInterface, SIGNAL(subresourceAdded(QString,QString,QString,bool,bool)),
             SLOT(fromKMailAddSubresource( QString, QString, QString, bool, bool )) );
    connect( mKmailGroupwareInterface, SIGNAL(subresourceDeleted(QString,QString)),
             SLOT(fromKMailDelSubresource(QString,QString)) );
    connect( mKmailGroupwareInterface, SIGNAL(asyncLoadResult(QMap<quint32,QString>,QString,QString)),
             SLOT( fromKMailAsyncLoadResult(QMap<quint32,QString>,QString,QString)) );
  }
  return ( mKmailGroupwareInterface != 0 );
}

bool KMailConnection::fromKMailAddIncidence( const QString& type,
                                             const QString& folder,
                                             uint sernum,
                                             int format,
                                             const QString& data )
{
  if ( format != KMail::StorageXML
      && format != KMail::StorageIcalVcard )
    return false;
//   kDebug(5650) <<"KMailConnection::fromKMailAddIncidence(" << type <<","
//                 << folder << "). iCal:\n" << ical;
  return mResource->fromKMailAddIncidence( type, folder, sernum, format, data );
}

void KMailConnection::fromKMailDelIncidence( const QString& type,
                                             const QString& folder,
                                             const QString& xml )
{
  kDebug(5650) <<"KMailConnection::fromKMailDelIncidence(" << type <<","
                << folder << ", " << xml << " )";
  mResource->fromKMailDelIncidence( type, folder, xml );
}

void KMailConnection::fromKMailRefresh( const QString& type, const QString& folder )
{
//   kDebug(5650) <<"KMailConnection::fromKMailRefresh(" << type <<","
//                 << folder << " )";
  mResource->fromKMailRefresh( type, folder );
}

void KMailConnection::fromKMailAddSubresource( const QString& type,
                                               const QString& resource,
                                               const QString& label,
                                               bool writable,
                                               bool alarmRelevant )
{
//   kDebug(5650) <<"KMailConnection::fromKMailAddSubresource(" << type <<","
//                 << resource << " )";
  mResource->fromKMailAddSubresource( type, resource, label,
                                      writable, alarmRelevant );
}

void KMailConnection::fromKMailDelSubresource( const QString& type,
                                               const QString& resource )
{
//   kDebug(5650) <<"KMailConnection::fromKMailDelSubresource(" << type <<","
//                 << resource << " )";
  mResource->fromKMailDelSubresource( type, resource );
}

void KMailConnection::fromKMailAsyncLoadResult( const QMap<quint32, QString>& map,
                                                const QString& type,
                                                const QString& folder )
{
  mResource->fromKMailAsyncLoadResult( map, type, folder );
}

bool KMailConnection::kmailSubresources( QList<KMail::SubResource>& lst,
                                         const QString& contentsType )
{
  if ( !connectToKMail() )
    return false;
  return checkReply<QList<KMail::SubResource>, QList<KMail::SubResource> >( mKmailGroupwareInterface->subresourcesKolab( contentsType ), lst );
}

bool KMailConnection::kmailIncidencesCount( int& count,
                                            const QString& mimetype,
                                            const QString& resource )
{
  if ( !connectToKMail() )
    return false;
  return checkReply<int, int>( mKmailGroupwareInterface->incidencesKolabCount( mimetype, resource ), count );
}

bool KMailConnection::kmailIncidences( KMail::SernumDataPair::List& lst,
                                       const QString& mimetype,
                                       const QString& resource,
                                       int startIndex,
                                       int nbMessages )
{
  if ( !connectToKMail() )
    return false;
  return checkReply<QList<KMail::SernumDataPair>, QList<KMail::SernumDataPair> >(
      mKmailGroupwareInterface->incidencesKolab( mimetype, resource, startIndex, nbMessages ), lst );
}


bool KMailConnection::kmailGetAttachment( KUrl& url,
                                          const QString& resource,
                                          quint32 sernum,
                                          const QString& filename )
{
  if ( !connectToKMail() )
    return false;
  return checkReply<QString, KUrl>( mKmailGroupwareInterface->getAttachment( resource, sernum, filename ), url );
}

bool KMailConnection::kmailAttachmentMimetype( QString & mimeType,
                                               const QString & resource,
                                               quint32 sernum,
                                               const QString & filename )
{
  if ( !connectToKMail() )
    return false;
  return checkReply<QString, QString>( mKmailGroupwareInterface->attachmentMimetype( resource, sernum, filename ), mimeType );
}

bool KMailConnection::kmailListAttachments(QStringList &list,
                                            const QString & resource, quint32 sernum)
{
  if ( !connectToKMail() )
    return false;
  return checkReply<QStringList, QStringList>( mKmailGroupwareInterface->listAttachments( resource, sernum ), list );
}

bool KMailConnection::kmailDeleteIncidence( const QString& resource,
                                            quint32 sernum )
{
  if ( !connectToKMail() )
    return false;
  return checkReply( mKmailGroupwareInterface->deleteIncidenceKolab( resource, sernum ) );
}

bool KMailConnection::kmailUpdate( const QString& resource,
                                   quint32& sernum,
                                   const QString& subject,
                                   const QString& plainTextBody,
                                   const KMail::CustomHeader::List& customHeaders,
                                   const QStringList& attachmentURLs,
                                   const QStringList& attachmentMimetypes,
                                   const QStringList& attachmentNames,
                                   const QStringList& deletedAttachments )
{
  if ( !connectToKMail() )
    return false;
  return checkReply<quint32,quint32>( mKmailGroupwareInterface->update(
      resource,
      sernum,
      subject,
      plainTextBody,
      customHeaders,
      attachmentURLs,
      attachmentMimetypes,
      attachmentNames,
      deletedAttachments ), sernum );
}

bool KMailConnection::kmailAddSubresource( const QString& resource,
                                           const QString& parent,
                                           const QString& contentsType )
{
  if ( !connectToKMail() )
    return false;
  return checkReply( mKmailGroupwareInterface->addSubresource( resource, parent, contentsType ) );
}

bool KMailConnection::kmailRemoveSubresource( const QString& resource )
{
  if ( !connectToKMail() )
    return false;
  return checkReply( mKmailGroupwareInterface->removeSubresource( resource ) );
}


bool KMailConnection::kmailStorageFormat( KMail::StorageFormat& type,
                                          const QString& folder )
{
  if ( !connectToKMail() )
    return false;
  QDBusReply<int> reply = mKmailGroupwareInterface->storageFormat( folder );
  if ( reply.isValid() )
    type = static_cast<KMail::StorageFormat>( reply.value() );
  return mKmailGroupwareInterface->lastError().type() == QDBusError::NoError;
}


bool KMailConnection::kmailTriggerSync( const QString &contentsType )
{
  if ( !connectToKMail() )
    return false;
  return checkReply( mKmailGroupwareInterface->triggerSync( contentsType ) );
}

void KMailConnection::dbusServiceOwnerChanged( const QString &service, const QString &oldOwner,
                                               const QString &newOwner )
{
  // The owner of the D-Bus service we're interested in changed, so either connect or disconnect.
  if ( mOldServiceName == service && !service.isEmpty() ) {
    if ( mKmailGroupwareInterface )
    {
      // Delete the stub so that the next time we need to talk to kmail,
      // we'll know that we need to start a new one.
      delete mKmailGroupwareInterface;
      mKmailGroupwareInterface = 0;
    }
    else {
      const bool kmailJustStarted = oldOwner.isEmpty();
      if ( kmailJustStarted ) { // Vampire protection
        if ( !connectToKMail() ) {
          kWarning(5650) << "Could not connect to KMail, even though the D-Bus service just became available!";
        }
      }
    }
  }
}

#include "kmailconnection.moc"
