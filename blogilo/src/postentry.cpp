/***************************************************************************
 *   This file is part of the Bilbo Blogger.                               *
 *   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
 *   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QtGui>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <klineedit.h>
#include <KMessageBox>
#include "postentry.h"
#include "composer/bilboeditor.h"
#include "bilbomedia.h"
#include "backend.h"
#include "dbman.h"
#include "global.h"
#include "sendtoblogdialog.h"
#include <kio/job.h>
#include "settings.h"
#include "bilboblog.h"
#include "syncuploader.h"

#define MINUTE 60000

PostEntry::PostEntry( QWidget *parent )
        : QFrame( parent )
{
    kDebug();
    createUi();
    editPostWidget = new BilboEditor( this );
    editPostWidget->setMediaList( &mMediaList );
    this->layout()->addWidget( editPostWidget );
    mTimer = new QTimer(this);
    mTimer->start(Settings::autosaveInterval() * MINUTE);
    connect( mTimer, SIGNAL(timeout()), this, SLOT( saveTemporary() ) );
    progress = 0;
    mCurrentPostBlogId = -1;
    mNumOfFilesToBeUploaded = 0;
    isPostContentModified = false;
    connect( editPostWidget, SIGNAL(textChanged()), this, SLOT(slotPostModified()) );
//     connect( txtTitle, SIGNAL(textChanged(QString)), this, SLOT(slotPostModified()) );
    connect( editPostWidget, SIGNAL( sigShowStatusMessage( const QString&, bool ) ), 
            this, SIGNAL( showStatusMessage( const QString&, bool ) ) );
    connect( editPostWidget, SIGNAL( sigBusy( bool ) ), this, SIGNAL( sigBusy( bool ) ) );
}

void PostEntry::aboutToQuit()
{
    kDebug();
    saveTemporary(true);
}

void PostEntry::settingsChanged()
{
    kDebug();
    mTimer->setInterval(Settings::autosaveInterval() * MINUTE);
    if(Settings::autosaveInterval())
        mTimer->start();
    else
        mTimer->stop();
}

void PostEntry::createUi()
{
    this->resize( 626, 307 );
    gridLayout = new QGridLayout( this );

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSizeConstraint( QLayout::SetDefaultConstraint );

    labelTitle = new QLabel( this );
    labelTitle->setText( i18nc( "noun, the post title", "Title:" ) );
    horizontalLayout->addWidget( labelTitle );

    txtTitle = new KLineEdit( this );
    horizontalLayout->addWidget( txtTitle );
    labelTitle->setBuddy( txtTitle );
    connect( txtTitle, SIGNAL( textChanged( const QString& ) ), this,
             SLOT( sltTitleChanged( const QString& ) ) );

    gridLayout->addLayout( horizontalLayout, 0, 0, 1, 1 );

}

void PostEntry::sltTitleChanged( const QString& title )
{
    mCurrentPost.setTitle( title );
    this->editPostWidget->setCurrentTitle( title );
    Q_EMIT sigTitleChanged( title );
}

QString PostEntry::postTitle() const
{
    return mCurrentPost.title();
}

void PostEntry::setPostTitle( const QString & title )
{
    kDebug();
    this->txtTitle->setText( title );
    mCurrentPost.setTitle( title );
    this->editPostWidget->setCurrentTitle( title );
}

void PostEntry::setPostBody( const QString & content, const QString &additionalContent )
{
    kDebug();
    QString body;
    if(additionalContent.isEmpty()) {
        body = content;
    } else {
        body = content + "</p><!--split--><p>" + additionalContent;
        mCurrentPost.setAdditionalContent(QString());
    }
    if(body.isEmpty()){
        body = "<p></p>";//This is because of Bug #387578
    }
    mCurrentPost.setContent( body );
    this->editPostWidget->setHtmlContent( body );
    isPostContentModified = false;
    connect( editPostWidget, SIGNAL(textChanged()), this, SLOT(slotPostModified()) );
//     connect( txtTitle, SIGNAL(textChanged(QString)), this, SLOT(slotPostModified()) );
}

int PostEntry::currentPostBlogId()
{
    return mCurrentPostBlogId;
}

void PostEntry::setCurrentPostBlogId( int blog_id )
{
    kDebug();
    mCurrentPostBlogId = blog_id;
    if ( blog_id != -1 && DBMan::self()->blogList().contains( blog_id ) ) {
        setDefaultLayoutDirection( DBMan::self()->blogList().value( blog_id )->direction() );
    }
}

void PostEntry::setCurrentPostFromEditor()
{
    if( isPostContentModified ) {
        kDebug();
        const QString& str = this->editPostWidget->htmlContent();
        mCurrentPost.setContent( str );
        isPostContentModified = false;
        connect( editPostWidget, SIGNAL(textChanged()), this, SLOT(slotPostModified()) );
    }
}

BilboPost* PostEntry::currentPost()
{
    setCurrentPostFromEditor();
    return &mCurrentPost;
}

void PostEntry::setCurrentPost( const BilboPost &post )
{
    kDebug();
//     if(mCurrentPost)
//         delete mCurrentPost;
    mCurrentPost = post;
//     kDebug()<<"postId: "<<mCurrentPost.postId();
    this->setPostBody( mCurrentPost.content(), mCurrentPost.additionalContent() );
    this->setPostTitle( mCurrentPost.title() );
}

Qt::LayoutDirection PostEntry::defaultLayoutDirection()
{
    return this->txtTitle->layoutDirection();
}

void PostEntry::setDefaultLayoutDirection( Qt::LayoutDirection direction )
{
    kDebug();
    this->editPostWidget->setLayoutDirection( direction );
    this->txtTitle->setLayoutDirection( direction );
}

PostEntry::~PostEntry()
{
    kDebug();
//     delete mCurrentPost;
}

QMap< QString, BilboMedia * > & PostEntry::mediaList()
{
    return mMediaList;
}

bool PostEntry::uploadMediaFiles( Backend *backend )
{
    kDebug();
    bool localBackend = false;
    bool result = true;
    if( !backend ) {
        localBackend = true;
        backend = new Backend( mCurrentPostBlogId, this );
    }
    if( mMediaList.size()>0 ) {
        progress = new QProgressBar( this );
        this->layout()->addWidget( progress );
        progress->setRange( 0, 0 );
        QMap <QString, BilboMedia*>::iterator it = mMediaList.begin();
        QMap <QString, BilboMedia*>::iterator endIt = mMediaList.end();
        for ( ; it != endIt; ++it ) {
            if( !it.value()->isUploaded() ){
                BilboMedia *media = it.value();
                SyncUploader *uploader = new SyncUploader(this);
                if( !uploader->uploadMedia( backend, media ) ){
                    QString err = i18n( "Uploading the media file %1 failed.\n%3", media->name(), uploader->errorMessage());
                    emit postPublishingDone( true, err );
                    uploader->deleteLater();
                    result = false;
                    break;
                }
                uploader->deleteLater();
            }
        }
    }
    if ( editPostWidget->updateMediaPaths() ) {
        mCurrentPost.setContent( this->editPostWidget->htmlContent() );
    } else {
        kDebug() << "Updateing media pathes failed!";
    }
    if(localBackend)
        backend->deleteLater();
    return result;
}

void PostEntry::sltError( const QString & errMsg )
{
    kDebug();
    QString err = i18n( "An error occurred on the last transaction.\n%1", errMsg );
    emit postPublishingDone( true, err );
    deleteProgressBar();
    sender()->deleteLater();
}

void PostEntry::submitPost( int blogId, const BilboPost &postData )
{
    kDebug();
    setCurrentPostFromEditor();
    if ( mCurrentPost.content().isEmpty() || mCurrentPost.title().isEmpty() ) {
        if ( KMessageBox::warningContinueCancel( this,
            i18n( "Your post title or body is empty!\nAre you sure you want to submit this post?" )
            ) == KMessageBox::Cancel )
            return;
    }
    bool isNew = false;
    if(mCurrentPost.status() == BilboPost::New)
        isNew = true;
    SendToBlogDialog *dia = new SendToBlogDialog( isNew, mCurrentPost.isPrivate(), this);
    dia->setAttribute(Qt::WA_DeleteOnClose, false);
    if( dia->exec() == KDialog::Accepted ) {
        this->setCursor( Qt::BusyCursor );
        mCurrentPost.setProperties( postData );
        mCurrentPostBlogId = blogId;

        QString msgType;
        if(dia->isPrivate()) {
            msgType =  i18nc("Post status, e.g Draft or Published Post", "draft");
            mCurrentPost.setPrivate(true);
        } else {
            msgType =  i18nc("Post status, e.g Draft or Published Post", "post");
            mCurrentPost.setPrivate(false);
        }

        QString statusMsg;
        if(dia->isNew()) {
            statusMsg = i18n("Submiting new %1...", msgType);
            isNewPost = true;
        } else {
            statusMsg = i18n("Modifying %1...", msgType);
            isNewPost = false;
        }

        emit showStatusMessage(statusMsg, true);
        Backend *b = new Backend(mCurrentPostBlogId, this);
        connect( b, SIGNAL(sigError(const QString&)), this, SLOT(sltError(const QString&)) );
        if ( uploadMediaFiles(b) ) {
            kDebug()<<"Uploading";
            if( !progress ) {
                progress = new QProgressBar( this );
                this->layout()->addWidget( progress );
                progress->setRange( 0, 0 );
            }
            connect( b, SIGNAL( sigPostPublished( int, BilboPost* ) ), this, SLOT( sltPostPublished( int, BilboPost* ) ) );
            if(isNewPost)
                b->publishPost( mCurrentPost );
            else
                b->modifyPost( mCurrentPost );
        } else {
            deleteProgressBar();
        }
    }
}

void PostEntry::sltPostPublished( int blog_id, BilboPost *post )
{
    kDebug() << "BlogId: " << blog_id << "Post Id on server: " << post->postId();
    DBMan::self()->removeTempEntry(mCurrentPost);
    QString msg;
    setCurrentPost(*post);
    if ( mCurrentPost.isPrivate() ) {
        msg = i18n( "Draft with title \"%1\" saved successfully.", post->title() );
    } else if(mCurrentPost.status() == BilboPost::Modified){
        msg = i18n( "Post with title \"%1\" modified successfully.", post->title() );
    } else {
        msg = i18n( "Post with title \"%1\" published successfully.", post->title() );
    }
//     KMessageBox::information( this, msg, "Successful" );
    deleteProgressBar();
    this->unsetCursor();
    emit postPublishingDone( false, msg );
    sender()->deleteLater(); //FIXME Check if this command needed or NOT -Mehrdad
}

void PostEntry::deleteProgressBar()
{
    kDebug();
    if(progress){
        this->layout()->removeWidget( progress );
        progress->deleteLater();
    }
    progress = 0;
}

void PostEntry::saveLocally()
{
    kDebug();
    if( currentPost()->content().isEmpty() ) {
        if( KMessageBox::warningYesNo(this, i18n("The current post content is empty, \
are you sure you want to save an empty post?")) == KMessageBox::No )
            return;
    }
    mCurrentPost.setId( DBMan::self()->saveLocalEntry( *currentPost(), mCurrentPostBlogId ) );
    emit postSavedLocally();
    emit showStatusMessage(i18n( "Post saved locally." ), false);
    kDebug()<<"Locally saved";
}

void PostEntry::saveTemporary( bool force )
{
    if( isPostContentModified || ( !currentPost()->content().isEmpty() && force ) ) {
        mCurrentPost.setId( DBMan::self()->saveTempEntry( *currentPost(), mCurrentPostBlogId) );
        emit postSavedTemporary();
        kDebug()<<"Temporary saved";
    }
}

void PostEntry::slotPostModified()
{
    kDebug();
    disconnect( editPostWidget, SIGNAL(textChanged()), this, SLOT(slotPostModified()) );
//         disconnect( txtTitle, SIGNAL(textChanged(QString)), this, SLOT(slotPostModified()) );
    emit postModified();
    isPostContentModified = true;
}

#include "postentry.moc"
