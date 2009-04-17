/*
    This file is part of Akregator.

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

#include "addfeeddialog.h"
#include "feed.h"
#include "feedpropertiesdialog.h"
#include "folder.h"
#include "subscriptionlistview.h"

#include <KInputDialog>
#include <KLocalizedString>
#include <KUrl>

#include <QPointer>
#include <QTimer>
#include <QClipboard>

#include <cassert>

using namespace Akregator;

class CreateFeedCommand::Private
{
    CreateFeedCommand* const q;
public:
    explicit Private( CreateFeedCommand* qq );

    void doCreate();

    QPointer<Folder> m_rootFolder;
    QPointer<SubscriptionListView> m_subscriptionListView;
    QString m_url;
    QPointer<Folder> m_parentFolder;
    QPointer<TreeNode> m_after;
    bool m_autoexec;
};

CreateFeedCommand::Private::Private( CreateFeedCommand* qq )
  : q( qq ),
    m_rootFolder( 0 ),
    m_subscriptionListView( 0 ),
    m_parentFolder( 0 ),
    m_after( 0 ),
    m_autoexec( false )
{

}

void CreateFeedCommand::Private::doCreate()
{
    assert( m_rootFolder );
    assert( m_subscriptionListView );

    QPointer<AddFeedDialog> afd = new AddFeedDialog( q->parentWidget(), "add_feed" );

    QString url = m_url;

    if( url.isEmpty() )
    {
        const QClipboard* const clipboard = QApplication::clipboard();
        assert( clipboard );
        const QString clipboardText = clipboard->text();
        // Check for the hostname, since the isValid method is not strict enough
        if( !KUrl( clipboardText ).isEmpty() )
            url = clipboardText;
    }

    afd->setUrl( KUrl::fromPercentEncoding( url.toLatin1() ) );

    QPointer<QObject> thisPointer( q );

    if ( m_autoexec )
        afd->accept();
    else
        afd->exec();

    if ( !thisPointer ) // "this" might have been deleted while exec()!
        return;

    Feed* const feed = afd->feed();
    delete afd;

    if ( !feed )
    {
        q->done();
        return;
    }

    QPointer<FeedPropertiesDialog> dlg = new FeedPropertiesDialog( q->parentWidget(), "edit_feed" );
    dlg->setFeed( feed );
    dlg->selectFeedName();

    if ( !m_autoexec && ( dlg->exec() != QDialog::Accepted || !thisPointer ) )
    {
        delete feed;
    }
    else
    {
        m_parentFolder = m_parentFolder ? m_parentFolder : m_rootFolder;
        m_parentFolder->insertChild( feed, m_after );
        m_subscriptionListView->ensureNodeVisible( feed );
    }

    delete dlg;
    q->done();
}

CreateFeedCommand::CreateFeedCommand( QObject* parent ) : Command( parent ), d( new Private( this ) )
{

}

CreateFeedCommand::~CreateFeedCommand()
{
    delete d;
}

void CreateFeedCommand::setSubscriptionListView( SubscriptionListView* view )
{
    d->m_subscriptionListView = view;
}

void CreateFeedCommand::setRootFolder( Folder* rootFolder )
{
    d->m_rootFolder = rootFolder;
}

void CreateFeedCommand::setUrl( const QString& url )
{
    d->m_url = url;
}

void CreateFeedCommand::setPosition( Folder* parent, TreeNode* after )
{
    d->m_parentFolder = parent;
    d->m_after = after;
}

void CreateFeedCommand::setAutoExecute( bool autoexec )
{
    d->m_autoexec = autoexec;
}

void CreateFeedCommand::doStart()
{
    QTimer::singleShot( 0, this, SLOT( doCreate() ) );
}

void CreateFeedCommand::doAbort()
{

}

#include "createfeedcommand.moc"
