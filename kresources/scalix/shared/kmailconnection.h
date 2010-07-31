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

#ifndef KMAILCONNECTION_H
#define KMAILCONNECTION_H

#include <dcopobject.h>
#include <kmail/kmailicalIface.h>

class KURL;
class DCOPClient;
class KMailICalIface_stub;

namespace Scalix {

class ResourceScalixBase;

/**
  This class provides the kmail connectivity for IMAP resources.
*/
class KMailConnection : public TQObject, public DCOPObject {
  Q_OBJECT
  K_DCOP

  // These are the methods called by KMail when the resource changes
k_dcop:
  bool fromKMailAddIncidence( const TQString& type, const TQString& resource,
                              Q_UINT32 sernum, int format, const TQString& xml );
  void fromKMailDelIncidence( const TQString& type, const TQString& resource,
                              const TQString& xml );
  void fromKMailRefresh( const TQString& type, const TQString& resource );
  void fromKMailAddSubresource( const TQString& type, const TQString& resource, const TQString& label );
  void fromKMailDelSubresource( const TQString& type, const TQString& resource );
  void fromKMailAsyncLoadResult( const TQMap<Q_UINT32, TQString>& map, const TQString& type,
                                 const TQString& folder );

public:
  KMailConnection( ResourceScalixBase* resource, const TQCString& objId );
  virtual ~KMailConnection();

  /**
   * Do the connection to KMail.
   */
  bool connectToKMail();

  // Call the DCOP methods
  bool kmailSubresources( TQValueList<KMailICalIface::SubResource>& lst,
                          const TQString& contentsType );
  bool kmailIncidencesCount( int& count,
                             const TQString& mimetype,
                             const TQString& resource );
  bool kmailIncidences( TQMap<Q_UINT32, TQString>& lst, const TQString& mimetype,
                        const TQString& resource,
                        int startIndex,
                        int nbMessages );

  bool kmailGetAttachment( KURL& url, const TQString& resource, Q_UINT32 sernum,
                           const TQString& filename );
  bool kmailDeleteIncidence( const TQString& resource, Q_UINT32 sernum );
  bool kmailUpdate( const TQString& resource,
                    Q_UINT32& sernum,
                    const TQString& subject,
                    const TQString& plainTextBody,
                    const TQMap<TQCString, TQString>& customHeaders,
                    const TQStringList& attachmentURLs,
                    const TQStringList& attachmentMimetypes,
                    const TQStringList& attachmentNames,
                    const TQStringList& deletedAttachments );

  bool kmailStorageFormat( KMailICalIface::StorageFormat& type, const TQString& folder);

  bool kmailTriggerSync( const TQString& contentsType );

private slots:
  virtual void unregisteredFromDCOP( const TQCString& );

private:
  /** Connect a signal from KMail to a local slot. */
  bool connectKMailSignal( const TQCString&, const TQCString& );

  ResourceScalixBase* mResource;
  DCOPClient* mDCOPClient;
  KMailICalIface_stub* mKMailIcalIfaceStub;
};

}

#endif // KMAILCONNECTION_H
