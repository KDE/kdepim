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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

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

#include <dcopobject.h>
#include <kmail/kmailicalIface.h>

class KURL;
class DCOPClient;
class KMailICalIface_stub;

namespace Kolab {

class ResourceKolabBase;

/**
  This class provides the kmail connectivity for IMAP resources.
*/
class KMailConnection : public QObject, public DCOPObject {
  Q_OBJECT
  K_DCOP

  // These are the methods called by KMail when the resource changes
k_dcop:
  bool fromKMailAddIncidence( const QString& type, const QString& resource,
                              Q_UINT32 sernum, int format, const QString& xml );
  void fromKMailDelIncidence( const QString& type, const QString& resource,
                              const QString& xml );
  void fromKMailRefresh( const QString& type, const QString& resource );
  void fromKMailAddSubresource( const QString& type, const QString& resource, const QString& label );
  void fromKMailDelSubresource( const QString& type, const QString& resource );
  void fromKMailAsyncLoadResult( const QMap<Q_UINT32, QString>& map, const QString& type,
                                 const QString& folder );

public:
  KMailConnection( ResourceKolabBase* resource, const QCString& objId );
  virtual ~KMailConnection();

  /**
   * Do the connection to KMail.
   */
  bool connectToKMail();

  // Call the DCOP methods
  bool kmailSubresources( QValueList<KMailICalIface::SubResource>& lst,
                          const QString& contentsType );
  bool kmailIncidences( QMap<Q_UINT32, QString>& lst, const QString& mimetype,
                        const QString& resource );
  bool kmailGetAttachment( KURL& url, const QString& resource, Q_UINT32 sernum,
                           const QString& filename );
  bool kmailDeleteIncidence( const QString& resource, Q_UINT32 sernum );
  bool kmailUpdate( const QString& resource,
                    Q_UINT32& sernum,
                    const QString& subject,
                    const QString& plainTextBody,
                    const QMap<QCString, QString>& customHeaders,
                    const QStringList& attachmentURLs,
                    const QStringList& attachmentMimetypes,
                    const QStringList& attachmentNames,
                    const QStringList& deletedAttachments );

private slots:
  virtual void unregisteredFromDCOP( const QCString& );

private:
  /** Connect a signal from KMail to a local slot. */
  bool connectKMailSignal( const QCString&, const QCString& );

  ResourceKolabBase* mResource;
  DCOPClient* mDCOPClient;
  KMailICalIface_stub* mKMailIcalIfaceStub;
};

}

#endif // KMAILCONNECTION_H
