/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2002 - 2004 Klar�vdalens Datakonsult AB
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

#ifndef KABC_RESOURCEIMAP_H
#define KABC_RESOURCEIMAP_H

#include <kabc/vcardconverter.h>
#include <libkdepim/resourceabc.h>
#include <dcopobject.h>
#include <resourceimapshared.h>


namespace KABC {

  class FormatPlugin;

/**
 * This class implements a KAddressBook resource that keeps its
 * addresses in an IMAP folder in KMail (or other conforming email
 * clients).
 */
class ResourceIMAP : public KPIM::ResourceABC,
                     public ResourceIMAPBase::ResourceIMAPShared
{
  Q_OBJECT

public:
  /**
   * Constructor
   */
  ResourceIMAP( const KConfig* );

  /**
   * Destructor.
   */
  virtual ~ResourceIMAP();

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
  void subresourceAdded( const QString& type, const QString& id );
  void subresourceDeleted( const QString& type, const QString& id );
  void asyncLoadResult( const QStringList&, const QString&, const QString& );

  bool addIncidence( const QString& type, const QString& resource,
                     const QString& ical );
  void deleteIncidence( const QString& type, const QString& resource,
                        const QString& uid );
  void slotRefresh( const QString& type, const QString& resource );

  /// Return the list of subresources.
  QStringList subresources() const;

  /// Return the list of active subresources.
  QStringList activeSubresources() const;

  /// Is this subresource active?
  bool subresourceActive( const QString& ) const;

  // ############ TODO
  virtual void setSubresourceActive( const QString &, bool ) {}

  /// Completion weight for a given subresource
  virtual int subresourceCompletionWeight( const QString& ) const;

  /// Label for a given subresource
  virtual QString subresourceLabel( const QString& ) const;

  /// Set completion weight for a given subresource
  virtual void setSubresourceCompletionWeight( const QString&, int );

  /// Give the uidmap. Used for ordered searching
  QMap<QString, QString> uidToResourceMap() const { return mUidmap; }

signals:
  void signalSubresourceAdded( Resource*, const QString&, const QString& );
  void signalSubresourceRemoved( Resource*, const QString&, const QString& );

protected:
  void insertAddressee( const Addressee&, const QString& resource );
  void doClose();

  void loadSubResourceConfig( KConfig& config, const QString& name );
  bool loadResource( const QString& resource );

  // parse a list of vcards and add the result to the resource
  void populate( const QStringList&, const QString& resource );

  QString configFile() const {
    return ResourceIMAPBase::ResourceIMAPShared::configFile( "kabc" );
  }

  FormatPlugin* mFormat;
  QStringList mDeletedAddressees;
  QCString mAppId;

  KABC::VCardConverter mConverter;

  struct SubResource {
    SubResource() : active(false) {} // for qmap
    SubResource( bool _active, int _completionWeight )
      : active( _active ), completionWeight( _completionWeight ) {}
    bool active;
    int completionWeight;
  };

  // The list of subresources
  typedef QMap<QString, SubResource> ResourceMap;
  ResourceMap mResources;
  // Mapping from uid to resource
  QMap<QString, QString> mUidmap;
};

}

#endif
