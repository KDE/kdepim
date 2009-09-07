/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>

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
#include "kmreaderwin.h"
#include "mailviewer_p.h"
#include "configurewidget.h"
#include "csshelper.h"
#include "globalsettings.h"

//KDE includes
#include <KHTMLPart>
#include <KHTMLView>

/*
#include <kpimutils/kfileio.h>
#include "kmmsgpartdlg.h"
#include "mailsourceviewer.h"
#include <QTextDocument>
#include <QByteArray>
#include <QImageReader>
#include <QCloseEvent>
#include <QEvent>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QScrollArea>
#include <QSignalMapper>
#include <QDesktopWidget>
#include <QModelIndex>

#include "kcursorsaver.h"
#include "vcardviewer.h"
#include "objecttreeparser.h"
#include "partmetadata.h"
#include "attachmentstrategy.h"
#include "headerstrategy.h"
#include "headerstyle.h"
#include "khtmlparthtmlwriter.h"
#include "htmlstatusbar.h"
#include "csshelper.h"
#include "urlhandlermanager.h"
#include "util.h"
#include "nodehelper.h"
#include "mimetreemodel.h"
#include "global.h"
#include "configurewidget.h"
#include "interfaces/bodypart.h"
#include "editorwatcher.h"

#include <kicon.h>
#include "libkdepim/broadcaststatus.h"
#include "libkdepim/attachmentpropertiesdialog.h"

#include <kmime/kmime_mdn.h>
#ifdef KMAIL_READER_HTML_DEBUG
#include "filehtmlwriter.h"
#include "teehtmlwriter.h"
#endif

//KMime headers
#include <kmime/kmime_message.h>
#include <kmime/kmime_headers.h>

//Akonadi includes
#include <akonadi/item.h>
#include <akonadi/itemmodifyjob.h>

#include "kleo/specialjob.h"
#include "kleo/cryptobackend.h"
#include "kleo/cryptobackendfactory.h"

// KABC includes
#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>

// khtml headers
#include <khtml_part.h>
#include <khtmlview.h> // So that we can get rid of the frames
#include <dom/html_element.h>
#include <dom/html_block.h>
#include <dom/html_document.h>
#include <dom/dom_string.h>

#include <kde_file.h>
#include <kactionmenu.h>
// for the click on attachment stuff (dnaber):
#include <kcharsets.h>
#include <kmenu.h>
#include <kstandarddirs.h>  // Sven's : for access and getpid
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetypetrader.h>
#include <kglobalsettings.h>
#include <krun.h>
#include <ktemporaryfile.h>
#include <kdialog.h>
#include <kaction.h>
#include <kfontaction.h>
#include <kiconloader.h>
#include <kcodecs.h>
#include <kascii.h>
#include <kselectaction.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kconfiggroup.h>
#include <kactioncollection.h>
#include <KColorScheme>
#include <KApplication>
#include <kio/netaccess.h>

#include <QClipboard>
#include <QCursor>
#include <QTextCodec>
#include <QLayout>
#include <QLabel>
#include <QSplitter>
#include <QStyle>
#include <QTreeView>

// X headers...
#undef Never
#undef Always

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#include <kvbox.h>
#include <QTextDocument>
#endif

using namespace KMime;
*/

MailViewer::MailViewer(QWidget *aParent,
                         KSharedConfigPtr config,
                         QWidget *mainWindow,
                         KActionCollection* actionCollection,
                         Qt::WindowFlags aFlags )
  : QWidget(aParent, aFlags ), d_ptr(new MailViewerPrivate(this, config, mainWindow, actionCollection) )
{
  connect( d_ptr, SIGNAL( replaceMsgByUnencryptedVersion() ), SIGNAL( replaceMsgByUnencryptedVersion() ) );
  connect( d_ptr, SIGNAL( popupMenu(KMime::Message &, const KUrl &, const QPoint&) ), SIGNAL( popupMenu(KMime::Message &, const KUrl &, const QPoint&) ) );
  connect( d_ptr, SIGNAL( urlClicked(const KUrl&, int ) ), SIGNAL( urlClicked(const KUrl&, int ) ) );
  connect( d_ptr, SIGNAL( noDrag() ), SIGNAL( noDrag() ) );
  setMessage( 0, Delayed );
}

MailViewer::~MailViewer()
{
  //the d_ptr is automatically deleted
}


void MailViewer::setMessage(KMime::Message* message, UpdateMode updateMode, Ownership ownership)
{
  Q_D(MailViewer);
  d->setMessage( message, updateMode, ownership);
}


void MailViewer::setMessageItem(const Akonadi::Item &item, UpdateMode updateMode)
{
  Q_D(MailViewer);
  d->setMessageItem( item, updateMode );
}

void MailViewer::displaySplashPage( const QString &info )
{
  Q_D(MailViewer);
  d->displaySplashPage( info );
}


void MailViewer::enableMessageDisplay()
{
  Q_D(MailViewer);
  d->enableMessageDisplay();
}

void MailViewer::printMessage( KMime::Message* message )
{
   Q_D(MailViewer);
   d->printMessage( message );
}

void MailViewer::print()
{
  Q_D(MailViewer);
  if ( !message() )
    return;
  d->mViewer->view()->print();
}

void MailViewer::resizeEvent( QResizeEvent * )
{
  Q_D(MailViewer);
  if( !d->mResizeTimer.isActive() )
  {
    //
    // Combine all resize operations that are requested as long a
    // the timer runs.
    //
    d->mResizeTimer.start( 100 );
  }
}

void MailViewer::closeEvent( QCloseEvent *e )
{
  Q_D(MailViewer);
  QWidget::closeEvent( e );
  d->writeConfig();
}

void MailViewer::slotScrollUp()
{
  Q_D(MailViewer);
  d->mViewer->view()->scrollBy( 0, -10 );
}

void MailViewer::slotScrollDown()
{
  Q_D(MailViewer);
  d->mViewer->view()->scrollBy( 0, 10 );
}

bool MailViewer::atBottom() const
{
  Q_D(const MailViewer);
  KHTMLView *view = d->mViewer->view();
  return view->contentsY() + view->visibleHeight() >= view->contentsHeight();
}

void MailViewer::slotJumpDown()
{
  Q_D(MailViewer);
  KHTMLView *view = d->mViewer->view();
  view->scrollBy( 0, view->visibleHeight() );
}

void MailViewer::slotScrollPrior()
{
  Q_D(MailViewer);
  KHTMLView *view = d->mViewer->view();
  view->scrollBy( 0, -(int)(height() * 0.8 ) );
}

void MailViewer::slotScrollNext()
{
  Q_D(MailViewer);
  KHTMLView *view = d->mViewer->view();
  view->scrollBy( 0, (int)(height() * 0.8 ) );
}

QString MailViewer::selectedText()
{
  Q_D(MailViewer);
  QString temp = d->mViewer->selectedText();
  return temp;
}

void MailViewer::setHtmlOverride( bool override )
{
  Q_D(MailViewer);
  d->setHtmlOverride( override );
}

bool MailViewer::htmlOverride() const
{
  Q_D(const MailViewer);;
  return d->htmlOverride();
}

void MailViewer::setHtmlLoadExtOverride( bool override )
{
  Q_D(MailViewer);
  d->setHtmlLoadExtOverride( override );
}

bool MailViewer::htmlLoadExtOverride() const
{
  Q_D(const MailViewer);
  return d->htmlLoadExtOverride();
}

bool MailViewer::htmlMail() const
{
  Q_D(const MailViewer);
  d->htmlMail();
}

bool MailViewer::htmlLoadExternal() const
{
  Q_D(const MailViewer);
  d->htmlLoadExternal();
}

bool MailViewer::isFixedFont() const
{
  Q_D(const MailViewer);
  return d->mUseFixedFont;

}
void MailViewer::setUseFixedFont( bool useFixedFont )
{
  Q_D(MailViewer);
  d->mUseFixedFont = useFixedFont;
}

QWidget* MailViewer::mainWindow()
{
  Q_D(MailViewer);
  return d->mMainWindow;
}

void MailViewer::setDecryptMessageOverwrite( bool overwrite )
{
  Q_D(MailViewer);
  d->setDecryptMessageOverwrite( overwrite );
}

bool MailViewer::showSignatureDetails() const
{
  Q_D(const MailViewer);
  return d->showSignatureDetails();
}

void MailViewer::setShowSignatureDetails( bool showDetails )
{
  Q_D(MailViewer);
  d->setShowSignatureDetails( showDetails );
}

bool MailViewer::showAttachmentQuicklist() const
{
  Q_D(const MailViewer);
  return d->showAttachmentQuicklist();
}

void MailViewer::setShowAttachmentQuicklist( bool showAttachmentQuicklist  )
{
  Q_D(MailViewer);
  d->setShowAttachmentQuicklist( showAttachmentQuicklist );
}

QWidget* MailViewer::configWidget()
{
  Q_D(MailViewer);
  ConfigureWidget *w = new ConfigureWidget();
  connect( w, SIGNAL( settingsChanged() ), d, SLOT( slotSettingsChanged() ) );
  return w;
}

KConfigSkeleton *MailViewer::configObject()
{
  return GlobalSettings::self();
}

KMime::Message* MailViewer::message() const
{
  Q_D(const MailViewer);
  return d->mMessage;
}

void MailViewer::styleChange( QStyle& oldStyle )
{
  Q_D(MailViewer);
  d->setStyleDependantFrameWidth();
  QWidget::styleChange( oldStyle );
}

bool MailViewer::event(QEvent *e)
{
  Q_D(MailViewer);
  if (e->type() == QEvent::PaletteChange)
  {
    delete d->mCSSHelper;
    d->mCSSHelper = new CSSHelper( d->mViewer->view() );
/*FIXME(Andras) port it
    if (message())
      message()->readConfig();
*/
    d->update( MailViewer::Force ); // Force update
    return true;
  }
  return QWidget::event(e);
}

void MailViewer::slotFind()
{
  Q_D(MailViewer);
  d->mViewer->findText();
}

#include "kmreaderwin.moc"


