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

#ifndef RESOURCEKOLABBASE_H
#define RESOURCEKOLABBASE_H

#include <tqstring.h>
#include <tqmap.h>
#include <tqstringlist.h>

#include "subresource.h"
#include <kmail/kmailicalIface.h>

class TQCString;
class KURL;

namespace Kolab {

enum ResourceType { Tasks, Events, Incidences, Contacts, Notes };

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
  ResourceKolabBase( const TQCString& objId );
  virtual ~ResourceKolabBase();

  // These are the methods called by KMail when the resource changes
  virtual bool fromKMailAddIncidence( const TQString& type,
                                      const TQString& resource,
                                      Q_UINT32 sernum,
                                      int format,
                                      const TQString& data ) = 0;
  virtual void fromKMailDelIncidence( const TQString& type,
                                      const TQString& resource,
                                      const TQString& xml ) = 0;
  virtual void fromKMailRefresh( const TQString& type,
                                 const TQString& resource ) = 0;
  virtual void fromKMailAddSubresource( const TQString& type,
                                        const TQString& resource,
                                        const TQString& label,
                                        bool writable,
                                        bool alarmRelevant ) = 0;
  virtual void fromKMailDelSubresource( const TQString& type,
                                        const TQString& resource ) = 0;

  virtual void fromKMailAsyncLoadResult( const TQMap<Q_UINT32, TQString>& map,
                                         const TQString& type,
                                         const TQString& folder ) = 0;
protected:
  /// Do the connection to KMail.
  bool connectToKMail() const;

  // These are the KMail dcop function connections. The docs here say
  // "Get", which here means that the first argument is the return arg

  /// List all folders with a certain contentsType. Returns a TQMap with
  /// resourcename/writable pairs
  bool kmailSubresources( TQValueList<KMailICalIface::SubResource>& lst,
                          const TQString& contentsType ) const;

  /// Get the number of messages in this folder.
  /// Used to iterate over kmailIncidences by chunks
  bool kmailIncidencesCount( int& count, const TQString& mimetype,
                             const TQString& resource ) const;

  /// Get the mimetype attachments from a chunk of messages from this folder.
  /// Returns a TQMap with serialNumber/attachment pairs.
  bool kmailIncidences( TQMap<Q_UINT32, TQString>& lst, const TQString& mimetype,
                        const TQString& resource,
                        int startIndex,
                        int nbMessages ) const;

  bool kmailTriggerSync( const TQString& contentType ) const;

public: // for Contact
  /// Get an attachment from a mail. Returns a URL to it. This can
  /// be called by the resource after obtaining the incidence.
  /// The resource must delete the temp file.
  bool kmailGetAttachment( KURL& url, const TQString& resource,
                           Q_UINT32 sernum,
                           const TQString& filename ) const;

  /** Get the mimetype of the specified attachment. */
  bool kmailAttachmentMimetype( TQString &mimeType, TQString &resource,
                                Q_UINT32 sernum, const TQString &filename ) const;

  /// List all attachments of a mail.
  bool kmailListAttachments( TQStringList &list, const TQString &resource,
                             Q_UINT32 sernum ) const;

protected:
  /// Delete an incidence.
  bool kmailDeleteIncidence( const TQString& resource, Q_UINT32 sernum );

  KMailICalIface::StorageFormat kmailStorageFormat( const TQString& folder ) const;

  typedef TQMap<TQCString, TQString> CustomHeaderMap;

  /// Update an incidence. The list of attachments are URLs.
  /// The parameter sernum is updated with the right KMail serial number
  bool kmailUpdate( const TQString& resource, Q_UINT32& sernum,
                    const TQString& xml,
                    const TQString& mimetype,
                    const TQString& subject,
                    const CustomHeaderMap& customHeaders = CustomHeaderMap(),
                    const TQStringList& attachmentURLs = TQStringList(),
                    const TQStringList& attachmentMimetypes = TQStringList(),
                    const TQStringList& attachmentNames = TQStringList(),
                    const TQStringList& deletedAttachments = TQStringList() );

  bool kmailAddSubresource( const TQString& resource, const TQString& parent,
                            const TQString& contentsType );
  bool kmailRemoveSubresource( const TQString& resource );

  /// Get the full path of the config file.
  TQString configFile( const TQString& type ) const;

  /// If only one of these is writable, return that. Otherwise return null.
  TQString findWritableResource( const ResourceType &type,
                                const ResourceMap& resources,
                                const TQString& text = TQString::null );

  enum ErrorCode {
    NoError,
    NoWritableFound,   /**< No writable resource is available */
    UserCancel         /**< User canceled the operation */
  };
  ErrorCode mErrorCode;

  bool mSilent;

  /**
   * This is used to store a mapping from the XML UID to the KMail
   * serial number of the mail it's stored in. That provides a quick way
   * to access the storage in KMail.
   */
  UidMap mUidMap;

  /// This is used to distinguish operations triggered by the user,
  /// from operations triggered by KMail
  TQStringList mUidsPendingAdding;
  TQStringList mUidsPendingDeletion;
  TQStringList mUidsPendingUpdate;

private:
  mutable KMailConnection* mConnection;
};

}

#endif // RESOURCEKOLABBASE_H
