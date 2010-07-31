/*
    This file is part of the scalix resource - based on the kolab resource.

    Copyright (c) 2002 - 2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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

#ifndef KABC_RESOURCESCALIX_H
#define KABC_RESOURCESCALIX_H

#include <libkdepim/resourceabc.h>
#include <dcopobject.h>
#include "../shared/resourcescalixbase.h"
#include "../shared/subresource.h"
#include <kmail/kmailicalIface.h>
#include <kdepimmacros.h>

namespace KABC {

  class FormatPlugin;

/**
 * This class implements a KAddressBook resource that keeps its
 * addresses in an Scalix folder in KMail (or other conforming email
 * clients).
 */
class KDE_EXPORT ResourceScalix : public KPIM::ResourceABC,
                     public Scalix::ResourceScalixBase
{
  Q_OBJECT

public:
  /**
   * Constructor
   */
  ResourceScalix( const KConfig* );

  /**
   * Destructor.
   */
  virtual ~ResourceScalix();

  /**
   * Open the contacts list
   */
  virtual bool doOpen();

  /**
   * Request a ticket, you have to pass through save() to
   * allow locking.
   */
  virtual Ticket *requestSaveTicket();

  /**
     Releases the ticket previousely requested with requestSaveTicket().
     The resource has to remove its locks in this function.
  */
  virtual void releaseSaveTicket( Ticket* );

  /**
  * Load all addressees to the addressbook
   */
  virtual bool load();

  /**
  * Save all addressees to the addressbook.
   *
   * @param ticket The ticket you get by requestSaveTicket()
   */
  virtual bool save( Ticket *ticket );

  /**
     Insert an addressee into the resource.
  */
  virtual void insertAddressee( const Addressee& );

  /**
  * Removes a addressee from resource. This method is mainly
   * used by record-based resources like LDAP or SQL.
   */
  virtual void removeAddressee( const Addressee& addr );

  // Listen to KMail changes in the amount of sub resources
  void fromKMailAddSubresource( const TQString& type, const TQString& id,
                                const TQString& label, bool writable );
  void fromKMailDelSubresource( const TQString& type, const TQString& id );

  bool fromKMailAddIncidence( const TQString& type, const TQString& resource,
                              Q_UINT32 sernum, int format, const TQString& contact );
  void fromKMailDelIncidence( const TQString& type, const TQString& resource,
                              const TQString& contact );
  void fromKMailRefresh( const TQString& type, const TQString& resource );

  void fromKMailAsyncLoadResult( const TQMap<Q_UINT32, TQString>& map,
                                 const TQString& type,
                                 const TQString& folder );

  /// Return the list of subresources.
  TQStringList subresources() const;

  /// Is this subresource active?
  bool subresourceActive( const TQString& ) const;
  /// Is this subresource writabel?
  bool subresourceWritable( const TQString& ) const;

  virtual void setSubresourceActive( const TQString &, bool );

  /// Completion weight for a given subresource
  virtual int subresourceCompletionWeight( const TQString& ) const;

  /// Label for a given subresource
  virtual TQString subresourceLabel( const TQString& ) const;

  /// Set completion weight for a given subresource
  virtual void setSubresourceCompletionWeight( const TQString&, int );

  /// Give the uidmap. Used for ordered searching
  TQMap<TQString, TQString> uidToResourceMap() const;

protected:
  bool kmailUpdateAddressee( const Addressee& );

  void doClose();

  void loadSubResourceConfig( KConfig& config, const TQString& name,
                              const TQString& label, bool writable );
  bool loadSubResource( const TQString& subResource );
  bool loadSubResourceHelper( const TQString& subResource, const char* mimetype, KMailICalIface::StorageFormat format );
  TQString loadContact( const TQString& contactData, const TQString& subResource,
                       Q_UINT32 sernum, const KMailICalIface::StorageFormat format );

  TQString configFile() const {
    return Scalix::ResourceScalixBase::configFile( "kabc" );
  }

  // The list of subresources
  Scalix::ResourceMap mSubResources;
  TQString mCachedSubresource;
  bool mLocked;
};

}

#endif // KABC_RESOURCESCALIX_H
