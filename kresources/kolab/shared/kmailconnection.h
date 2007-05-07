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

#ifndef KMAILCONNECTION_H
#define KMAILCONNECTION_H

#include <QObject>
#include <kmail/kmailicalIface.h>
#include "kolabshared_export.h"

class KUrl;
class QString;
class OrgKdeKmailGroupwareInterface;

namespace Kolab {

class ResourceKolabBase;

/**
  This class provides the kmail connectivity for IMAP resources.
*/
class KOLABSHARED_EXPORT KMailConnection : public QObject {
  Q_OBJECT

  // These are the methods called by KMail when the resource changes
private Q_SLOTS:
  bool fromKMailAddIncidence( const QString& type, const QString& resource,
                              quint32 sernum, int format, const QString& xml );
  void fromKMailDelIncidence( const QString& type, const QString& resource,
                              const QString& xml );
  void fromKMailRefresh( const QString& type, const QString& resource );
  void fromKMailAddSubresource( const QString& type, const QString& resource, const QString& label );
  void fromKMailDelSubresource( const QString& type, const QString& resource );
  void fromKMailAsyncLoadResult( const QMap<quint32, QString>& map, const QString& type,
                                 const QString& folder );

public:
  KMailConnection( ResourceKolabBase* resource );
  virtual ~KMailConnection();

  /**
   * Do the connection to KMail.
   */
  bool connectToKMail();

  // Call the DBus methods
  bool kmailSubresources( QList<KMailICalIface::SubResource>& lst,
                          const QString& contentsType );
  bool kmailIncidencesCount( int& count,
                             const QString& mimetype,
                             const QString& resource );
  bool kmailIncidences( QMap<quint32, QString>& lst, const QString& mimetype,
                        const QString& resource,
                        int startIndex,
                        int nbMessages );

  bool kmailGetAttachment( KUrl& url, const QString& resource, quint32 sernum,
                           const QString& filename );
  bool kmailDeleteIncidence( const QString& resource, quint32 sernum );
  bool kmailUpdate( const QString& resource,
                    quint32& sernum,
                    const QString& subject,
                    const QString& plainTextBody,
                    const QMap<QByteArray, QString>& customHeaders,
                    const QStringList& attachmentURLs,
                    const QStringList& attachmentMimetypes,
                    const QStringList& attachmentNames,
                    const QStringList& deletedAttachments );

  bool kmailStorageFormat( KMailICalIface::StorageFormat& type, const QString& folder);

  bool kmailTriggerSync( const QString& contentsType );

private slots:
  void dbusServiceOwnerChanged(const QString & service, const QString & oldOwner, const QString & newOwner);

private:
  /** Connect a signal from KMail to a local slot. */
  bool connectKMailSignal( const QByteArray&, const QByteArray& );

  ResourceKolabBase* mResource;
  OrgKdeKmailGroupwareInterface* mKmailGroupwareInterface;
};

}

#endif // KMAILCONNECTION_H
