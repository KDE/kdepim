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

Viewer::Viewer(QWidget *aParent,
                         KSharedConfigPtr config,
                         QWidget *mainWindow,
                         KActionCollection* actionCollection,
                         Qt::WindowFlags aFlags )
  : QWidget(aParent, aFlags ), d_ptr(new ViewerPrivate(this, config, mainWindow, actionCollection) )
{
  connect( d_ptr, SIGNAL( replaceMsgByUnencryptedVersion() ), SIGNAL( replaceMsgByUnencryptedVersion() ) );
  connect( d_ptr, SIGNAL( popupMenu(KMime::Message &, const KUrl &, const QPoint&) ), SIGNAL( popupMenu(KMime::Message &, const KUrl &, const QPoint&) ) );
  connect( d_ptr, SIGNAL( popupMenu(const Akonadi::Item &, const KUrl &, const QPoint&) ), SIGNAL( popupMenu(const Akonadi::Item &, const KUrl &, const QPoint&) ) );
  connect( d_ptr, SIGNAL( urlClicked( const Akonadi::Item &, const KUrl & ) ), SIGNAL( urlClicked( const Akonadi::Item &,  const KUrl& ) ) );
  connect( d_ptr, SIGNAL( noDrag() ), SIGNAL( noDrag() ) );
  connect( d_ptr, SIGNAL( requestConfigSync() ), SIGNAL( requestConfigSync() ) );
  connect( d_ptr, SIGNAL( showReader( KMime::Content* , bool , const QString&, const QString&, const QString & ) ),
           SIGNAL( showReader( KMime::Content*, bool, const QString&, const QString&, const QString & )) );

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


void Viewer::setMessageItem(const Akonadi::Item &item, UpdateMode updateMode)
{
  Q_D(Viewer);
  if ( item.loadedPayloadParts().contains( Akonadi::MessagePart::Body ) ) {
    d->setMessageItem( item, updateMode );
  } else {
    Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob( item, this );
    job->fetchScope().fetchFullPayload( true );
    connect( job, SIGNAL(result(KJob*)), d, SLOT(itemFetchResult(KJob*)) );
    d->displaySplashPage( i18n( "Loading message..." ) );
  }
}

void Viewer::displaySplashPage( const QString &info )
{
  Q_D(Viewer);
  d->displaySplashPage( info );
}


void Viewer::enableMessageDisplay()
{
  Q_D(Viewer);
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

void Viewer::slotScrollUp()
{
  Q_D(Viewer);
  QPoint point = d->mViewer->page()->mainFrame()->scrollPosition();
  point -= QPoint(0, 10);
  d->mViewer->page()->mainFrame()->setScrollPosition( point );
}

void Viewer::slotScrollDown()
{
  Q_D(Viewer);
  QPoint point = d->mViewer->page()->mainFrame()->scrollPosition();
  point += QPoint(0, 10);
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

bool Viewer::decryptMessage() const
{
  Q_D(const Viewer);
  return d->decryptMessage();
}

bool Viewer::showSignatureDetails() const
{
  Q_D(const Viewer);
  return d->showSignatureDetails();
}

void Viewer::setShowSignatureDetails( bool showDetails )
{
  Q_D(Viewer);
  d->setShowSignatureDetails( showDetails );
}

bool Viewer::showAttachmentQuicklist() const
{
  Q_D(const Viewer);
  return d->showAttachmentQuicklist();
}

void Viewer::setShowAttachmentQuicklist( bool showAttachmentQuicklist  )
{
  Q_D(Viewer);
  d->setShowAttachmentQuicklist( showAttachmentQuicklist );
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

void Viewer::setPrintFont( const QFont& font )
{
  Q_D(Viewer);
  d->mCSSHelper->setPrintFont( font );
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

const HeaderStyle * Viewer::headerStyle() const
{
  Q_D( const Viewer );
  return d->headerStyle();
}

void Viewer::setHeaderStyleAndStrategy( const HeaderStyle * style,
                                        const HeaderStrategy * strategy )
{
  Q_D( Viewer );
  d->setHeaderStyleAndStrategy( style, strategy );
}

MailWebView *Viewer::htmlPart() const
{
  Q_D( const Viewer );
  return d->htmlPart();
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

void Viewer::saveRelativePosition()
{
  Q_D( Viewer );
  d->saveRelativePosition();
}

KUrl Viewer::urlClicked() const
{
  Q_D( const Viewer );
  return d->mUrlClicked;
}

void Viewer::update( Viewer::UpdateMode updateMode )
{
  Q_D( Viewer );
  d->update( updateMode );
}

void Viewer::setMessagePart( KMime::Content* aMsgPart, bool aHTML,
                              const QString& aFileName, const QString& pname )
{
  Q_D( Viewer );
  d->setMessagePart( aMsgPart, aHTML, aFileName, pname );
}

void Viewer::slotShowMessageSource()
{
  Q_D( Viewer );
  d->slotShowMessageSource();
}

bool Viewer::noMDNsWhenEncrypted() const
{
  Q_D( const Viewer );
  return d->noMDNsWhenEncrypted();
}

void Viewer::readConfig()
{
  Q_D( Viewer );
  d->readConfig();
}

bool Viewer::disregardUmask() const
{
  Q_D( const Viewer );
  return d->disregardUmask();
}

void Viewer::setDisregardUmask( bool b)
{
  Q_D( Viewer );
  d->setDisregardUmask( b );
}

QAbstractItemModel* Viewer::messageTreeModel() const
{
  return d_func()->mMimePartModel;
}


}

#include "viewer.moc"


