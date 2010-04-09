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

#include "addcontactjob.h"

#include <akonadi/collectiondialog.h>
#include <akonadi/contact/contactsearchjob.h>
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>
#include <kabc/addressee.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QPointer>

using namespace KPIM;

class AddContactJob::Private
{
  public:
    Private( AddContactJob *qq, const KABC::Addressee &contact, QWidget *parentWidget )
      : q( qq ), mContact( contact ), mParentWidget( parentWidget )
    {
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

      if ( !contacts.isEmpty() ) { // contact is already part of the address book...
        const QString text = i18n( "The VCard's primary email address is already in "
                                   "your address book; however, you may save the VCard "
                                   "into a file and import it into the address book manually." );
        KMessageBox::information( mParentWidget, text );
        q->setError( UserDefinedError );
        q->emitResult();
        return;
      }


      // ask user in which address book the new contact shall be stored
      const QStringList mimeTypes( KABC::Addressee::mimeType() );
      QPointer<Akonadi::CollectionDialog> dlg = new Akonadi::CollectionDialog( mParentWidget );
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
          item.setPayload<KABC::Addressee>( mContact );

          // save the new item in akonadi storage
          Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, addressBook );
          q->connect( job, SIGNAL( result( KJob* ) ), SLOT( slotAddContactDone( KJob* ) ) );
        } else {
          q->setError( UserDefinedError );
          q->emitResult();
        }
      } else {
        q->setError( UserDefinedError );
        q->emitResult();
      }

      delete dlg;
    }

    void slotAddContactDone( KJob *job )
    {
      if ( job->error() ) {
        q->setError( job->error() );
        q->setErrorText( job->errorText() );
        q->emitResult();
        return;
      }

      const QString text = i18n( "The VCard was added to your address book; "
                                 "you can add more information to this "
                                 "entry by opening the address book." );
      KMessageBox::information( mParentWidget, text, QString(), "addedtokabc" );

      q->emitResult();
    }

    AddContactJob *q;
    KABC::Addressee mContact;
    QWidget *mParentWidget;
};

AddContactJob::AddContactJob( const KABC::Addressee &contact, QWidget *parentWidget, QObject *parent )
  : KJob( parent ), d( new Private( this, contact, parentWidget ) )
{
}

AddContactJob::~AddContactJob()
{
  delete d;
}

void AddContactJob::start()
{
  // first check whether a contact with the same email exists already
  Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob( this );
  searchJob->setLimit( 1 );
  searchJob->setQuery( Akonadi::ContactSearchJob::Email, d->mContact.preferredEmail(),
                       Akonadi::ContactSearchJob::ExactMatch );

  connect( searchJob, SIGNAL( result( KJob* ) ), SLOT( slotSearchDone( KJob* ) ) );
}

#include "addcontactjob.moc"
