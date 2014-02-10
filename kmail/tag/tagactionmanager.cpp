/* Copyright 2010 Thomas McGuire <mcguire@kde.org>
   Copyright 2011-2012-2013 Laurent Montel <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "tagactionmanager.h"

#include "messageactions.h"

#include "mailcommon/tag/addtagdialog.h"

#include <KAction>
#include <KActionCollection>
#include <KToggleAction>
#include <KXMLGUIClient>
#include <KActionMenu>
#include <KMenu>
#include <KLocalizedString>
#include <KJob>

#include <QSignalMapper>
#include <QPointer>

#include <Akonadi/TagFetchJob>
#include <Akonadi/TagAttribute>

using namespace KMail;

static int s_numberMaxTag = 10;

TagActionManager::TagActionManager( QObject *parent, KActionCollection *actionCollection,
                                    MessageActions *messageActions, KXMLGUIClient *guiClient )
    : QObject( parent ),
      mActionCollection( actionCollection ),
      mMessageActions( messageActions ),
      mMessageTagToggleMapper( 0 ),
      mGUIClient( guiClient ),
      mSeparatorMoreAction( 0 ),
      mSeparatorNewTagAction( 0 ),
      mMoreAction( 0 ),
      mNewTagAction( 0 ),
      mTagFetchInProgress( false )
{
    mMessageActions->messageStatusMenu()->menu()->addSeparator();

    //TODO monitor tags
    //mTags should be replaced by a generic TagCache with a monitor built in.

//     Nepomuk2::ResourceWatcher* watcher = new Nepomuk2::ResourceWatcher(this);
//     watcher->addType(Soprano::Vocabulary::NAO::Tag());
//     connect(watcher, SIGNAL(resourceCreated(Nepomuk2::Resource,QList<QUrl>)), this, SLOT(resourceCreated(Nepomuk2::Resource,QList<QUrl>)));
//     connect(watcher, SIGNAL(resourceRemoved(QUrl,QList<QUrl>)),this, SLOT(resourceRemoved(QUrl,QList<QUrl>)));
//     connect(watcher, SIGNAL(propertyChanged(Nepomuk2::Resource,Nepomuk2::Types::Property,QVariantList,QVariantList)),this,SLOT(propertyChanged(Nepomuk2::Resource)));
//     watcher->start();
}

TagActionManager::~TagActionManager()
{
}

void TagActionManager::clearActions()
{
    //Remove the tag actions from the toolbar
    if ( !mToolbarActions.isEmpty() ) {
        if ( mGUIClient->factory() ) {
            mGUIClient->unplugActionList( QLatin1String("toolbar_messagetag_actions") );
        }
        mToolbarActions.clear();
    }

    //Remove the tag actions from the status menu and the action collection,
    //then delete them.
    foreach( KAction *action, mTagActions ) {
        mMessageActions->messageStatusMenu()->removeAction( action );

        // This removes and deletes the action at the same time
        mActionCollection->removeAction( action );
    }

    if ( mSeparatorMoreAction ) {
        mMessageActions->messageStatusMenu()->removeAction( mSeparatorMoreAction );
    }

    if ( mSeparatorNewTagAction ) {
        mMessageActions->messageStatusMenu()->removeAction( mSeparatorNewTagAction );
    }

    if ( mNewTagAction ) {
        mMessageActions->messageStatusMenu()->removeAction( mNewTagAction );
    }

    if ( mMoreAction ) {
        mMessageActions->messageStatusMenu()->removeAction( mMoreAction );
    }


    mTagActions.clear();
    delete mMessageTagToggleMapper;
    mMessageTagToggleMapper = 0;
}

void TagActionManager::createTagAction( const MailCommon::Tag::Ptr &tag, bool addToMenu )
{
    QString cleanName( i18n("Message Tag %1", tag->tagName ) );
    cleanName.replace(QLatin1Char('&'), QLatin1String("&&"));
    KToggleAction * const tagAction = new KToggleAction( KIcon( tag->iconName ),
                                                         cleanName, this );
    tagAction->setShortcut( tag->shortcut );
    tagAction->setIconText( tag->name() );
    tagAction->setChecked( tag->id() == mNewTagId );

    mActionCollection->addAction( tag->name(), tagAction );
    connect( tagAction, SIGNAL(triggered(bool)),
             mMessageTagToggleMapper, SLOT(map()) );

    // The shortcut configuration is done in the config dialog.
    // The shortcut set in the shortcut dialog would not be saved back to
    // the tag descriptions correctly.
    tagAction->setShortcutConfigurable( false );
    mMessageTagToggleMapper->setMapping( tagAction, QString::number(tag->tag().id()) );

    mTagActions.insert( tag->id(), tagAction );
    if ( addToMenu )
        mMessageActions->messageStatusMenu()->menu()->addAction( tagAction );

    if ( tag->inToolbar ) {
        mToolbarActions.append( tagAction );
    }
}

void TagActionManager::createActions()
{
    if ( mTagFetchInProgress )
      return
    clearActions();

    if ( mTags.isEmpty() ) {
      mTagFetchInProgress = true;
      //TODO set type filter
      Akonadi::TagFetchJob *fetchJob = new Akonadi::TagFetchJob(this);
      fetchJob->fetchAttribute<Akonadi::TagAttribute>();
      connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(finishedTagListing(KJob*)));
    } else {
        createTagActions();
    }
}

void TagActionManager::onSignalMapped(const QString& tag)
{
  emit tagActionTriggered(Akonadi::Tag(tag.toLongLong()));
}

void TagActionManager::createTagActions()
{
  kDebug();
    //Use a mapper to understand which tag button is triggered
    mMessageTagToggleMapper = new QSignalMapper( this );
    connect( mMessageTagToggleMapper, SIGNAL(mapped(QString)),
             this, SIGNAL(onSignalMapped(QString)) );

    // Create a action for each tag and plug it into various places
    int i = 0;
    bool needToAddMoreAction = false;
    const int numberOfTag(mTags.count());
    foreach( const MailCommon::Tag::Ptr &tag, mTags ) {
        if(tag->tagStatus)
            continue;
        if ( i< s_numberMaxTag )
            createTagAction( tag,true );
        else
        {
            if ( tag->inToolbar || !tag->shortcut.isEmpty() ) {
                createTagAction( tag, false );
            }

            if ( i == s_numberMaxTag && i < numberOfTag )
            {
                needToAddMoreAction = true;
            }
        }
        ++i;
    }


    if(!mSeparatorNewTagAction) {
        mSeparatorNewTagAction = new QAction( this );
        mSeparatorNewTagAction->setSeparator( true );
    }
    mMessageActions->messageStatusMenu()->menu()->addAction( mSeparatorNewTagAction );

    if (!mNewTagAction) {
        mNewTagAction = new KAction( i18n( "Add new tag..." ), this );
        connect( mNewTagAction, SIGNAL(triggered(bool)),
                 this, SLOT(newTagActionClicked()) );
    }
    mMessageActions->messageStatusMenu()->menu()->addAction( mNewTagAction );

    if (needToAddMoreAction) {
        if(!mSeparatorMoreAction) {
            mSeparatorMoreAction = new QAction( this );
            mSeparatorMoreAction->setSeparator( true );
        }
        mMessageActions->messageStatusMenu()->menu()->addAction( mSeparatorMoreAction );

        if (!mMoreAction) {
            mMoreAction = new KAction( i18n( "More..." ), this );
            connect( mMoreAction, SIGNAL(triggered(bool)),
                     this, SIGNAL(tagMoreActionClicked()) );
        }
        mMessageActions->messageStatusMenu()->menu()->addAction( mMoreAction );
    }

    if ( !mToolbarActions.isEmpty() && mGUIClient->factory() ) {
        mGUIClient->plugActionList( QLatin1String("toolbar_messagetag_actions"), mToolbarActions );
    }
}

void TagActionManager::finishedTagListing(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
    }
    Akonadi::TagFetchJob *fetchJob = static_cast<Akonadi::TagFetchJob*>(job);
    foreach (const Akonadi::Tag &result, fetchJob->tags()) {
        mTags.append( MailCommon::Tag::fromAkonadi( result ) );
    }
    mTagFetchInProgress = false;
    qSort( mTags.begin(), mTags.end(), MailCommon::Tag::compare );
    createTagActions();
}

void TagActionManager::updateActionStates( int numberOfSelectedMessages,
                                           const Akonadi::Item &selectedItem )
{
    mNewTagId = -1;
    QMap<qint64,KToggleAction*>::const_iterator it = mTagActions.constBegin();
    QMap<qint64,KToggleAction*>::const_iterator end = mTagActions.constEnd();
    if ( numberOfSelectedMessages >= 1 )
    {
        Q_ASSERT( selectedItem.isValid() );
        for ( ; it != end; ++it ) {
            //FIXME Not very performant tag label retrieval
            QString label(QLatin1String("not found"));
            foreach (const MailCommon::Tag::Ptr &tag, mTags) {
                if (tag->id() == it.key()) {
                    label = tag->name();
                    break;
                }
            }

            it.value()->setEnabled( true );
            if (numberOfSelectedMessages == 1) {
                const bool hasTag = selectedItem.hasTag(Akonadi::Tag(it.key()));
                it.value()->setChecked( hasTag );
                it.value()->setText( i18n("Message Tag %1", label ) );
            } else {
                it.value()->setChecked( false );
                it.value()->setText( i18n("Toggle Message Tag %1", label ) );
            }
        }
    }
    else {
        for ( ; it != end; ++it ) {
            it.value()->setEnabled( false );
        }
    }
}

// void TagActionManager::tagsChanged()
// {
//     mTags.clear(); // re-read the tags
//     createActions();
// }

//TODO onTagCreated(const Akonadi::Tag &)
// void TagActionManager::resourceCreated(const Nepomuk2::Resource& res,const QList<QUrl>&)
// {
//     const QList<QUrl> checked = checkedTags();
//
//     clearActions();
//     mTags.append( MailCommon::Tag::fromNepomuk( res ) );
//     qSort( mTags.begin(), mTags.end(), MailCommon::Tag::compare );
//     createTagActions();
//
//     checkTags( checked );
// }

//TODO onTagRemoved(const Akonadi::Tag &)
// void TagActionManager::resourceRemoved(const QUrl& url,const QList<QUrl>&)
// {
//     foreach( const MailCommon::Tag::Ptr &tag, mTags ) {
//         if(tag->nepomukResourceUri == url) {
//             mTags.removeAll(tag);
//             break;
//         }
//     }
//
//     const QList<QUrl> checked = checkedTags();
//
//     clearActions();
//     qSort( mTags.begin(), mTags.end(), MailCommon::Tag::compare );
//     createTagActions();
//
//     checkTags( checked );
// }

//TODO onTagChanged(const Akonadi::Tag &)
// void TagActionManager::propertyChanged(const Nepomuk2::Resource& res)
// {
//     foreach( const MailCommon::Tag::Ptr &tag, mTags ) {
// //         if(tag->nepomukResourceUri == res.uri()) {
// //             mTags.removeAll(tag);
// //             break;
// //         }
//     }
//     mTags.append( MailCommon::Tag::fromNepomuk( res ) );
//
//     QList<QUrl> checked = checkedTags();
//
//     clearActions();
//     qSort( mTags.begin(), mTags.end(), MailCommon::Tag::compare );
//     createTagActions();
//
//     checkTags( checked );
// }

void TagActionManager::newTagActionClicked()
{
    QPointer<MailCommon::AddTagDialog> dialog = new MailCommon::AddTagDialog(QList<KActionCollection*>() << mActionCollection, 0);
    dialog->setTags(mTags);
    if ( dialog->exec() == QDialog::Accepted ) {
        mNewTagId = dialog->tag().id();
        // Assign tag to all selected items right away
        emit tagActionTriggered( dialog->tag() );
    }
    delete dialog;
}

// void TagActionManager::checkTags(const QList< QUrl >& tags)
// {
//     foreach( const QUrl &url, tags ) {
//         const QString str = url.toString();
//         if ( mTagActions.contains( str ) ) {
//             mTagActions[str]->setChecked( true );
//         }
//     }
// }
//
// QList< QUrl > TagActionManager::checkedTags() const
// {
//     QMap<qint64,KToggleAction*>::const_iterator it = mTagActions.constBegin();
//     QMap<qint64,KToggleAction*>::const_iterator end = mTagActions.constEnd();
//     QList<QUrl> checked;
//     for ( ; it != end; ++it ) {
//         if ( it.value()->isChecked() ) {
//             checked << it.key();
//         }
//     }
//
//     return checked;
// }

