/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

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

#include <qpushbutton.h>

#include "mobilegui.h"

#include "mobilemain.h"
#include <kstatusbar.h>
#include "mobilemain.moc"

MobileMain::MobileMain(CommandScheduler *scheduler, KandyPrefs *prefs)
    : KMainWindow( 0, "MobileMain" )
{
  mView = new MobileGui(scheduler, prefs, this);
  setCentralWidget(mView);
  setupActions();

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
}

void MobileMain::optionsConfigureKeys()
{
    KKeyDialog::configure( actionCollection(), this );
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
