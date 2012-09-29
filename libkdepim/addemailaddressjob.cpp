/*
  Copyright 2010 Tobias Koenig <tokoe@kde.org>
  Copyright 2010 Nicolas Lécureuil <nicolas.lecureuil@free.fr>

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

#include <Akonadi/CollectionDialog>
#include <Akonadi/Contact/ContactSearchJob>
#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/Collection>
#include <Akonadi/Contact/ContactEditorDialog>

#include <KABC/Addressee>

#include <KLocale>
#include <KMessageBox>

#include <QPointer>

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
        const QString text =
          i18nc( "@info",
                 "A contact with the email address <email>%1</email> "
                 "is already in your address book.", mCompleteAddress );

        KMessageBox::information( mParentWidget, text, QString(), QLatin1String("alreadyInAddressBook") );
        q->setError( UserDefinedError );
        q->emitResult();
        return;
      }

      const QStringList mimeTypes( KABC::Addressee::mimeType() );

      Akonadi::CollectionFetchJob * const addressBookJob = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive );
      addressBookJob->fetchScope().setContentMimeTypes( mimeTypes );
      q->connect( addressBookJob, SIGNAL(result(KJob*)), SLOT(slotCollectionsFetched(KJob*)) );
    }

    void slotCollectionsFetched( KJob *job )
    {
      if ( job->error() ) {
        q->setError( job->error() );
        q->setErrorText( job->errorText() );
        q->emitResult();
        return;
      }

      const Akonadi::CollectionFetchJob *addressBookJob = qobject_cast<Akonadi::CollectionFetchJob*>( job );

      Akonadi::Collection::List canCreateItemCollections ;

      foreach( const Akonadi::Collection &collection, addressBookJob->collections() ) {
        if ( Akonadi::Collection::CanCreateItem & collection.rights() ) {
          canCreateItemCollections.append(collection);
        }
      }


      Akonadi::Collection addressBook;

      const int nbItemCollection(canCreateItemCollections.size());
      if ( nbItemCollection == 0 ) {
        KMessageBox::information (
          mParentWidget,
          i18nc( "@info",
                 "You must create an address book before adding a contact." ),
          i18nc( "@title:window", "No Address Book Available" ) );
        q->setError( UserDefinedError );
        q->emitResult();
        return;
      }
      else if ( nbItemCollection == 1 ) {
        addressBook = canCreateItemCollections[0];
      }
      else {
        // ask user in which address book the new contact shall be stored
        const QStringList mimeTypes( KABC::Addressee::mimeType() );
        QPointer<Akonadi::CollectionDialog> dlg = new Akonadi::CollectionDialog;
        dlg->setMimeTypeFilter( mimeTypes );
        dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
        dlg->setCaption( i18nc( "@title:window", "Select Address Book" ) );
        dlg->setDescription(
          i18nc( "@info",
                 "Select the address book where the contact will be saved:" ) );

        bool gotIt = true;
        if ( !dlg->exec() ) {
          q->setError( UserDefinedError );
          q->emitResult();
          gotIt = false;
        } else {
          addressBook = dlg->selectedCollection();
        }
        delete dlg;
        if ( !gotIt ) {
          return;
        }
      }

      if ( !addressBook.isValid() ) {
        q->setError( UserDefinedError );
        q->emitResult();
        return;
      }
      KABC::Addressee contact;
      contact.setNameFromString( mName );
      contact.insertEmail( mEmail, true );

      // create the new item
      Akonadi::Item item;
      item.setMimeType( KABC::Addressee::mimeType() );
      item.setPayload<KABC::Addressee>( contact );

      // save the new item in akonadi storage
      Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob( item, addressBook, q );
      q->connect( createJob, SIGNAL(result(KJob*)), SLOT(slotAddContactDone(KJob*)) );
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

      const QString text =
        i18nc( "@info",
               "<para>A contact for <email>%1</email> was successfully added "
               "to your address book.</para>"
               "<para>Do you want to edit this new contact now?</para>",
               mCompleteAddress );

      if ( KMessageBox::questionYesNo(
             mParentWidget,
             text,
             QString(),
             KStandardGuiItem::yes(),
             KStandardGuiItem::no(),
             QLatin1String( "addedtokabc" ) ) == KMessageBox::Yes ) {
        QPointer<Akonadi::ContactEditorDialog> dlg =
          new Akonadi::ContactEditorDialog( Akonadi::ContactEditorDialog::EditMode,
                                            mParentWidget );
        dlg->setContact( mItem );
        dlg->exec();
        delete dlg;
      }
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
  searchJob->setQuery( Akonadi::ContactSearchJob::Email, d->mEmail,
                       Akonadi::ContactSearchJob::ExactMatch );
  connect( searchJob, SIGNAL(result(KJob*)), SLOT(slotSearchDone(KJob*)) );
}

Akonadi::Item AddEmailAddressJob::contact() const
{
  return d->mItem;
}

#include "addemailaddressjob.moc"
