/*
    knode.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/
#include "knode.h"
#include "knglobals.h"
#include "knwidgets.h"

#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <kmenubar.h>
#include <kiconloader.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kapplication.h>

#include "broadcaststatus.h"
#include "krsqueezedtextlabel.h"
#include "progressdialog.h"
#include "statusbarprogresswidget.h"

//GUI
#include "knmainwidget.h"
#include "knarticlewindow.h"
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"

KNMainWindow::KNMainWindow( TQWidget* pWidget )
  : KMainWindow(pWidget,"mainWindow")
{
  //setupStatusBar();
  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);

  //config stuff
  KStdAction::quit(kapp, TQT_SLOT(closeAllWindows()), actionCollection());
  KStdAction::configureToolbars(this, TQT_SLOT(slotConfToolbar()), actionCollection());
  KStdAction::keyBindings(this, TQT_SLOT(slotConfKeys()), actionCollection());

  m_mainWidget = new KNMainWidget( this, true, this, 0 );
  connect( m_mainWidget, TQT_SIGNAL(signalCaptionChangeRequest(const TQString&)),
           TQT_SLOT( setCaption(const TQString&)) );
  setCentralWidget( m_mainWidget );
  setupStatusBar();
  connect( KPIM::BroadcastStatus::instance(), TQT_SIGNAL(statusMsg(const TQString&)),
    this, TQT_SLOT(slotShowStatusMsg(const TQString& )) );
  createGUI( "knodeui.rc" );
  knGlobals.instance = 0;

  applyMainWindowSettings(KGlobal::config(),"mainWindow_options");
}

KNMainWindow::~KNMainWindow()
{
  saveMainWindowSettings(knGlobals.config(),"mainWindow_options");
}

void KNMainWindow::openURL( const KURL& url )
{
  m_mainWidget->openURL( url );
}

void KNMainWindow::slotConfToolbar()
{
  saveMainWindowSettings(knGlobals.config(),"mainWindow_options");
  KEditToolbar dlg(actionCollection(), "knodeui.rc");
  connect(&dlg,TQT_SIGNAL( newToolbarConfig() ), this, TQT_SLOT( slotNewToolbarConfig() ));
  dlg.exec();
}

void KNMainWindow::slotNewToolbarConfig()
{
  createGUI("knodeui.rc");
  //initPopups();
  applyMainWindowSettings(knGlobals.config(),"mainWindow_options");
}

void KNMainWindow::slotConfKeys()
{
  KKeyDialog::configure(actionCollection(), true);
}

bool KNMainWindow::queryClose()
{
  return m_mainWidget->queryClose();
}

void KNMainWindow::setupStatusBar()
{
  mProgressDialog = new KPIM::ProgressDialog( statusBar(), this );
  mProgressDialog->hide();

  mLittleProgress = new StatusbarProgressWidget( mProgressDialog, statusBar() );
  mLittleProgress->show();

  statusBar()->addWidget( mLittleProgress, 0 , true );

  mStatusMsgLabel = new KRSqueezedTextLabel( TQString::null, statusBar() );
  mStatusMsgLabel->setAlignment( AlignLeft | AlignVCenter );
  statusBar()->addWidget( mStatusMsgLabel, 2 );
  statusBar()->addWidget(m_mainWidget->statusBarLabelFilter(), 2);
  statusBar()->addWidget(m_mainWidget->statusBarLabelGroup(), 3);
}

void KNMainWindow::slotShowStatusMsg( const TQString &msg ) {
  mStatusMsgLabel->setText( msg );
}

#include "knode.moc"
