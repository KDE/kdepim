/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "vcardviewer.h"

#include <akonadi/collectiondialog.h>
#include <akonadi/contact/contactsearchjob.h>
#include <akonadi/contact/contactviewer.h>
#include <akonadi/itemcreatejob.h>

#include <kabc/vcardconverter.h>
#include <kabc/addressee.h>
using KABC::VCardConverter;
using KABC::Addressee;

#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QPointer>
#include <QtCore/QString>

Q_DECLARE_METATYPE( KABC::Addressee )

using namespace MessageViewer;

VCardViewer::VCardViewer(QWidget *parent, const QByteArray& vCard)
  : KDialog( parent )
{
  setCaption( i18n("VCard Viewer") );
  setButtons( User1|User2|User3|Close );
  setModal( false );
  setDefaultButton( Close );
  setButtonGuiItem( User1, KGuiItem(i18n("&Import")) );
  setButtonGuiItem( User2, KGuiItem(i18n("&Next Card")) );
  setButtonGuiItem( User3, KGuiItem(i18n("&Previous Card")) );
  mContactViewer = new Akonadi::ContactViewer(this);
  setMainWidget(mContactViewer);

  VCardConverter vcc;
  mAddresseeList = vcc.parseVCards( vCard );
  if ( !mAddresseeList.empty() ) {
    itAddresseeList = mAddresseeList.begin();
    mContactViewer->setRawContact( *itAddresseeList );
    if ( mAddresseeList.size() <= 1 ) {
      showButton(User2, false);
      showButton(User3, false);
    }
    else
      enableButton(User3, false);
  }
  else {
    mContactViewer->setRawContact(KABC::Addressee());
    enableButton(User1, false);
  }
  connect( this, SIGNAL( user1Clicked() ), SLOT( slotUser1() ) );
  connect( this, SIGNAL( user2Clicked() ), SLOT( slotUser2() ) );
  connect( this, SIGNAL( user3Clicked() ), SLOT( slotUser3() ) );

  resize(300,400);
}

VCardViewer::~VCardViewer()
{
}

void VCardViewer::slotUser1()
{
  KABC::Addressee contact = *itAddresseeList;

  // check whether a contact with the same email address exists already
  Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob( this );
  searchJob->setLimit( 1 );
  searchJob->setQuery( Akonadi::ContactSearchJob::Email, contact.preferredEmail(), Akonadi::ContactSearchJob::ExactMatch );
  searchJob->setProperty( "newContact", QVariant::fromValue( contact ) );

  connect( searchJob, SIGNAL( result( KJob* ) ), SLOT( slotDuplicatedContactChecked( KJob* ) ) );
}

void VCardViewer::slotUser2()
{
  // next vcard
  mContactViewer->setRawContact( *(++itAddresseeList) );
  if ( itAddresseeList == --(mAddresseeList.end()) )
    enableButton(User2, false);
  enableButton(User3, true);
}

void VCardViewer::slotUser3()
{
  // previous vcard
  mContactViewer->setRawContact( *(--itAddresseeList) );
  if ( itAddresseeList == mAddresseeList.begin() )
    enableButton(User3, false);
  enableButton(User2, true);
}

void VCardViewer::slotDuplicatedContactChecked( KJob *job )
{
  const Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );

  if ( job->error() ) {
    // TODO: show error message to user
    return;
  }

  const KABC::Addressee::List contacts = searchJob->contacts();

  if ( !contacts.isEmpty() ) { // contact is already part of the address book...
    const QString text = i18n( "The VCard's primary email address is already in "
                               "your address book; however, you may save the VCard "
                               "into a file and import it into the address book manually." );
    KMessageBox::information( this, text );
    return;
  }

  // ask user in which address book the new contact shall be stored
  const QStringList mimeTypes( KABC::Addressee::mimeType() );
  QPointer<Akonadi::CollectionDialog> dlg = new Akonadi::CollectionDialog( this );
  dlg->setMimeTypeFilter( mimeTypes );
  dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
  dlg->setCaption( i18n( "Select Address Book" ) );
  dlg->setDescription( i18n( "Select the address book the new contact shall be saved in:" ) );

  if ( dlg->exec() == QDialog::Accepted ) {
    const Akonadi::Collection addressBook = dlg->selectedCollection();
    if ( addressBook.isValid() ) {
      // create the new item
      Akonadi::Item item;
      item.setMimeType( KABC::Addressee::mimeType() );
      item.setPayload<KABC::Addressee>( searchJob->property( "newContact" ).value<KABC::Addressee>() );

      // save the new item in akonadi storage
      Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, addressBook );
      connect( job, SIGNAL( result( KJob* ) ), SLOT( slotVCardAdded( KJob* ) ) );
    }
  }

  delete dlg;
}

void VCardViewer::slotVCardAdded( KJob *job )
{
  if ( job->error() ) {
    // TODO: show error message to user
    return;
  }

  const QString text = i18n( "The VCard was added to your address book; "
                             "you can add more information to this "
                             "entry by opening the address book." );
  KMessageBox::information( this, text, QString(), "addedtokabc" );
}

#include "vcardviewer.moc"
