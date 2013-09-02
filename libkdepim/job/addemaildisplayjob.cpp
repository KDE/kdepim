/*
  Copyright 2013 Laurent Montel <montel@kde.org>

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

#include "addemaildisplayjob.h"
#include "misc/broadcaststatus.h"
#include "widgets/selectedcollectiondialog.h"


#include <Akonadi/CollectionDialog>
#include <Akonadi/Contact/ContactSearchJob>
#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/Collection>
#include <Akonadi/Contact/ContactEditorDialog>
#include <Akonadi/AgentTypeDialog>
#include <Akonadi/AgentType>
#include <Akonadi/AgentFilterProxyModel>
#include <Akonadi/AgentInstanceCreateJob>

#include <KABC/Addressee>
#include <KABC/ContactGroup>

#include <KLocale>
#include <KMessageBox>

#include <QPointer>
#include <QTextDocument>

using namespace KPIM;

class AddEmailDiplayJob::Private
{
public:
    Private( AddEmailDiplayJob *qq, const QString &emailString, QWidget *parentWidget )
        : q( qq ),
          mShowAsHTML(false),
          mRemoteContent(false),
          mCompleteAddress( emailString ),
          mParentWidget( parentWidget )
    {
        KABC::Addressee::parseEmailAddress( emailString, mName, mEmail );
    }

    void slotResourceCreationDone( KJob* job )
    {
        if ( job->error() ) {
            q->setError( job->error() );
            q->setErrorText( job->errorText() );
            q->emitResult();
            return;
        }
        createContact();
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

        const Akonadi::Item::List items = searchJob->items();
        if ( items.isEmpty() ) {
            createContact();
        } else {
            Akonadi::Item item = items.at(0);
            KABC::Addressee contact = searchJob->contacts()[0];
            //TODO
            //TODO if multiple ?

        }
    }

    void createContact()
    {
        const QStringList mimeTypes( KABC::Addressee::mimeType() );

        Akonadi::CollectionFetchJob * const addressBookJob =
                new Akonadi::CollectionFetchJob( Akonadi::Collection::root(),
                                                 Akonadi::CollectionFetchJob::Recursive );

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

        const Akonadi::CollectionFetchJob *addressBookJob =
                qobject_cast<Akonadi::CollectionFetchJob*>( job );

        Akonadi::Collection::List canCreateItemCollections ;

        foreach ( const Akonadi::Collection &collection, addressBookJob->collections() ) {
            if ( Akonadi::Collection::CanCreateItem & collection.rights() ) {
                canCreateItemCollections.append(collection);
            }
        }

        Akonadi::Collection addressBook;

        const int nbItemCollection( canCreateItemCollections.size() );
        if ( nbItemCollection == 0 ) {
            if(KMessageBox::questionYesNo(
                        mParentWidget,
                        i18nc( "@info",
                               "You must create an address book before adding a contact. Do you want to create an address book?" ),
                        i18nc( "@title:window", "No Address Book Available" ) ) == KMessageBox::Yes) {
                Akonadi::AgentTypeDialog dlg( mParentWidget );
                dlg.setCaption( i18n("Add Address Book") );
                dlg.agentFilterProxyModel()->addMimeTypeFilter(KABC::Addressee::mimeType());
                dlg.agentFilterProxyModel()->addMimeTypeFilter(KABC::ContactGroup::mimeType());
                dlg.agentFilterProxyModel()->addCapabilityFilter( QLatin1String( "Resource" ) );

                if ( dlg.exec() ) {
                    const Akonadi::AgentType agentType = dlg.agentType();

                    if ( agentType.isValid() ) {
                        Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( agentType, q );
                        q->connect( job, SIGNAL(result(KJob*)), SLOT(slotResourceCreationDone(KJob*)) );
                        job->configure( mParentWidget );
                        job->start();
                        return;
                    } else { //if agent is not valid => return error and finish job
                        q->setError( UserDefinedError );
                        q->emitResult();
                        return;
                    }
                } else { //Canceled create agent => return error and finish job
                    q->setError( UserDefinedError );
                    q->emitResult();
                    return;
                }
            } else {
                q->setError( UserDefinedError );
                q->emitResult();
                return;
            }
        } else if ( nbItemCollection == 1 ) {
            addressBook = canCreateItemCollections[0];
        } else {
            // ask user in which address book the new contact shall be stored
            QPointer<SelectedCollectionDialog> dlg = new SelectedCollectionDialog( mParentWidget );

            bool gotIt = true;
            if ( dlg->exec() ) {
                addressBook = dlg->selectedCollection();
            } else {
                q->setError( UserDefinedError );
                q->emitResult();
                gotIt = false;
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
        Akonadi::Item item = createJob->item();
        //TODO add info.

        q->emitResult();
    }

    void slotContactEditorError(const QString &error)
    {
        KMessageBox::error(mParentWidget, i18n("Contact cannot be stored: %1", error), i18n("Failed to store contact"));
    }

    void contactStored( const Akonadi::Item & )
    {
        KPIM::BroadcastStatus::instance()->setStatusMsg( i18n( "Contact created successfully" ) );
    }


    AddEmailDiplayJob *q;
    bool mShowAsHTML;
    bool mRemoteContent;
    QString mCompleteAddress;
    QString mEmail;
    QString mName;
    QWidget *mParentWidget;
};

AddEmailDiplayJob::AddEmailDiplayJob( const QString &email, QWidget *parentWidget, QObject *parent )
    : KJob( parent ), d( new Private( this, email, parentWidget ) )
{
}

AddEmailDiplayJob::~AddEmailDiplayJob()
{
    delete d;
}

void AddEmailDiplayJob::showAsHTML(bool html)
{
    d->mShowAsHTML = html;
}

void AddEmailDiplayJob::remoteContent(bool b)
{
    d->mRemoteContent = b;
}


void AddEmailDiplayJob::start()
{
    // first check whether a contact with the same email exists already
    Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob( this );
    searchJob->setLimit( 1 );
    searchJob->setQuery( Akonadi::ContactSearchJob::Email, d->mEmail,
                         Akonadi::ContactSearchJob::ExactMatch );
    connect( searchJob, SIGNAL(result(KJob*)), SLOT(slotSearchDone(KJob*)) );
}

#include "addemaildisplayjob.moc"
