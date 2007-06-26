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
#include <kdbusservicestarter.h>
#include <kmail_groupwareinterface.h>
#include <kmail_util.h>
#include <klocale.h>
#include <QDBusAbstractInterface>
#include <QDBusError>

using namespace Kolab;
typedef QList<KMail::SubResource> QListKmailSubResource;
typedef QMap<quint32, QString> Quint32StringMap;
typedef QMap<QByteArray, QString> ByteArrayStringMap;
Q_DECLARE_METATYPE(ByteArrayStringMap)

Q_DECLARE_METATYPE(KMail::SubResource)
Q_DECLARE_METATYPE(QListKmailSubResource)
Q_DECLARE_METATYPE(Quint32StringMap)
Q_DECLARE_METATYPE(KMail::StorageFormat)

const QDBusArgument &operator<<(QDBusArgument &arg, const KMail::SubResource &subResource)
{
    arg.beginStructure();
    arg << subResource.location << subResource.label << subResource.writable << subResource.alarmRelevant;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, KMail::SubResource &subResource)
{
    arg.beginStructure();
    arg >> subResource.location >> subResource.label >> subResource.writable >> subResource.alarmRelevant;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator<<(QDBusArgument &arg, const KMail::StorageFormat &format)
{
    arg.beginStructure();
    quint32 foo = format;
    arg << foo;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, KMail::StorageFormat &format)
{
    arg.beginStructure();
    quint32 foo = format;
    arg >> foo;
    arg.endStructure();
    return arg;
}

static void registerTypes()
{
    static bool registered = false;
    if (!registered) {
      qDBusRegisterMetaType<KMail::SubResource>();
      qDBusRegisterMetaType< QListKmailSubResource >();
      qDBusRegisterMetaType<Quint32StringMap>();
      qDBusRegisterMetaType<KMail::StorageFormat>();
      qDBusRegisterMetaType<ByteArrayStringMap>();
      registered = true;
    }
}

KMailConnection::KMailConnection( ResourceKolabBase* resource )
  : mResource( resource )
  , mKmailGroupwareInterface( 0 )
{
  // Make the connection to KMail ready
  QObject::connect(QDBusConnection::sessionBus().interface(),
                   SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                   this, SLOT(dbusServiceOwnerChanged(QString,QString,QString)));

}


KMailConnection::~KMailConnection()
{
  delete mKmailGroupwareInterface;
  mKmailGroupwareInterface = 0;
}

bool KMailConnection::connectToKMail()
{
   kDebug()<<" bool KMailConnection::connectToKMail()\n";
  if ( !mKmailGroupwareInterface ) {
    QString error;
    QString dbusService;
    int result = KDBusServiceStarter::self()->
      findServiceFor( "DBUS/ResourceBackend/IMAP", QString(),
                      &error, &dbusService );
    if ( result != 0 ) {
      kError(5650) << "Couldn't connect to the IMAP resource backend\n";
      // TODO: You might want to show "error" (if not empty) here,
      // using e.g. KMessageBox
      return false;
    }
    mKmailGroupwareInterface = new QDBusInterface( dbusService, "/Groupware" , "org.kde.kmail.groupware", QDBusConnection::sessionBus());
    registerTypes();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "incidenceAdded", this, SLOT(fromKMailAddIncidence(QString,QString,quint32,int,QString) ) );
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "incidenceDeleted", this, SLOT( fromKMailDelIncidence(QString,QString,QString) ) );
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "signalRefresh", this, SLOT( fromKMailRefresh(QString,QString) ) );
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "subresourceAdded", this, SLOT(fromKMailAddSubresource( QString, QString, QString, bool, bool ) ) );
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "subresourceDeleted", this, SLOT(fromKMailDelSubresource(QString,QString) ) );
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "asyncLoadResult", this, SLOT( fromKMailAsyncLoadResult(QMap<quint32, QString>, QString, QString) ) );

/*

    // Attach to the KMail signals
    if ( !connectKMailSignal( "incidenceAdded(QString,QString,quint32,int,QString)",
                              "fromKMailAddIncidence(QString,QString,quint32,int,QString)" ) )
      kError(5650) << "DCOP connection to incidenceAdded failed" << endl;
    if ( !connectKMailSignal( "incidenceDeleted(QString,QString,QString)",
                              "fromKMailDelIncidence(QString,QString,QString)" ) )
      kError(5650) << "DCOP connection to incidenceDeleted failed" << endl;
    if ( !connectKMailSignal( "signalRefresh(QString,QString)",
                              "fromKMailRefresh(QString,QString)" ) )
      kError(5650) << "DCOP connection to signalRefresh failed" << endl;
    if ( !connectKMailSignal( "subresourceAdded( QString, QString, QString, bool, bool )",
                              "fromKMailAddSubresource( QString, QString, QString, bool, bool )" ) )
      kError(5650) << "DCOP connection to subresourceAdded failed" << endl;
    if ( !connectKMailSignal( "subresourceDeleted(QString,QString)",
                              "fromKMailDelSubresource(QString,QString)" ) )
      kError(5650) << "DCOP connection to subresourceDeleted failed" << endl;
    if ( !connectKMailSignal( "asyncLoadResult(QMap<quint32, QString>, QString, QString)",
                              "fromKMailAsyncLoadResult(QMap<quint32, QString>, QString, QString)" ) )
      kError(5650) << "DCOP connection to asyncLoadResult failed" << endl;
*/
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
//   kDebug(5650) << "KMailConnection::fromKMailAddIncidence( " << type << ", "
//                 << folder << " ). iCal:\n" << ical << endl;
  return mResource->fromKMailAddIncidence( type, folder, sernum, format, data );
}

void KMailConnection::fromKMailDelIncidence( const QString& type,
                                             const QString& folder,
                                             const QString& xml )
{
//   kDebug(5650) << "KMailConnection::fromKMailDelIncidence( " << type << ", "
//                 << folder << ", " << uid << " )\n";
  mResource->fromKMailDelIncidence( type, folder, xml );
}

void KMailConnection::fromKMailRefresh( const QString& type, const QString& folder )
{
//   kDebug(5650) << "KMailConnection::fromKMailRefresh( " << type << ", "
//                 << folder << " )\n";
  mResource->fromKMailRefresh( type, folder );
}

void KMailConnection::fromKMailAddSubresource( const QString& type,
                                               const QString& resource,
                                               const QString& label,
                                               bool writable,
                                               bool alarmRelevant )
{
//   kDebug(5650) << "KMailConnection::fromKMailAddSubresource( " << type << ", "
//                 << resource << " )\n";
  mResource->fromKMailAddSubresource( type, resource, label,
                                      writable, alarmRelevant );
}

void KMailConnection::fromKMailDelSubresource( const QString& type,
                                               const QString& resource )
{
//   kDebug(5650) << "KMailConnection::fromKMailDelSubresource( " << type << ", "
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

  QDBusReply<QListKmailSubResource> r = mKmailGroupwareInterface->call( "subresourcesKolab", contentsType );
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

bool KMailConnection::kmailIncidences( QMap<quint32, QString>& lst,
                                       const QString& mimetype,
                                       const QString& resource,
                                       int startIndex,
                                       int nbMessages )
{
  if ( !connectToKMail() )
    return false;
  registerTypes();
  QDBusReply<Quint32StringMap> r = mKmailGroupwareInterface->call( "incidencesKolab",  mimetype, resource, startIndex, nbMessages );
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
                                   const QMap<QByteArray, QString>& customHeaders,
                                   const QStringList& attachmentURLs,
                                   const QStringList& attachmentMimetypes,
                                   const QStringList& attachmentNames,
                                   const QStringList& deletedAttachments )
{
  if ( !connectToKMail() )
    return false;
  registerTypes();
  bool ret = false;
  //TODO fix me
  /*QList<QVariant> arg;
  arg <<resource;
  arg<<sernum;
  arg<<subject;
  arg<<plainTextBody;
  arg<<customHeaders;
  arg<<attachmentURLs;
  arg<<attachmentMimetypes;
  arg<<attachmentNames;
  arg<<deletedAttachments;
  QDBusReply<bool> reply = mKmailGroupwareInterface->callWithArgumentList( QDBus::NoBlock, "update", arg );
  if ( reply.isValid() )
  ret = reply;*/
  return ret;
}

bool KMailConnection::kmailStorageFormat( KMail::StorageFormat& type,
                                          const QString& folder )
{
  bool ok = connectToKMail();
  QDBusReply<KMail::StorageFormat> reply = mKmailGroupwareInterface->call( "storageFormat", folder );
  if ( reply.isValid() )
    type = reply;
  return ok && (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}


bool KMailConnection::kmailTriggerSync( const QString &contentsType )
{
  bool ok = connectToKMail();
  QDBusReply<bool> val =  mKmailGroupwareInterface->call( "triggerSync", contentsType );
  bool ret = val;
  return ok && ret;
}

void KMailConnection::dbusServiceOwnerChanged(const QString & service, const QString & oldOwner, const QString & newOwner)
{
  if (mKmailGroupwareInterface && mKmailGroupwareInterface->service()==service)
  {
    // Delete the stub so that the next time we need to talk to kmail,
    // we'll know that we need to start a new one.
    delete mKmailGroupwareInterface;
    mKmailGroupwareInterface = 0;
  }
}

#include "kmailconnection.moc"
