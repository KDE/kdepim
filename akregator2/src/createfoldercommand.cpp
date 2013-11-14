/*
    This file is part of Akregator2.

    Copyright (C) 2008-2009 Frank Osterfeld <osterfeld@kde.org>

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

#include "createfoldercommand.h"
#include "command_p.h"
#include "feedlistview.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionCreateJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/Item>
#include <Akonadi/Session>

#include <krss/feedcollection.h>
#include <KRss/Item>

#include <KLocalizedString>
#include <KRandom>

#include <QPointer>

using namespace Akonadi;
using namespace Akregator2;
using namespace KRss;

class CreateFolderCommand::Private
{
    CreateFolderCommand* const q;
public:
    explicit Private( const Collection& c, const QString& title, CreateFolderCommand* qq );

    void collectionCreated( KJob* );

    QPointer<FeedListView> feedListView;
    Collection parentCollection;
    QString title;
    Session* session;
};

CreateFolderCommand::Private::Private( const Collection& parent, const QString& t, CreateFolderCommand* qq )
    : q( qq )
    , parentCollection( parent )
    , title( t )
    , session( 0 )

{
    q->setUserVisible( false );
    q->setShowErrorDialog( true );
}

void CreateFolderCommand::Private::collectionCreated( KJob* j )
{
    if ( j->error() ) {
        q->setError( Command::SomeError );
        q->setErrorText( j->errorText() );
        q->emitResult();
        return;
    }
    CollectionCreateJob* job = qobject_cast<CollectionCreateJob*>( j );
    Q_ASSERT( job );
    if ( feedListView )
        feedListView->scrollToCollection( job->collection() );
    q->emitResult();
}

CreateFolderCommand::CreateFolderCommand( const Collection& pc, const QString& t, QObject* parent ) : Command( parent ), d( new Private( pc, t, this ) )
{
}

CreateFolderCommand::~CreateFolderCommand()
{
    delete d;
}

void CreateFolderCommand::setSession( Session* s ) {
    d->session = s;
}

void CreateFolderCommand::setFeedListView( FeedListView* view )
{
    d->feedListView = view;
}

void CreateFolderCommand::doStart()
{
    Q_ASSERT( d->session );

    d->parentCollection = FeedCollection::findFolder( d->parentCollection );
    if ( !d->parentCollection.isValid() ) {
        setError( SomeError );
        setErrorText( tr("Invalid parent collection. Cannot create folder") );
        emitResult();
        return;
    }

    const QString title = !d->title.isEmpty() ? d->title : i18n("New Folder");

    FeedCollection c;
    c.setParentCollection( d->parentCollection );
    c.setName( title + KRandom::randomString( 8 ) );
    c.setTitle( title );
    c.setContentMimeTypes( QStringList() << Collection::mimeType() << KRss::Item::mimeType() );
    c.attribute<Akonadi::EntityDisplayAttribute>( Akonadi::Collection::AddIfMissing )->setDisplayName( title );
    c.setIsFolder( true );
    CollectionCreateJob * const job = new CollectionCreateJob( c, d->session );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(collectionCreated(KJob*)) );
    job->start();
}

#include "moc_createfoldercommand.cpp"
