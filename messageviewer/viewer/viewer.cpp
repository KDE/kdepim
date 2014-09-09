/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>
  Copyright (c) 2013 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

// define this to copy all html that is written to the readerwindow to
// filehtmlwriter.out in the current working directory
//#define KMAIL_READER_HTML_DEBUG 1


#include "viewer.h"
#include "viewer_p.h"
#include "widgets/configurewidget.h"
#include "csshelper.h"
#include "settings/globalsettings.h"
#include "viewer/mailwebview.h"
#include "viewer/mimetreemodel.h"
#include "viewer/mimeparttreeview.h"
#include "adblock/adblockmanager.h"

#include <Akonadi/KMime/MessageParts>
#include <AkonadiCore/itemfetchjob.h>
#include <AkonadiCore/itemfetchscope.h>
#include <KUrl>

#include <MailTransport/mailtransport/errorattribute.h>

#include <KLocalizedString>

#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <KGlobalSettings>

namespace MessageViewer {

Viewer::Viewer( QWidget *aParent, QWidget *mainWindow, KActionCollection *actionCollection,
                Qt::WindowFlags aFlags )
    : QWidget( aParent, aFlags ), d_ptr( new ViewerPrivate( this, mainWindow, actionCollection ) )
{
    initialize();
}


Viewer::~Viewer()
{
    //the d_ptr is automatically deleted
}

void Viewer::initialize()
{
    connect( d_ptr, SIGNAL(replaceMsgByUnencryptedVersion()),
             SIGNAL(replaceMsgByUnencryptedVersion()) );
    connect( d_ptr, SIGNAL(popupMenu(Akonadi::Item,KUrl,KUrl,QPoint)),
             SIGNAL(popupMenu(Akonadi::Item,KUrl,KUrl,QPoint)) );
    connect( d_ptr, SIGNAL(urlClicked(Akonadi::Item,KUrl)),
             SIGNAL(urlClicked(Akonadi::Item,KUrl)) );
    connect(d_ptr, &ViewerPrivate::requestConfigSync, this, &Viewer::requestConfigSync);
    connect(d_ptr, &ViewerPrivate::makeResourceOnline, this, &Viewer::makeResourceOnline);
    connect( d_ptr, SIGNAL(showReader(KMime::Content*,bool,QString)),
             SIGNAL(showReader(KMime::Content*,bool,QString)) );
    connect(d_ptr, &ViewerPrivate::showMessage, this, &Viewer::showMessage);
    connect( d_ptr, SIGNAL(showStatusBarMessage(QString)),
             this, SIGNAL(showStatusBarMessage(QString)) );
    connect( d_ptr, SIGNAL(itemRemoved()),
             this, SIGNAL(itemRemoved()) );
    connect(d_ptr, &ViewerPrivate::changeDisplayMail, this, &Viewer::slotChangeDisplayMail);
    connect(d_ptr, &ViewerPrivate::moveMessageToTrash, this, &Viewer::moveMessageToTrash);

    connect(KGlobalSettings::self(), &KGlobalSettings::kdisplayFontChanged, d_ptr, &ViewerPrivate::slotGeneralFontChanged);

    setMessage( KMime::Message::Ptr(), Delayed );
}

void Viewer::setMessage(KMime::Message::Ptr message, UpdateMode updateMode )
{
    Q_D(Viewer);
    if ( message == d->message() )
        return;
    d->setMessage( message, updateMode );
}


void Viewer::setMessageItem( const Akonadi::Item &item, UpdateMode updateMode )
{
    Q_D(Viewer);
    if ( d->messageItem() == item )
        return;
    if ( !item.isValid() || item.loadedPayloadParts().contains( Akonadi::MessagePart::Body ) ) {
        d->setMessageItem( item, updateMode );
    } else {
        Akonadi::ItemFetchJob* job = createFetchJob( item );
        connect( job, SIGNAL(result(KJob*)), d, SLOT(itemFetchResult(KJob*)) );
        d->displaySplashPage( i18n( "Loading message..." ) );
    }
}

QString Viewer::messagePath() const
{
    Q_D( const Viewer );
    return d->mMessagePath;
}

void Viewer::setMessagePath( const QString& path )
{
    Q_D( Viewer );
    d->mMessagePath = path;
}

void Viewer::displaySplashPage( const QString &info )
{
    Q_D( Viewer );
    d->displaySplashPage( info );
}

void Viewer::enableMessageDisplay()
{
    Q_D( Viewer );
    d->enableMessageDisplay();
}

void Viewer::printMessage( const Akonadi::Item &msg )
{
    Q_D( Viewer );
    d->printMessage( msg );
}

void Viewer::printPreviewMessage( const Akonadi::Item &message )
{
    Q_D( Viewer );
    d->printPreviewMessage( message );
}

void Viewer::printPreview()
{
    Q_D(Viewer);
    d->slotPrintPreview();
}

void Viewer::print()
{
    Q_D(Viewer);
    d->slotPrintMsg();
}

void Viewer::resizeEvent( QResizeEvent * )
{
    Q_D(Viewer);
    if( !d->mResizeTimer.isActive() )
    {
        //
        // Combine all resize operations that are requested as long a
        // the timer runs.
        //
        d->mResizeTimer.start( 100 );
    }
}

void Viewer::closeEvent( QCloseEvent *e )
{
    Q_D(Viewer);
    QWidget::closeEvent( e );
    d->writeConfig();
}

void Viewer::slotAttachmentSaveAs()
{
    Q_D( Viewer );
    d->slotAttachmentSaveAs();
}

void Viewer::slotAttachmentSaveAll()
{
    Q_D( Viewer );
    d->slotAttachmentSaveAll();
}

void Viewer::slotSaveMessage()
{
    Q_D( Viewer );
    d->slotSaveMessage();
}

void Viewer::slotScrollUp( int pixels )
{
    Q_D(Viewer);
    d->mViewer->scrollUp( qAbs( pixels ) );
}

void Viewer::slotScrollDown( int pixels )
{
    Q_D(Viewer);
    d->mViewer->scrollDown( qAbs( pixels ) );
}

bool Viewer::atBottom() const
{
    Q_D(const Viewer);
    return d->mViewer->isScrolledToBottom();
}

void Viewer::slotJumpDown()
{
    Q_D(Viewer);
    d->mViewer->scrollPageDown( 100 );
}

void Viewer::slotScrollPrior()
{
    Q_D(Viewer);
    d->mViewer->scrollPageUp( 80 );
}

void Viewer::slotScrollNext()
{
    Q_D(Viewer);
    d->mViewer->scrollPageDown( 80 );
}

QString Viewer::selectedText() const
{
    Q_D(const Viewer);
    QString temp = d->mViewer->selectedText();
    return temp;
}

Viewer::DisplayFormatMessage Viewer::displayFormatMessageOverwrite() const
{
    Q_D(const Viewer);
    return d->displayFormatMessageOverwrite();
}

void Viewer::setDisplayFormatMessageOverwrite(Viewer::DisplayFormatMessage format)
{
    Q_D(Viewer);
    d->setDisplayFormatMessageOverwrite(format);
}

void Viewer::setHtmlLoadExtOverride( bool override )
{
    Q_D(Viewer);
    d->setHtmlLoadExtOverride( override );
}

void Viewer::setAppName( const QString& appName )
{
    Q_D(Viewer);
    d->mAppName = appName;
}

bool Viewer::htmlLoadExtOverride() const
{
    Q_D(const Viewer);
    return d->htmlLoadExtOverride();
}

bool Viewer::htmlMail() const
{
    Q_D(const Viewer);
    return d->htmlMail();
}

bool Viewer::htmlLoadExternal() const
{
    Q_D(const Viewer);
    return d->htmlLoadExternal();
}

bool Viewer::isFixedFont() const
{
    Q_D(const Viewer);
    return d->mUseFixedFont;

}
void Viewer::setUseFixedFont( bool useFixedFont )
{
    Q_D(Viewer);
    d->setUseFixedFont( useFixedFont );
}

QWidget* Viewer::mainWindow()
{
    Q_D(Viewer);
    return d->mMainWindow;
}

void Viewer::setDecryptMessageOverwrite( bool overwrite )
{
    Q_D(Viewer);
    d->setDecryptMessageOverwrite( overwrite );
}

QWidget* Viewer::configWidget()
{
    Q_D(Viewer);
    ConfigureWidget *w = new ConfigureWidget();
    connect( w, SIGNAL(settingsChanged()), d, SLOT(slotSettingsChanged()) );
    return w;
}

KMime::Message::Ptr Viewer::message() const
{
    Q_D(const Viewer);
    return d->mMessage;
}

Akonadi::Item Viewer::messageItem() const
{
    Q_D(const Viewer);
    return d->mMessageItem;
}

bool Viewer::event(QEvent *e)
{
    Q_D(Viewer);
    if (e->type() == QEvent::PaletteChange) {
        delete d->mCSSHelper;
        d->mCSSHelper = new CSSHelper( d->mViewer );
        d->update( Viewer::Force ); // Force update
        return true;
    }
    return QWidget::event(e);
}

void Viewer::slotFind()
{
    Q_D(Viewer);
    d->slotFind();
}

void Viewer::slotTranslate()
{
    Q_D(Viewer);
    d->slotTranslate();
}


const AttachmentStrategy * Viewer::attachmentStrategy() const
{
    Q_D(const Viewer);
    return d->attachmentStrategy();
}

void Viewer::setAttachmentStrategy( const AttachmentStrategy * strategy )
{
    Q_D(Viewer);
    d->setAttachmentStrategy( strategy );
}

QString Viewer::overrideEncoding() const
{
    Q_D( const Viewer );
    return d->overrideEncoding();
}

void Viewer::setOverrideEncoding( const QString &encoding )
{
    Q_D( Viewer );
    d->setOverrideEncoding( encoding );

}

CSSHelper* Viewer::cssHelper() const
{
    Q_D( const Viewer );
    return d->cssHelper();
}


KToggleAction *Viewer::toggleFixFontAction()
{
    Q_D( Viewer );
    return d->mToggleFixFontAction;
}

KToggleAction *Viewer::toggleMimePartTreeAction()
{
    Q_D( Viewer );
    return d->mToggleMimePartTreeAction;
}

QAction *Viewer::selectAllAction()
{
    Q_D( Viewer );
    return d->mSelectAllAction;
}

HeaderStrategy * Viewer::headerStrategy() const
{
    Q_D( const Viewer );
    return d->headerStrategy();
}

HeaderStyle * Viewer::headerStyle() const
{
    Q_D( const Viewer );
    return d->headerStyle();
}

void Viewer::setHeaderStyleAndStrategy( HeaderStyle * style,
                                        HeaderStrategy * strategy )
{
    Q_D( Viewer );
    d->setHeaderStyleAndStrategy( style, strategy );
}

void Viewer::setExternalWindow( bool b )
{
    Q_D( Viewer );
    d->setExternalWindow( b );
}

QAction *Viewer::viewSourceAction()
{
    Q_D( Viewer );
    return d->mViewSourceAction;
}

QAction *Viewer::copyURLAction()
{
    Q_D( Viewer );
    return d->mCopyURLAction;
}

QAction *Viewer::copyAction()
{
    Q_D( Viewer );
    return d->mCopyAction;
}

QAction *Viewer::speakTextAction()
{
    Q_D( Viewer );
    return d->mSpeakTextAction;
}

QAction *Viewer::copyImageLocation()
{
    Q_D( Viewer );
    return d->mCopyImageLocation;
}

QAction *Viewer::translateAction()
{
    Q_D( Viewer );
    return d->mTranslateAction;
}

QAction *Viewer::saveAsAction()
{
    Q_D( Viewer );
    return d->mSaveMessageAction;
}

QAction *Viewer::urlOpenAction()
{
    Q_D( Viewer );
    return d->mUrlOpenAction;
}

void Viewer::setPrinting(bool enable)
{
    Q_D( Viewer );
    d->setPrinting( enable );
}

void Viewer::writeConfig( bool force )
{
    Q_D( Viewer );
    d->writeConfig( force );
}

KUrl Viewer::urlClicked() const
{
    Q_D( const Viewer );
    return d->mClickedUrl;
}

KUrl Viewer::imageUrlClicked() const
{
    Q_D( const Viewer );
    return d->mImageUrl;
}

void Viewer::update( MessageViewer::Viewer::UpdateMode updateMode )
{
    Q_D( Viewer );
    d->update( updateMode );
}

void Viewer::setMessagePart( KMime::Content* aMsgPart )
{
    Q_D( Viewer );
    d->setMessagePart( aMsgPart );
}

void Viewer::slotShowMessageSource()
{
    Q_D( Viewer );
    d->slotShowMessageSource();
}

void Viewer::readConfig()
{
    Q_D( Viewer );
    d->readConfig();
}

QAbstractItemModel* Viewer::messageTreeModel() const
{
#ifndef QT_NO_TREEVIEW
    return d_func()->mMimePartTree->mimePartModel();
#else
    return 0;
#endif
}

Akonadi::ItemFetchJob* Viewer::createFetchJob( const Akonadi::Item &item )
{
    Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob( item );
    job->fetchScope().fetchAllAttributes();
    job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
    job->fetchScope().fetchFullPayload( true );
    job->fetchScope().fetchAttribute<MailTransport::ErrorAttribute>();
    return job;
}

void Viewer::setScrollBarPolicy( Qt::Orientation orientation, Qt::ScrollBarPolicy policy )
{
    Q_D( Viewer );
    d->mViewer->setScrollBarPolicy( orientation, policy );
}

void Viewer::addMessageLoadedHandler( AbstractMessageLoadedHandler *handler )
{
    Q_D( Viewer );

    if ( !handler )
        return;

    d->mMessageLoadedHandlers.insert( handler );
}

void Viewer::removeMessageLoadedHandler( AbstractMessageLoadedHandler *handler )
{
    Q_D( Viewer );

    d->mMessageLoadedHandlers.remove( handler );
}

void Viewer::deleteMessage()
{
    Q_D( Viewer );
    emit deleteMessage( d->messageItem() );
}

void Viewer::selectAll()
{
    Q_D( Viewer );
    d->selectAll();
}

void Viewer::clearSelection()
{
    Q_D( Viewer );
    d->clearSelection();
}


void Viewer::copySelectionToClipboard()
{
    Q_D( Viewer );
    d->slotCopySelectedText();
}

void Viewer::setZoomFactor( qreal zoomFactor )
{
    Q_D( Viewer );
    d->setZoomFactor(zoomFactor);
}

void Viewer::slotZoomReset()
{
    Q_D( Viewer );
    d->slotZoomReset();
}

void Viewer::slotZoomIn()
{
    Q_D( Viewer );
    d->slotZoomIn();
}

void Viewer::slotZoomOut()
{
    Q_D( Viewer );
    d->slotZoomOut();
}

void Viewer::setZoomTextOnly( bool textOnly )
{
    Q_D( Viewer );
    d->setZoomTextOnly( textOnly );
}

bool Viewer::zoomTextOnly() const
{
    Q_D(const Viewer);
    return d->mZoomTextOnly;
}

QAction *Viewer::findInMessageAction()
{
    Q_D( Viewer );
    return d->mFindInMessageAction;
}

void Viewer::slotChangeDisplayMail(Viewer::DisplayFormatMessage mode,bool loadExternal)
{
    setHtmlLoadExtOverride(loadExternal);
    setDisplayFormatMessageOverwrite(mode);
    update(Viewer::Force);
}

QAction *Viewer::saveMessageDisplayFormatAction()
{
    Q_D( Viewer );
    return d->mSaveMessageDisplayFormat;
}

QAction *Viewer::resetMessageDisplayFormatAction()
{
    Q_D( Viewer );
    return d->mResetMessageDisplayFormat;
}

void Viewer::saveMainFrameScreenshotInFile(const QString &filename)
{
    Q_D( Viewer );
    return d->saveMainFrameScreenshotInFile(filename);
}

QAction *Viewer::blockImage()
{
    Q_D( Viewer );
    return d->mBlockImage;
}

bool Viewer::adblockEnabled() const
{
#ifndef KDEPIM_NO_WEBKIT
    return MessageViewer::AdBlockManager::self()->isEnabled();
#else
    return false;
#endif
}

QAction *Viewer::openBlockableItems()
{
    Q_D( Viewer );
    return d->mBlockableItems;
}

bool Viewer::isAShortUrl(const KUrl &url) const
{
    Q_D( const Viewer );
    return d->isAShortUrl(url);
}

QAction *Viewer::expandShortUrlAction()
{
    Q_D( Viewer );
    return d->mExpandUrlAction;
}

QAction *Viewer::createTodoAction()
{
    Q_D( Viewer );
    return d->mCreateTodoAction;
}

QAction *Viewer::createEventAction()
{
    Q_D( Viewer );
    return d->mCreateEventAction;
}

void Viewer::showOpenAttachmentFolderWidget(const KUrl &url)
{
    Q_D( Viewer );
    d->showOpenAttachmentFolderWidget(url);
}

}



