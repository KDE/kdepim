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

#include "createfeedcommand.h"
#include "editfeedcommand.h"

#include "addfeeddialog.h"
#include "editfeedcommand.h"
#include "command_p.h"

#include <Akonadi/CachePolicy>
#include <Akonadi/Collection>
#include <Akonadi/CollectionCreateJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/Item>

#include <krss/feedcollection.h>
#include <KRss/Item>

#include <KDebug>
#include <KInputDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRandom>
#include <KUrl>

#include <QPointer>
#include <QTimer>
#include <QClipboard>

using namespace Akregator2;
using namespace KRss;

class CreateFeedCommand::Private
{
    CreateFeedCommand* const q;
public:
    explicit Private( Akonadi::Session* session, CreateFeedCommand* qq );

    void doCreate();
    void creationDone( KJob* );
    void modificationDone();

    Akonadi::Session* m_session;
    QString m_url;
    bool m_autoexec;
    Akonadi::Collection m_parentCollection;
};

CreateFeedCommand::Private::Private( Akonadi::Session* session, CreateFeedCommand* qq )
  : q( qq ),
    m_session( session ),
    m_autoexec( false )
{
    q->setUserVisible( false );
    q->setShowErrorDialog( true );
}

void CreateFeedCommand::Private::doCreate()
{
    QString url = m_url;

    if ( !m_autoexec ) {
        QPointer<AddFeedDialog> afd = new AddFeedDialog( q->parentWidget() );

        if( url.isEmpty() ) {
            const QClipboard* const clipboard = QApplication::clipboard();
            Q_ASSERT( clipboard );
            const QString clipboardText = clipboard->text();
            // Check for the hostname, since the isValid method is not strict enough
            if( !KUrl( clipboardText ).isEmpty() )
                url = clipboardText;
        }

        afd->setUrl( KUrl::fromPercentEncoding( url.toLatin1() ) );

        EmitResultGuard guard( q );

        if ( afd->exec() != QDialog::Accepted || !guard.exists() ) {
            delete afd;
            guard.emitResult();
            return;
        }

        url = afd->url();

        delete afd;
    }

    if ( url.isEmpty() ) {
        q->emitResult();
        return;
    }

    Akonadi::CachePolicy policy;
    policy.setInheritFromParent( false );
    policy.setSyncOnDemand( false );
    policy.setLocalParts( QStringList() << KRss::Item::HeadersPart << KRss::Item::ContentPart << Akonadi::Item::FullPayload );

    KRss::FeedCollection feed;
    feed.setRemoteId( url );
    feed.setXmlUrl( url );
    feed.setContentMimeTypes( QStringList( QLatin1String("application/rss+xml") ) );
    feed.setCachePolicy( policy );
    feed.attribute<Akonadi::EntityDisplayAttribute>( Akonadi::Collection::AddIfMissing )->setIconName( QLatin1String("application-rss+xml") );
    feed.setParentCollection( m_parentCollection );
    feed.setName( url + KRandom::randomString( 8 ) );
    feed.setTitle( url );
    Akonadi::CollectionCreateJob* job = new Akonadi::CollectionCreateJob( feed );
    q->connect( job, SIGNAL(finished(KJob*)), q, SLOT(creationDone(KJob*)) );
    job->start();
}

void CreateFeedCommand::Private::creationDone( KJob* job )
{
    if ( job->error() )
        q->setErrorAndEmitResult( i18n("Could not add feed: %1", job->errorString()) );
    else {
        Akonadi::CollectionCreateJob* createJob = qobject_cast<Akonadi::CollectionCreateJob*>( job );
        Q_ASSERT( createJob );
        EditFeedCommand* cmd = new EditFeedCommand( q );
        connect( cmd, SIGNAL(finished()), q, SLOT(modificationDone()) );
        cmd->setParentWidget( q->parentWidget() );
        cmd->setCollection( createJob->collection() );
        cmd->setSession( m_session );
        cmd->start();
    }
}

void CreateFeedCommand::Private::modificationDone()
{
    EditFeedCommand* cmd = qobject_cast<EditFeedCommand*>( q->sender() );
    Q_ASSERT( cmd );
    if ( cmd->error() )
        q->setErrorAndEmitResult( i18n("Could not edit feed: %1", cmd->errorString()) );
    else
        q->emitResult();
}

CreateFeedCommand::CreateFeedCommand( Akonadi::Session* session, QObject* parent ) : Command( parent ), d( new Private( session, this ) )
{

}

CreateFeedCommand::~CreateFeedCommand()
{
    delete d;
}

void CreateFeedCommand::setUrl( const QString& url )
{
    d->m_url = url;
}

void CreateFeedCommand::setAutoExecute( bool autoexec )
{
    d->m_autoexec = autoexec;
}

void CreateFeedCommand::setParentCollection( const Akonadi::Collection& collection )
{
    d->m_parentCollection = collection;
}

void CreateFeedCommand::doStart()
{
    d->doCreate();
}

