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

#ifndef RESOURCEKOLABBASE_H
#define RESOURCEKOLABBASE_H

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>

#include "subresource.h"
#include <kmail/kmailicalIface.h>

class QCString;
class KURL;

namespace Kolab {

class KMailConnection;

/**
  This class provides the kmail connectivity for IMAP resources.

  The main methods are:

  fromKMail...() : calls made _by_ KMail to add/delete data representation in the resource.

  kmail...()     : calls _into_ KMail made by the resource.

  e.g. fromKMailAddIncidence() is called by KMail
       when a new iCard is there after an IMAP sync.

       By calling fromKMailAddIncidence() KMail notifies
       the resource about the new incidence, so in the
       addressbook a new address will appear like magic.

  e.g. kmailAddIncidence() is called by the resource when
       iCard must be stored by KMail because the user has added
       an address in the addressbook.

       By calling kmailAddIncidence() the resource causes
       KMail to store the new address in the (IMAP) folder.
*/
class ResourceKolabBase {
public:
  ResourceKolabBase( const QCString& objId );
  virtual ~ResourceKolabBase();

  // These are the methods called by KMail when the resource changes
  virtual bool fromKMailAddIncidence( const QString& type,
                                      const QString& resource,
                                      Q_UINT32 sernum,
                                      const QString& xml ) = 0;
  virtual void fromKMailDelIncidence( const QString& type,
                                      const QString& resource,
                                      const QString& xml ) = 0;
  virtual void fromKMailRefresh( const QString& type,
                                 const QString& resource ) = 0;
  virtual void fromKMailAddSubresource( const QString& type,
                                        const QString& resource,
                                        const QString& label,
                                        bool writable ) = 0;
  virtual void fromKMailDelSubresource( const QString& type,
                                        const QString& resource ) = 0;

  virtual void fromKMailAsyncLoadResult( const QMap<Q_UINT32, QString>& map,
                                         const QString& type,
                                         const QString& folder ) = 0;
protected:
  /// Do the connection to KMail.
  bool connectToKMail() const;

  // These are the KMail dcop function connections. The docs here say
  // "Get", which here means that the first argument is the return arg

  /// List all folders with a certain contentsType. Returns a QMap with
  /// resourcename/writable pairs
  bool kmailSubresources( QValueList<KMailICalIface::SubResource>& lst,
                          const QString& contentsType ) const;

  /// Get the mimetype attachments from this folder. Returns a
  /// QMap with serialNumber/attachment pairs.
  bool kmailIncidences( QMap<Q_UINT32, QString>& lst, const QString& mimetype,
                        const QString& resource ) const;

public: // for Contact
  /// Get an attachment from a mail. Returns a URL to it. This can
  /// be called by the resource after obtaining the incidence.
  /// The resource must delete the temp file.
  bool kmailGetAttachment( KURL& url, const QString& resource,
                           Q_UINT32 sernum,
                           const QString& filename ) const;

protected:
  /// Delete an incidence.
  bool kmailDeleteIncidence( const QString& resource, Q_UINT32 sernum );

  /// Update an incidence. The list of attachments are URLs.
  /// The parameter sernum is updated with the right KMail serial number
  bool kmailUpdate( const QString& resource, Q_UINT32& sernum,
                    const QString& xml,
                    const QString& mimetype,
                    const QString& subject,
                    const QStringList& attachmentURLs = QStringList(),
                    const QStringList& attachmentMimetypes = QStringList(),
                    const QStringList& attachmentNames = QStringList(),
                    const QStringList& deletedAttachments = QStringList() );

  /// Get the full path of the config file.
  QString configFile( const QString& type ) const;

  /// If only one of these is writable, return that. Otherwise return null.
  QString findWritableResource( const ResourceMap& resources );

  bool mSilent;

  /**
   * This is used to store a mapping from the XML UID to the KMail
   * serial number of the mail it's stored in. That provides a quick way
   * to access the storage in KMail.
   */
  UidMap mUidMap;

  /// This is used to distinguish operations triggered by the user,
  /// from operations triggered by KMail
  QStringList mUidsPendingAdding;
  QStringList mUidsPendingDeletion;
  QStringList mUidsPendingUpdate;

private:
  mutable KMailConnection* mConnection;
};

}

#endif // RESOURCEKOLABBASE_H
