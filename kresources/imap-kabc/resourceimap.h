/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2002 Klarälvdalens Datakonsult AB 
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
*/

#ifndef RESOURCEIMAP_H
#define RESOURCEIMAP_H

#include <kabc/resource.h>

namespace KABC {

    class FormatPlugin;

/**
 * This class implements a KAddressBook resource that keeps its
 * addresses in an IMAP folder in KMail (or other conforming email
 * clients).
 */
class ResourceIMAP : public Resource
{
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
   * Writes back all settings to config file.
   */
  virtual void writeConfig( KConfig* );

  /**
   * Open the resource and returns if it was successfully
   */
  virtual bool doOpen();

  /**
   * Close the resource and returns if it was successfully
   */
  virtual void doClose();

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
     Loads all addressees asyncronously. You have to make sure that either
     the loadingFinished() or loadingError() signal is emitted from within
     this function.

     @return Whether the synchronous part of loading was successfully.
  */
  virtual bool asyncLoad();

  /**
   * Save all addressees to the addressbook.
   *
   * @param ticket The ticket you get by requestSaveTicket()
   */
  virtual bool save( Ticket *ticket );

  /**
     Saves all addressees asynchronously. You have to make sure that either
     the savingFinished() or savingError() signal is emitted from within
     this function.

     @param ticket You have to release the ticket later with
     releaseSaveTicket() explicitely.
     @return Whether the saving was successfully.
  */
  virtual bool asyncSave( Ticket *ticket );

  /**
   * Removes a addressee from resource. This method is mainly
   * used by record-based resources like LDAP or SQL.
   */
  virtual void removeAddressee( const Addressee& addr );

private:
    FormatPlugin* mFormat;
    QStringList mDeletedAddressees;
    QCString mAppId;
};

}

#endif
