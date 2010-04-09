/*
  Copyright 2010 Tobias Koenig <tokoe@kde.org>

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
*/

#include "addemailaddressjob.h"

#include <akonadi/collectiondialog.h>
#include <akonadi/contact/contactsearchjob.h>
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>
#include <kabc/addressee.h>
#include <klocale.h>
#include <kmessagebox.h>

using namespace KPIM;

class AddEmailAddressJob::Private
{
  public:
    Private( AddEmailAddressJob *qq, const QString &emailString, QWidget *parentWidget )
      : q( qq ), mCompleteAddress( emailString ), mParentWidget( parentWidget )
    {
      KABC::Addressee::parseEmailAddress( emailString, mName, mEmail );
    }

    void slotSearchDone( KJob *job )
    {
      if ( job->error() ) {
        q->setError( job->error() );
        q->setErrorText( job->errorText() );
        q->emitResult();
        return;
      }

      const Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );

      const KABC::Addressee::List contacts = searchJob->contacts();
      if ( !contacts.isEmpty() ) {
        const QString text = i18n( "<qt>The email address <b>%1</b> is already in your address book.</qt>",
                                   mCompleteAddress );

        KMessageBox::information( mParentWidget, text, QString(), "alreadyInAddressBook" );
        q->setError( UserDefinedError );
        q->emitResult();
        return;
      }

      KABC::Addressee contact;
      contact.setNameFromString( mName );
      contact.insertEmail( mEmail, true );

      // ask user in which address book the new contact shall be stored
      const QStringList mimeTypes( KABC::Addressee::mimeType() );
      Akonadi::CollectionDialog dlg;
      dlg.setMimeTypeFilter( mimeTypes );
      dlg.setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
      dlg.setCaption( i18n( "Select Address Book" ) );
      dlg.setDescription( i18n( "Select the address book the new contact shall be saved in:" ) );

      if ( !dlg.exec() ) {
        q->setError( UserDefinedError );
        q->emitResult();
        return;
      }

      const Akonadi::Collection addressBook = dlg.selectedCollection();
      if ( !addressBook.isValid() ) {
        q->setError( UserDefinedError );
        q->emitResult();
        return;
      }

      // create the new item
      Akonadi::Item item;
      item.setMimeType( KABC::Addressee::mimeType() );
      item.setPayload<KABC::Addressee>( contact );

      // save the new item in akonadi storage
      Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob( item, addressBook, q );
      q->connect( createJob, SIGNAL( result( KJob* ) ), SLOT( slotAddContactDone( KJob* ) ) );
    }

    void slotAddContactDone( KJob *job )
    {
      if ( job->error() ) {
        q->setError( job->error() );
        q->setErrorText( job->errorText() );
        q->emitResult();
        return;
      }

      const Akonadi::ItemCreateJob *createJob = qobject_cast<Akonadi::ItemCreateJob*>( job );
      mItem = createJob->item();

      const QString text = i18n( "<qt>The email address <b>%1</b> was added to your "
                                 "address book; you can add more information to this "
                                 "entry by opening the address book.</qt>", mCompleteAddress );
      KMessageBox::information( mParentWidget, text, QString(), "addedtokabc" );

      q->emitResult();
    }

    AddEmailAddressJob *q;
    QString mCompleteAddress;
    QString mEmail;
    QString mName;
    QWidget *mParentWidget;
    Akonadi::Item mItem;
};

AddEmailAddressJob::AddEmailAddressJob( const QString &email, QWidget *parentWidget, QObject *parent )
  : KJob( parent ), d( new Private( this, email, parentWidget ) )
{
}

AddEmailAddressJob::~AddEmailAddressJob()
{
  delete d;
}

void AddEmailAddressJob::start()
{
  // first check whether a contact with the same email exists already
  Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob( this );
  searchJob->setLimit( 1 );
  searchJob->setQuery( Akonadi::ContactSearchJob::Email, d->mEmail );
  connect( searchJob, SIGNAL( result( KJob* ) ), SLOT( slotSearchDone( KJob* ) ) );
}

Akonadi::Item AddEmailAddressJob::contact() const
{
  return d->mItem;
}

#include "addemailaddressjob.moc"
