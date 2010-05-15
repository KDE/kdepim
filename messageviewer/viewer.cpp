/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

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
#include "configurewidget.h"
#include "csshelper.h"
#include "globalsettings.h"
#include "mailwebview.h"
#include "findbar/findbar.h"
#include "mimetreemodel.h"

#include <akonadi/kmime/messageparts.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>

//KDE includes
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>

namespace MessageViewer {

Viewer::Viewer( QWidget *aParent, QWidget *mainWindow, KActionCollection *actionCollection,
                Qt::WindowFlags aFlags )
  : QWidget( aParent, aFlags ), d_ptr( new ViewerPrivate( this, mainWindow, actionCollection ) )
{
  connect( d_ptr, SIGNAL( replaceMsgByUnencryptedVersion() ),
          SIGNAL( replaceMsgByUnencryptedVersion() ) );
  connect( d_ptr, SIGNAL( popupMenu(KMime::Message &, const KUrl &, const QPoint&) ),
           SIGNAL( popupMenu(KMime::Message &, const KUrl &, const QPoint&) ) );
  connect( d_ptr, SIGNAL( popupMenu(const Akonadi::Item &, const KUrl &, const QPoint&) ),
           SIGNAL( popupMenu(const Akonadi::Item &, const KUrl &, const QPoint&) ) );
  connect( d_ptr, SIGNAL( urlClicked( const Akonadi::Item &, const KUrl & ) ),
           SIGNAL( urlClicked( const Akonadi::Item &,  const KUrl& ) ) );
  connect( d_ptr, SIGNAL( requestConfigSync() ), SIGNAL( requestConfigSync() ) );
  connect( d_ptr, SIGNAL( showReader( KMime::Content* , bool , const QString& ) ),
           SIGNAL( showReader( KMime::Content*, bool, const QString& )) );
  connect( d_ptr, SIGNAL( showMessage(KMime::Message::Ptr, const QString& )), this, SIGNAL( showMessage(KMime::Message::Ptr, const QString&)) );
  connect( d_ptr, SIGNAL( showStatusBarMessage( const QString & ) ),
           this, SIGNAL( showStatusBarMessage( const QString & ) ) );

  setMessage( KMime::Message::Ptr(), Delayed );
}

Viewer::~Viewer()
{
  //the d_ptr is automatically deleted
}


void Viewer::setMessage(KMime::Message::Ptr message, UpdateMode updateMode )
{
  Q_D(Viewer);
  d->setMessage( message, updateMode );
}


void Viewer::setMessageItem( const Akonadi::Item &item, UpdateMode updateMode )
{
  Q_D(Viewer);
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

void Viewer::printMessage( KMime::Message::Ptr message )
{
   Q_D(Viewer);
   d->printMessage( message );
}

void Viewer::print()
{
  Q_D(Viewer);
  if ( !message() )
    return;
  d->mViewer->print( false );
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

void Viewer::slotSaveMessage()
{
  Q_D( Viewer );
  d->slotSaveMessage();
}

void Viewer::slotScrollUp( int pixels )
{
  Q_D(Viewer);
  QPoint point = d->mViewer->page()->mainFrame()->scrollPosition();
  point -= QPoint( 0, qAbs( pixels ) );
  d->mViewer->page()->mainFrame()->setScrollPosition( point );
}

void Viewer::slotScrollDown( int pixels )
{
  Q_D(Viewer);
  QPoint point = d->mViewer->page()->mainFrame()->scrollPosition();
  point += QPoint( 0, qAbs( pixels ) );
  d->mViewer->page()->mainFrame()->setScrollPosition( point );
}

bool Viewer::atBottom() const
{
  Q_D(const Viewer);
  int pos = d->mViewer->page()->mainFrame()->scrollBarValue( Qt::Vertical );
  int max = d->mViewer->page()->mainFrame()->scrollBarMaximum( Qt::Vertical );
  return pos == max;
}

void Viewer::slotJumpDown()
{
  Q_D(Viewer);
  int height = d->mViewer->page()->viewportSize().height();
  int current = d->mViewer->page()->mainFrame()->scrollBarValue( Qt::Vertical );
  d->mViewer->page()->mainFrame()->setScrollBarValue( Qt::Vertical, current + height );
}

void Viewer::slotScrollPrior()
{
  Q_D(Viewer);
  int height = d->mViewer->page()->viewportSize().height();
  int current = d->mViewer->page()->mainFrame()->scrollBarValue( Qt::Vertical );
  d->mViewer->page()->mainFrame()->setScrollBarValue( Qt::Vertical, current - ( 0.8 * height ) );
}

void Viewer::slotScrollNext()
{
  Q_D(Viewer);
  int height = d->mViewer->page()->viewportSize().height();
  int current = d->mViewer->page()->mainFrame()->scrollBarValue( Qt::Vertical );
  d->mViewer->page()->mainFrame()->setScrollBarValue( Qt::Vertical, current + ( 0.8 * height ) );
}

QString Viewer::selectedText()
{
  Q_D(Viewer);
  QString temp = d->mViewer->selectedText();
  return temp;
}

void Viewer::setHtmlOverride( bool override )
{
  Q_D(Viewer);
  d->setHtmlOverride( override );
}

bool Viewer::htmlOverride() const
{
  Q_D(const Viewer);;
  return d->htmlOverride();
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
  connect( w, SIGNAL( settingsChanged() ), d, SLOT( slotSettingsChanged() ) );
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
  if (e->type() == QEvent::PaletteChange)
  {
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

KAction *Viewer::selectAllAction()
{
  Q_D( Viewer );
  return d->mSelectAllAction;
}

const HeaderStrategy * Viewer::headerStrategy() const
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
                                        const HeaderStrategy * strategy )
{
  Q_D( Viewer );
  d->setHeaderStyleAndStrategy( style, strategy );
}

KAction *Viewer::copyURLAction()
{
  Q_D( Viewer );
  return d->mCopyURLAction;
}

KAction *Viewer::copyAction()
{
  Q_D( Viewer );
  return d->mCopyAction;
}

KAction *Viewer::urlOpenAction()
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

void Viewer::slotUrlClicked()
{
  Q_D( Viewer );
  d->slotUrlClicked();
}

KUrl Viewer::urlClicked() const
{
  Q_D( const Viewer );
  return d->mClickedUrl;
}

void Viewer::update( Viewer::UpdateMode updateMode )
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
  return d_func()->mMimePartModel;
}

Akonadi::ItemFetchJob* Viewer::createFetchJob( const Akonadi::Item &item )
{
  Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob( item );
  job->fetchScope().fetchAllAttributes();
  job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
  job->fetchScope().fetchFullPayload( true );
  return job;
}

void Viewer::deleteMessage()
{
  Q_D( Viewer );
  emit deleteMessage( d->messageItem() );
}



}

#include "viewer.moc"


