// $Id$
// Copyright (C) 2001 Cornelius Schumacher <schumacher@kde.org>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmenubar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kedittoolbar.h>
#include <kurldrag.h>

#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>

#include "mobilegui.h"

#include "mobilemain.h"
#include <kstatusbar.h>
#include "mobilemain.moc"

MobileMain::MobileMain(CommandScheduler *scheduler)
    : KMainWindow( 0, "MobileMain" )
{
  mView = new MobileGui(scheduler,this);

  setCentralWidget(mView);
  setupActions();

//  statusBar()->insertItem(i18n(""),0,10);

  statusBar()->insertItem(i18n(" Disconnected "),1,0,true);
  connect(mView,SIGNAL(statusMessage(const QString &)),
          SLOT(showStatusMessage(const QString &)));
  connect(mView,SIGNAL(transientStatusMessage(const QString &)),
          SLOT(showTransientStatusMessage(const QString &)));
  statusBar()->show();

  setAutoSaveSettings();
}

MobileMain::~MobileMain()
{
}

void MobileMain::setupActions()
{
  KStdAction::quit(this, SLOT(close()), actionCollection());

  new KAction(i18n("Terminal"),0,this,SLOT(showTerminal()),
              actionCollection(),"show_terminal");

  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);
   
  KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
  KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

  new KAction(i18n("Connect"),0,this,SIGNAL(modemConnect()),
              actionCollection(),"modem_connect");
  new KAction(i18n("Disconnect"),0,this,SIGNAL(modemDisconnect()),
              actionCollection(),"modem_disconnect");

  createGUI("kandymobileui.rc");
}

void MobileMain::saveProperties(KConfig */*config*/)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void MobileMain::readProperties(KConfig */*config*/)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}

void MobileMain::dragEnterEvent(QDragEnterEvent *event)
{
    // do nothing
    KMainWindow::dragEnterEvent(event);

    // accept uri drops only
//    event->accept(KURLDrag::canDecode(event));
}

void MobileMain::dropEvent(QDropEvent *event)
{
    // this is a very simplistic implementation of a drop event.  we
    // will only accept a dropped URL.  the Qt dnd code can do *much*
    // much more, so please read the docs there

    // do nothing
    KMainWindow::dropEvent(event);
/*
    KURL::List list;

    // see if we can decode a URI.. if not, just ignore it
    if (KURLDrag::decode(event, list) && !list.isEmpty())
    {
        const KURL &url = uri.first();

        if (url.isLocalFile())
        {
            // load in the file
            load(url.path());
        }
    }
*/
}

void MobileMain::optionsConfigureKeys()
{
    KKeyDialog::configureKeys(actionCollection(),"kandymobileui.rc");
}

void MobileMain::optionsConfigureToolbars()
{
    // use the standard toolbar editor
    saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
    KEditToolbar dlg(actionCollection());
    connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(newToolbarConfig()));
    dlg.exec();
}

void MobileMain::newToolbarConfig()
{
    // recreate our GUI
    createGUI("kandymobileui.rc");
    applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void MobileMain::optionsPreferences()
{
  emit showPreferencesWin();
}

void MobileMain::showStatusMessage(const QString& text)
{
  // display the text on the statusbar
  statusBar()->message(text);
}

void MobileMain::showTransientStatusMessage(const QString& text)
{
  // display the text on the statusbar for 2 s.
  statusBar()->message(text,2000);
}

void MobileMain::changeCaption(const QString& text)
{
  // display the text on the caption
  setCaption(text);
}

bool MobileMain::queryClose()
{
#if 0
  if (m_view->isModified()) {
    switch (KMessageBox::warningYesNoCancel(this,
        i18n("Save changes to profile %1?").arg(mFilename))) {
      case KMessageBox::Yes :
        fileSave();
        return true;
      case KMessageBox::No :
        return true;
      default: // cancel
        return false;
    }
  } else {
    return true;
  }
#endif
  return true;
}

void MobileMain::showTerminal()
{
  emit showTerminalWin();
}

void MobileMain::setConnected(bool connected)
{
  if (connected) {
    statusBar()->changeItem(i18n(" Connected "),1);
    mView->readModelInformation();
    mView->refreshStatus();

  } else {
    statusBar()->changeItem(i18n(" Disconnected "),1);
  }
}
