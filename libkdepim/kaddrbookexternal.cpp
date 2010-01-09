/*
  Simple Addressbook for KMail
  Copyright Stefan Taferner <taferner@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kaddrbookexternal.h"

#include <akonadi/collection.h>
#include <akonadi/collectiondialog.h>
#include <akonadi/contact/contacteditordialog.h>
#include <akonadi/contact/contactgroupexpandjob.h>
#include <akonadi/contact/contactgroupsearchjob.h>
#include <akonadi/contact/contactsearchjob.h>
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>
#include <kabc/contactgroup.h>

#include <KLocale>
#include <KMessageBox>
#include <KToolInvocation>

using namespace KPIM;

void KAddrBookExternal::openEmail( const QString &email, const QString &addr, QWidget *parentWidget )
{
  Akonadi::Item item;
  Akonadi::Item::List items;

  // search whether contact with that email address already exists
  Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob;
  searchJob->setQuery( Akonadi::ContactSearchJob::Email, email );
  if ( searchJob->exec() )
    items = searchJob->items();

  if ( items.isEmpty() ) { // if not...

    // ask user in which address book the new contact shall be stored
    const QStringList mimeTypes( KABC::Addressee::mimeType() );
    Akonadi::CollectionDialog dlg;
    dlg.setMimeTypeFilter( mimeTypes );
    dlg.setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
    dlg.setCaption( i18n( "Select Address Book" ) );
    dlg.setDescription( i18n( "Select the address book the new contact shall be saved in:" ) );
    if ( !dlg.exec() )
      return;

    const Akonadi::Collection addressBook = dlg.selectedCollection();
    if ( !addressBook.isValid() )
      return;

    // create the new contact
    QString email;
    QString name;

    KABC::Addressee::parseEmailAddress( addr, name, email );
    KABC::Addressee contact;
    contact.setNameFromString( name );
    contact.insertEmail( email, true );

    // create the new item
    Akonadi::Item item;
    item.setMimeType( KABC::Addressee::mimeType() );
    item.setPayload<KABC::Addressee>( contact );

    // save the item in akonadi storage
    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, addressBook );
    if ( !job->exec() )
      return;

    // the new item is the matching item
    item = job->item();
  } else {

    // the found item is the matching item
    item = items.first();
  }

  // open the editor with the matching item
  Akonadi::ContactEditorDialog dlg( Akonadi::ContactEditorDialog::EditMode, parentWidget );
  dlg.setContact( item );
  dlg.exec();
}

void KAddrBookExternal::addEmail( const QString &addr, QWidget *parent )
{
  // extract name and email from email address string
  QString email;
  QString name;

  KABC::Addressee::parseEmailAddress( addr, name, email );

  KABC::Addressee::List contacts;

  // search whether a contact with the same email exists already
  Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob;
  searchJob->setQuery( Akonadi::ContactSearchJob::Email, email );
  if ( searchJob->exec() )
    contacts = searchJob->contacts();

  if ( contacts.isEmpty() ) { // if not, add a new one
    KABC::Addressee contact;
    contact.setNameFromString( name );
    contact.insertEmail( email, true );

    if ( KAddrBookExternal::addAddressee( contact ) ) {
      QString text = i18n( "<qt>The email address <b>%1</b> was added to your "
                           "address book; you can add more information to this "
                           "entry by opening the address book.</qt>", addr );
      KMessageBox::information( parent, text, QString(), "addedtokabc" );
    }
  } else { // inform the user otherwise
    QString text =
      i18n( "<qt>The email address <b>%1</b> is already in your address book.</qt>", addr );
    KMessageBox::information( parent, text, QString(), "alreadyInAddressBook" );
  }
}

void KAddrBookExternal::openAddressBook( QWidget * )
{
  KToolInvocation::startServiceByDesktopName( "kaddressbook" );
}

void KAddrBookExternal::addNewAddressee( QWidget *parentWidget )
{
  Akonadi::ContactEditorDialog dlg( Akonadi::ContactEditorDialog::CreateMode, parentWidget );
  dlg.exec();
}

bool KAddrBookExternal::addVCard( const KABC::Addressee &addressee, QWidget *parent )
{
  bool inserted = false;

  KABC::Addressee::List contacts;

  // check whether a contact with the same email address exists already
  Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob;
  searchJob->setQuery( Akonadi::ContactSearchJob::Email, addressee.preferredEmail() );
  if ( searchJob->exec() )
    contacts = searchJob->contacts();

  if ( contacts.isEmpty() ) { // if not, add a new one
    if ( KAddrBookExternal::addAddressee( addressee ) ) {
      QString text = i18n( "The VCard was added to your address book; "
                           "you can add more information to this "
                           "entry by opening the address book." );
      KMessageBox::information( parent, text, QString(), "addedtokabc" );
      inserted = true;
    }
  } else { // inform the user otherwise
    QString text = i18n( "The VCard's primary email address is already in "
                         "your address book; however, you may save the VCard "
                         "into a file and import it into the address book manually." );
    KMessageBox::information( parent, text );
    inserted = true;
  }

  return inserted;
}

bool KAddrBookExternal::addAddressee( const KABC::Addressee &addr )
{
  // ask user in which address book the new contact shall be stored
  const QStringList mimeTypes( KABC::Addressee::mimeType() );
  Akonadi::CollectionDialog dlg;
  dlg.setMimeTypeFilter( mimeTypes );
  dlg.setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
  dlg.setCaption( i18n( "Select Address Book" ) );
  dlg.setDescription( i18n( "Select the address book the new contact shall be saved in:" ) );

  if ( !dlg.exec() )
    return false;

  const Akonadi::Collection addressBook = dlg.selectedCollection();
  if ( !addressBook.isValid() )
    return false;

  // create the new item
  Akonadi::Item item;
  item.setMimeType( KABC::Addressee::mimeType() );
  item.setPayload<KABC::Addressee>( addr );

  // save the new item in akonadi storage
  Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, addressBook );
  if ( !job->exec() )
    return false;

  return true;
}

QString KAddrBookExternal::expandDistributionList( const QString &listName, bool &emptyList )
{
  emptyList = false;
  if ( listName.isEmpty() )
    return QString();

  // search the contact group by name
  Akonadi::ContactGroupSearchJob *job = new Akonadi::ContactGroupSearchJob;
  job->setQuery( Akonadi::ContactGroupSearchJob::Name, listName );
  if ( !job->exec() )
    return QString();

  const KABC::ContactGroup::List groups = job->contactGroups();
  if ( groups.isEmpty() )
    return QString();

  // expand the contact group to a list of email addresses
  Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob( groups.first() );
  if ( !expandJob->exec() )
    return QString();

  const KABC::Addressee::List contacts = expandJob->contacts();

  QStringList emails;
  foreach ( const KABC::Addressee &contact, contacts )
    emails.append( contact.fullEmail() );

  const QString listOfEmails = emails.join( ", " );
  emptyList = listOfEmails.isEmpty();

  return listOfEmails;
}
