/*
    This file is part of libkabc and/or kaddressbook.
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

#ifndef KABC_RESOURCEKOLAB_H
#define KABC_RESOURCEKOLAB_H

#include <libkdepim/resourceabc.h>
#include <dcopobject.h>
#include "../shared/resourcekolabbase.h"
#include "../shared/subresource.h"

namespace KABC {

  class FormatPlugin;

/**
 * This class implements a KAddressBook resource that keeps its
 * addresses in an Kolab folder in KMail (or other conforming email
 * clients).
 */
class ResourceKolab : public KPIM::ResourceABC,
                     public Kolab::ResourceKolabBase
{
  Q_OBJECT

public:
  /**
   * Constructor
   */
  ResourceKolab( const KConfig* );

  /**
   * Destructor.
   */
  virtual ~ResourceKolab();

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
  void fromKMailAddSubresource( const QString& type, const QString& id,
                                const QString& label, bool writable );
  void fromKMailDelSubresource( const QString& type, const QString& id );

  bool fromKMailAddIncidence( const QString& type, const QString& resource,
                              Q_UINT32 sernum, const QString& contact );
  void fromKMailDelIncidence( const QString& type, const QString& resource,
                              const QString& contact );
  void fromKMailRefresh( const QString& type, const QString& resource );

  void fromKMailAsyncLoadResult( const QMap<Q_UINT32, QString>& map,
                                 const QString& type,
                                 const QString& folder );

  /// Return the list of subresources.
  QStringList subresources() const;

  /// Is this subresource active?
  bool subresourceActive( const QString& ) const;

  // ############ TODO
  virtual void setSubresourceActive( const QString &, bool ) {}

  /// Completion weight for a given subresource
  virtual int subresourceCompletionWeight( const QString& ) const;

  /// Set completion weight for a given subresource
  virtual void setSubresourceCompletionWeight( const QString&, int );

  /// Give the uidmap. Used for ordered searching
  QMap<QString, QString> uidToResourceMap() const;

signals:
  void signalSubresourceAdded( Resource*, const QString&, const QString& );
  void signalSubresourceRemoved( Resource*, const QString&, const QString& );

protected:
  bool kmailUpdateAddressee( const Addressee& );

  void doClose();

  void loadSubResourceConfig( KConfig& config, const QString& name,
                              const QString& label, bool writable );
  bool loadSubResource( const QString& subResource );
  void loadContact( const QString& contactXML, const QString& subResource, Q_UINT32 sernum );

  QString configFile() const {
    return Kolab::ResourceKolabBase::configFile( "kabc" );
  }

  // The list of subresources
  Kolab::ResourceMap mSubResources;
};

}

#endif // KABC_RESOURCEKOLAB_H
