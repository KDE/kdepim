/*
    This file is part of Akregator2.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "editfeedcommand.h"
#include "command_p.h"
#include "feedpropertiesdialog.h"

#include <krss/feedcollection.h>

#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/Session>

#include <KLocalizedString>
#include <KInputDialog>
#include <KRandom>
#include <KMessageBox>

#include <QPointer>
#include <QTimer>

#include <cassert>

using namespace Akonadi;
using namespace Akregator2;
using namespace KRss;

class EditFeedCommand::Private
{
    EditFeedCommand* const q;
public:
    explicit Private( EditFeedCommand* qq );

    void collectionFetched( KJob* j ) {

        EmitResultGuard guard( q );

        if ( j->error() ) {
            guard.setErrorAndEmitResult( j->errorText(), Command::SomeError );
            return;
        }

        CollectionFetchJob* job = qobject_cast<CollectionFetchJob*>( j );
        Q_ASSERT( job );
        collection = job->collections().first();
        FeedCollection fc( collection );
        if ( fc.isFolder() ) {
            bool ok = false;
            const QString newName = KInputDialog::getText( i18n("Rename Folder"), i18n("Rename folder:"), fc.title(), &ok, q->parentWidget() );
            if ( ok ) {
                fc.setName( newName + KRandom::randomString( 8 ) );
                fc.setTitle( newName );
                CollectionModifyJob* job = new CollectionModifyJob( fc, session );
                connect( job, SIGNAL(finished(KJob*)), q, SLOT(collectionModified(KJob*)) );
                job->start();
            }
        } else {
            QPointer<FeedPropertiesDialog> dlg( new FeedPropertiesDialog( q->parentWidget() ) );
            dlg->setFeedTitle( fc.title() );
            dlg->setUrl( fc.xmlUrl() );
            dlg->setCustomFetchInterval( fc.fetchInterval() >= 0 );
            dlg->setFetchInterval( fc.fetchInterval() );

            if ( dlg->exec() != QDialog::Accepted ) {
                delete dlg;
                guard.emitCanceled();
                return;
            }
            fc.setName( dlg->feedTitle() + KRandom::randomString( 8 ) );
            fc.setTitle( dlg->feedTitle() );
            fc.setXmlUrl( dlg->url() );
            fc.setFetchInterval( dlg->hasCustomFetchInterval() ? dlg->fetchInterval() : -1 );
            delete dlg;
            CollectionModifyJob* job = new CollectionModifyJob( fc, session );
            connect( job, SIGNAL(finished(KJob*)), q, SLOT(collectionModified(KJob*)) );
            job->start();
        }
    }

    void collectionModified( KJob* j ) {
        if ( j->error() )
            q->setErrorAndEmitResult( j->errorText() );
        else
            q->emitResult();
    }

    Akonadi::Collection collection;
    Akonadi::Session* session;
};

EditFeedCommand::Private::Private( EditFeedCommand* qq )
    : q( qq ), session( 0 )
{
    q->setUserVisible( false );
    q->setShowErrorDialog( true );
}

EditFeedCommand::EditFeedCommand( QObject* parent )
    : Command( parent ), d( new Private( this ) )
{
}

EditFeedCommand::~EditFeedCommand()
{
    delete d;
}

void EditFeedCommand::setCollection( const Collection& c )
{
    d->collection = c;
}

Collection EditFeedCommand::collection() const
{
    return d->collection;
}

void EditFeedCommand::setSession( Session* s )
{
    d->session = s;
}

Session* EditFeedCommand::session() const
{
    return d->session;
}

void EditFeedCommand::doStart()
{
    Q_ASSERT( d->session );
    CollectionFetchJob* job = new CollectionFetchJob( d->collection, CollectionFetchJob::Base, d->session );
    connect( job, SIGNAL(result(KJob*)), this, SLOT(collectionFetched(KJob*)) );
    job->start();
}
