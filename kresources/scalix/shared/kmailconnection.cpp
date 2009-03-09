/*
    This file is part of the scalix resource - based on the kolab resource.

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
#include "resourcescalixbase.h"

#include <kdebug.h>
#include <kurl.h>
#include <kdbusservicestarter.h>
#include <kmail_groupwareinterface.h>
#include <kmail/groupware_types.h>
#include <klocale.h>
#include <QDBusAbstractInterface>
#include <QDBusError>
#include <QMap>

using namespace Scalix;

static void registerTypes()
{
    static bool registered = false;
    if (!registered) {
      KMail::registerGroupwareTypes();
      registered = true;
    }
}

KMailConnection::KMailConnection( ResourceScalixBase* resource )
  : mResource( resource )
  , mKmailGroupwareInterface( 0 )
{
  // Make the connection to KMail ready
  QObject::connect(QDBusConnection::sessionBus().interface(),
                   SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                   this, SLOT(dbusServiceOwnerChanged(QString,QString,QString)));

  registerTypes();
}


KMailConnection::~KMailConnection()
{
  delete mKmailGroupwareInterface;
  mKmailGroupwareInterface = 0;
}

bool KMailConnection::connectToKMail()
{
   kDebug()<<" bool KMailConnection::connectToKMail()";
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
    mKmailGroupwareInterface = new QDBusInterface( dbusService, KMAIL_DBUS_GROUPWARE_PATH, KMAIL_DBUS_GROUPWARE_INTERFACE, QDBusConnection::sessionBus());
    registerTypes();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect( KMAIL_DBUS_SERVICE, KMAIL_DBUS_GROUPWARE_PATH, KMAIL_DBUS_GROUPWARE_INTERFACE, "incidenceAdded", this, SLOT(fromKMailAddIncidence(QString,QString,quint32,int,QString) ) );
    dbus.connect( KMAIL_DBUS_SERVICE, KMAIL_DBUS_GROUPWARE_PATH, KMAIL_DBUS_GROUPWARE_INTERFACE, "incidenceDeleted", this, SLOT( fromKMailDelIncidence(QString,QString,QString) ) );
    dbus.connect( KMAIL_DBUS_SERVICE, KMAIL_DBUS_GROUPWARE_PATH, KMAIL_DBUS_GROUPWARE_INTERFACE, "signalRefresh", this, SLOT( fromKMailRefresh(QString,QString) ) );
    dbus.connect( KMAIL_DBUS_SERVICE, KMAIL_DBUS_GROUPWARE_PATH, KMAIL_DBUS_GROUPWARE_INTERFACE, "subresourceAdded", this, SLOT(fromKMailAddSubresource( QString, QString, QString, bool, bool ) ) );
    dbus.connect( KMAIL_DBUS_SERVICE, KMAIL_DBUS_GROUPWARE_PATH, KMAIL_DBUS_GROUPWARE_INTERFACE, "subresourceDeleted", this, SLOT(fromKMailDelSubresource(QString,QString) ) );
    dbus.connect( KMAIL_DBUS_SERVICE, KMAIL_DBUS_GROUPWARE_PATH, KMAIL_DBUS_GROUPWARE_INTERFACE, "asyncLoadResult", this, SLOT( fromKMailAsyncLoadResult(QMap<quint32, QString>, QString, QString) ) );

  }
  return ( mKmailGroupwareInterface != 0 );
}

bool KMailConnection::fromKMailAddIncidence( const QString& type,
                                             const QString& folder,
                                             quint32 sernum,
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
//   kDebug(5650) <<"KMailConnection::fromKMailDelIncidence(" << type <<","
//                 << folder << ", " << uid << " )\n";
  mResource->fromKMailDelIncidence( type, folder, xml );
}

void KMailConnection::fromKMailRefresh( const QString& type, const QString& folder )
{
//   kDebug(5650) <<"KMailConnection::fromKMailRefresh(" << type <<","
//                 << folder << " )\n";
  mResource->fromKMailRefresh( type, folder );
}

void KMailConnection::fromKMailAddSubresource( const QString& type,
                                               const QString& resource,
                                               const QString& label,
                                               bool writable,
                                               bool alarmRelevant )
{
//   kDebug(5650) <<"KMailConnection::fromKMailAddSubresource(" << type <<","
//                 << resource << " )\n";
  mResource->fromKMailAddSubresource( type, resource, label,
                                      writable, alarmRelevant );
}

void KMailConnection::fromKMailDelSubresource( const QString& type,
                                               const QString& resource )
{
//   kDebug(5650) <<"KMailConnection::fromKMailDelSubresource(" << type <<","
//                 << resource << " )\n";
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
  registerTypes();

  QDBusReply<KMail::SubResource::List> r = mKmailGroupwareInterface->call( "subresourcesKolab", contentsType );
  if ( r.isValid() )
  {
    lst = r;
  }
  return (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}

bool KMailConnection::kmailIncidencesCount( int& count,
                                            const QString& mimetype,
                                            const QString& resource )
{
  if ( !connectToKMail() )
    return false;

  QDBusReply<int> val= mKmailGroupwareInterface->call( "incidencesKolabCount", mimetype, resource );
  count = val;
  return (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}

bool KMailConnection::kmailIncidences( KMail::SernumDataPair::List& lst,
                                       const QString& mimetype,
                                       const QString& resource,
                                       int startIndex,
                                       int nbMessages )
{
  if ( !connectToKMail() )
    return false;
  registerTypes();
  QDBusReply<KMail::SernumDataPair::List> r = mKmailGroupwareInterface->call( "incidencesKolab",  mimetype, resource, startIndex, nbMessages );
  if (r.isValid()) {
    lst = r.value();
  }
  return (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}


bool KMailConnection::kmailGetAttachment( KUrl& url,
                                          const QString& resource,
                                          quint32 sernum,
                                          const QString& filename )
{
  if ( !connectToKMail() )
    return false;

  QDBusReply<QString> val = mKmailGroupwareInterface->call( "getAttachment", resource, sernum, filename );
  QString tmp = val;
  url = KUrl( val );
  return (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}

bool KMailConnection::kmailListAttachments(QStringList &list,
                                            const QString & resource, quint32 sernum)
{
  if ( !connectToKMail() )
    return false;

  QDBusReply<QStringList> r = mKmailGroupwareInterface->call( "listAttachments", resource, sernum );
  list = r;
  return (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}

bool KMailConnection::kmailDeleteIncidence( const QString& resource,
                                            quint32 sernum )
{
  QDBusReply<bool> deleteIncidence = mKmailGroupwareInterface->call( "deleteIncidenceKolab", resource, sernum );
  bool b = deleteIncidence;
  return connectToKMail()
    && b
    && (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
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
  bool ok = connectToKMail();

  QList<QVariant> arguments;
  arguments << resource;
  arguments << sernum;
  arguments << subject;
  arguments << plainTextBody;
  arguments << QVariant::fromValue( customHeaders );
  arguments << attachmentURLs;
  arguments << attachmentMimetypes;
  arguments << attachmentNames;
  arguments << deletedAttachments;

  QDBusReply<quint32> reply = mKmailGroupwareInterface->callWithArgumentList( QDBus::Block, "update", arguments );

  if ( reply.isValid() )
    sernum = reply;

  return ok && (mKmailGroupwareInterface->lastError().type() == QDBusError::NoError);
}

bool KMailConnection::kmailAddSubresource( const QString& resource,
                                           const QString& parent,
                                           const QString& contentsType )
{
  bool ok = connectToKMail();
  QDBusReply<bool> reply = mKmailGroupwareInterface->call( "addSubresource", resource, parent, contentsType );
  if ( reply.isValid() )
    ok = reply;
  return ok && (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}

bool KMailConnection::kmailRemoveSubresource( const QString& resource )
{
  bool ok = connectToKMail();
  QDBusReply<bool> reply = mKmailGroupwareInterface->call( "removeSubresource", resource );
  if ( reply.isValid() )
    ok = reply;
  return ok && (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}


bool KMailConnection::kmailStorageFormat( KMail::StorageFormat& type,
                                          const QString& folder )
{
  bool ok = connectToKMail();
  QDBusReply<int> reply = mKmailGroupwareInterface->call( "storageFormat", folder );
  if ( reply.isValid() )
    type = static_cast<KMail::StorageFormat>( reply.value() );
  return ok && (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}


bool KMailConnection::kmailTriggerSync( const QString &contentsType )
{
  bool ok = connectToKMail();
  QDBusReply<bool> val =  mKmailGroupwareInterface->call( "triggerSync", contentsType );
  bool ret = val;
  return ok && ret;
}

void KMailConnection::dbusServiceOwnerChanged(const QString & service, const QString&, const QString&)
{
  if (mKmailGroupwareInterface && mKmailGroupwareInterface->service() == service)
  {
    // Delete the stub so that the next time we need to talk to kmail,
    // we'll know that we need to start a new one.
    delete mKmailGroupwareInterface;
    mKmailGroupwareInterface = 0;
  }
}

#include "kmailconnection.moc"
