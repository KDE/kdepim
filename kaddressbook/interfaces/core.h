/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KAB_CORE_H
#define KAB_CORE_H

#include <qobject.h>

#include <kabc/field.h>
#include <kxmlguiclient.h>
#include <libkdepim/distributionlist.h>

namespace KABC {
class AddressBook;
class Resource;
}

class QWidget;

class KActionCollection;
class KConfig;
class KURL;

namespace KAB {

class SearchManager;

class Core : public QObject
{
  Q_OBJECT

  public:
    Core( KXMLGUIClient *client, QObject *parent, const char *name = 0 );

    /**
      Returns a pointer to the StdAddressBook of the application.
     */
    virtual KABC::AddressBook *addressBook() const = 0;

    /**
      Returns a pointer to the KConfig object of the application.
     */
    virtual KConfig *config() const = 0;

    /**
      Returns a pointer to the global KActionCollection object. So
      other classes can register their actions easily.
     */
    virtual KActionCollection *actionCollection() const = 0;

    /**
      Returns a pointer to the gui client.
     */
    virtual KXMLGUIClient *guiClient() const { return mGUIClient; }

    /**
      Returns the current sort field.
     */
    virtual KABC::Field *currentSortField() const = 0;

    /**
      Returns the uid list of the currently selected contacts.
     */
    virtual QStringList selectedUIDs() const = 0;

    /**
      Displays a ResourceSelectDialog and returns the selected
      resource or a null pointer if no resource was selected by
      the user.
     */
    virtual KABC::Resource *requestResource( QWidget *parent ) = 0;

    /**
      Returns the parent widget.
     */
    virtual QWidget *widget() const = 0;

    /**
      Deletes given contacts from the address book.

      @param uids The uids of the contacts, which shall be deleted.
     */
    virtual void deleteContacts( const QStringList &uids ) = 0;

    /**
      Returns all the distribution lists.
     */
    virtual KPIM::DistributionList::List distributionLists() const = 0;

    /**
      Returns the name of all the distribution lists.
     */
    virtual QStringList distributionListNames() const = 0;


    ////// This class isn't part of interfaces, so this method being here isn't really useful
    virtual SearchManager *searchManager() const = 0;

  public slots:
    /**
      Is called whenever a contact is selected in the view.
     */
    virtual void setContactSelected( const QString &uid ) = 0;

    /**
      DCOP METHOD: Adds the given email address to address book.
     */
    virtual void addEmail( const QString& addr ) = 0;

    /**
      DCOP METHOD: Imports the vCard, located at the given url.
     */
    virtual void importVCard( const KURL& url ) = 0;

    /**
      DCOP METHOD: Imports the given vCard.
     */
    virtual void importVCard( const QString& vCard ) = 0;

    /**
      DCOP METHOD: Opens contact editor to input a new contact.
     */
    virtual void newContact() = 0;

    /**
      DCOP METHOD: Returns the name of the contact, that matches the given
                   phone number.
     */
    virtual QString getNameByPhone( const QString& phone ) = 0;

    /**
      Shows an edit dialog for the given uid.
     */
    virtual void editContact( const QString &uid = QString::null ) = 0;

  signals:
    /**
      Forwarded by SearchManager
      Means distributionListNames might result something different.
     */
    void contactsUpdated();

  private:
    KXMLGUIClient *mGUIClient;
};

}

#endif
