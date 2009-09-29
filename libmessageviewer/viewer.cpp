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

//KDE includes
#include <KHTMLPart>
#include <KHTMLView>

namespace Message {

Viewer::Viewer(QWidget *aParent,
                         KSharedConfigPtr config,
                         QWidget *mainWindow,
                         KActionCollection* actionCollection,
                         Qt::WindowFlags aFlags )
  : QWidget(aParent, aFlags ), d_ptr(new ViewerPrivate(this, config, mainWindow, actionCollection) )
{
  connect( d_ptr, SIGNAL( replaceMsgByUnencryptedVersion() ), SIGNAL( replaceMsgByUnencryptedVersion() ) );
  connect( d_ptr, SIGNAL( popupMenu(KMime::Message &, const KUrl &, const QPoint&) ), SIGNAL( popupMenu(KMime::Message &, const KUrl &, const QPoint&) ) );
  connect( d_ptr, SIGNAL( urlClicked(const KUrl&, int ) ), SIGNAL( urlClicked(const KUrl&, int ) ) );
  connect( d_ptr, SIGNAL( noDrag() ), SIGNAL( noDrag() ) );
  setMessage( 0, Delayed );
}

Viewer::~Viewer()
{
  //the d_ptr is automatically deleted
}


void Viewer::setMessage(KMime::Message* message, UpdateMode updateMode, Ownership ownership)
{
  Q_D(Viewer);
  d->setMessage( message, updateMode, ownership);
}


void Viewer::setMessageItem(const Akonadi::Item &item, UpdateMode updateMode)
{
  Q_D(Viewer);
  d->setMessageItem( item, updateMode );
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

void Viewer::printMessage( KMime::Message* message )
{
   Q_D(Viewer);
   d->printMessage( message );
}

void Viewer::print()
{
  Q_D(Viewer);
  if ( !message() )
    return;
  d->mViewer->view()->print();
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

void Viewer::slotScrollUp()
{
  Q_D(Viewer);
  d->mViewer->view()->scrollBy( 0, -10 );
}

void Viewer::slotScrollDown()
{
  Q_D(Viewer);
  d->mViewer->view()->scrollBy( 0, 10 );
}

bool Viewer::atBottom() const
{
  Q_D(const Viewer);
  KHTMLView *view = d->mViewer->view();
  return view->contentsY() + view->visibleHeight() >= view->contentsHeight();
}

void Viewer::slotJumpDown()
{
  Q_D(Viewer);
  KHTMLView *view = d->mViewer->view();
  view->scrollBy( 0, view->visibleHeight() );
}

void Viewer::slotScrollPrior()
{
  Q_D(Viewer);
  KHTMLView *view = d->mViewer->view();
  view->scrollBy( 0, -(int)(d->mViewer->widget()->height() * 0.8 ) );
}

void Viewer::slotScrollNext()
{
  Q_D(Viewer);
  KHTMLView *view = d->mViewer->view();
  view->scrollBy( 0, (int)(d->mViewer->widget()->height() * 0.8 ) );
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

KConfigSkeleton *Viewer::configObject()
{
  return GlobalSettings::self();
}

KMime::Message* Viewer::message() const
{
  Q_D(const Viewer);
  return d->mMessage;
}

void Viewer::setPrintFont( const QFont& font )
{
  Q_D(Viewer);
  d->mCSSHelper->setPrintFont( font );
}

void Viewer::styleChange( QStyle& oldStyle )
{
  Q_D(Viewer);
  d->setStyleDependantFrameWidth();
  QWidget::styleChange( oldStyle );
}

bool Viewer::event(QEvent *e)
{
  Q_D(Viewer);
  if (e->type() == QEvent::PaletteChange)
  {
    delete d->mCSSHelper;
    d->mCSSHelper = new CSSHelper( d->mViewer->view() );
/*FIXME(Andras) port it
    if (message())
      message()->readConfig();
*/
    d->update( Viewer::Force ); // Force update
    return true;
  }
  return QWidget::event(e);
}

void Viewer::slotFind()
{
  Q_D(Viewer);
  d->mViewer->findText();
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

Message::CSSHelper* Viewer::cssHelper() const
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

const HeaderStyle * Viewer::headerStyle() const {
  Q_D( const Viewer );

  return d->headerStyle();
}

}

#include "viewer.moc"


