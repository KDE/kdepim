/*
    knode.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qpixmap.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstdaction.h>
#include <kaccel.h>
#include <kurl.h>
#include <klocale.h>
#include <kwin.h>
#include <kdebug.h>
#include <kmenubar.h>

#include "knode.h"
#include "knodeview.h"
#include "knlistview.h"
#include "utilities.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "knaccountmanager.h"
#include "knserverinfo.h"
#include "knarticlewidget.h"
#include "knnetaccess.h"


KNGlobals knGlobals;

//======================================================================================================

KNProgress::KNProgress (int desiredHeight, int minValue, int maxValue, int value, KProgress::Orientation orient, QWidget *parent, const char *name)
 : KProgress(minValue, maxValue, value, orient, parent, name), desHeight(desiredHeight)
{
  setFixedWidth(110);
  setFrameStyle(QFrame::Box | QFrame::Raised);
  setLineWidth(1);
  setBackgroundMode(QWidget::PaletteBackground);
  disableProgressBar();
}


KNProgress::~KNProgress()
{}


// 0% and no text
void KNProgress::disableProgressBar()
{
  setFormat(QString::null);
  setValue(0);
  repaint(false);
}


// manual operation
void KNProgress::setProgressBar(int value,const QString& text)
{
  setFormat(text);
  if (value>1000) {
    setValue(1000);
    update();       // circumvent the optimization of setValue
  } else
    setValue(value);
}


QSize KNProgress::sizeHint() const
{
  return QSize(KProgress::sizeHint().width(),desHeight);
}


//===============================================================================================


KNMainWindow::KNMainWindow() : KMainWindow(0,"mainWindow"), b_lockInput(false)
{
  knGlobals.top=this;
  knGlobals.topWidget=this;
  kapp->setMainWidget(this);  // this makes the external viewer windows close on shutdown...

  //statusbar
  KStatusBar *sb=statusBar();
  p_rogBar=new KNProgress(sb->sizeHint().height()-4,0,1000,0, KProgress::Horizontal,sb );
  knGlobals.progressBar=p_rogBar;
  sb->addWidget(p_rogBar);
  sb->insertItem(QString::null, SB_MAIN,2);
  sb->setItemAlignment (SB_MAIN,AlignLeft | AlignVCenter);
  sb->insertItem(QString::null, SB_FILTER,2);
  sb->setItemAlignment (SB_FILTER,AlignLeft | AlignVCenter);
  sb->insertItem(QString::null,SB_GROUP,3);
  sb->setItemAlignment (SB_GROUP,AlignLeft | AlignVCenter);

  //view
  setCaption(i18n("KDE News Reader"));
  v_iew=new KNodeView(this, "knodeView");
  setCentralWidget(v_iew);
  knGlobals.view=v_iew;

  //actions
  a_ccel=new KAccel(this);
  v_iew->a_ctNavReadThrough->plugAccel(a_ccel);
  v_iew->a_ctArtFilterKeyb->plugAccel(a_ccel);
  v_iew->a_ctArtSortHeadersKeyb->plugAccel(a_ccel);
  v_iew->articleView()->setCharsetKeyboardAction()->plugAccel(a_ccel);
  
  KStdAction::quit(kapp, SLOT(closeAllWindows()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  KStdAction::preferences(this, SLOT(slotSettings()), actionCollection());

  a_ctWinToggleToolbar   = KStdAction::showToolbar(this, SLOT(slotWinToggleToolbar()), actionCollection());
  a_ctWinToggleStatusbar = KStdAction::showStatusbar(this, SLOT(slotWinToggleStatusbar()), actionCollection());

  createGUI("knodeui.rc");
  v_iew->initPopups(this);

  //apply settings
  KConfig *conf = KGlobal::config();
  conf->setGroup("mainWindow_options");
  resize(759,478);  // default optimized for 800x600
  applyMainWindowSettings(conf);
  a_ctWinToggleToolbar->setChecked(!toolBar()->isHidden());
  a_ctWinToggleStatusbar->setChecked(!statusBar()->isHidden());

  // set the keyboard focus indicator on the first item in the Collection View
  // we do this here because everything needs to be properly setup when we
  // call setActive()
  if(v_iew->collectionView()->firstChild()) {
    QListViewItem *i = v_iew->collectionView()->firstChild();
    bool open = i->isOpen();
    v_iew->collectionView()->setActive(i,true);
    i->setOpen(open);
  }
  v_iew->collectionView()->setFocus();

  setStatusMsg();

  if(firstStart()) {  // open the config dialog on the first start
    show();              // the settings dialog must appear in front of the main window!
    slotSettings();
  }
}



KNMainWindow::~KNMainWindow()
{
  KConfig *conf = KGlobal::config();
  conf->setGroup("mainWindow_options");
  saveMainWindowSettings(conf);
  delete a_ccel;
}


//================================== GUI =================================


void KNMainWindow::setStatusMsg(const QString& text, int id)
{
  statusBar()->clear();
  if (text.isEmpty() && (id==SB_MAIN))
    if (knGlobals.netAccess->currentMsg().isEmpty())
      statusBar()->changeItem(i18n(" Ready"),SB_MAIN);
    else
      statusBar()->changeItem(knGlobals.netAccess->currentMsg(), SB_MAIN);   // restore the original message
  else
    statusBar()->changeItem(text, id);
}


void KNMainWindow::setStatusHelpMsg(const QString& text)
{
   statusBar()->message(text, 2000);
}


void KNMainWindow::setCursorBusy(bool b)
{
  if(b) kapp->setOverrideCursor(waitCursor);
  else  kapp->restoreOverrideCursor();
}


void KNMainWindow::blockUI(bool b)
{
  b_lockInput = b;
  menuBar()->setEnabled(!b);
  a_ccel->setEnabled(!b);
  v_iew->blockUI(b);
  setCursorBusy(b);
}


// processEvents with some blocking
void KNMainWindow::secureProcessEvents()
{
  b_lockInput = true;
  menuBar()->setEnabled(false);
  a_ccel->setEnabled(false);
  v_iew->blockUI(true);

  kapp->processEvents();

  b_lockInput = false;
  v_iew->blockUI(false);
  menuBar()->setEnabled(true);
  a_ccel->setEnabled(true);
}


QSize KNMainWindow::sizeHint() const
{
  return QSize(759,478);    // default optimized for 800x600
}


void KNMainWindow::slotWinToggleToolbar()
{
  if(toolBar()->isVisible())
    toolBar()->hide();
  else
    toolBar()->show();
}


void KNMainWindow::slotWinToggleStatusbar()
{
  if (statusBar()->isVisible())
    statusBar()->hide();
  else
    statusBar()->show();
}


void KNMainWindow::slotConfKeys()
{
  KActionCollection coll(*actionCollection());

  coll.take(v_iew->a_ctArtSortHeaders);   // hack, remove actions which cant have a shortcut
  coll.take(v_iew->a_ctArtFilter);
  coll.take(v_iew->articleView()->setCharsetAction());

  KKeyDialog::configureKeys(&coll, xmlFile(), true, this);
}


void KNMainWindow::slotConfToolbar()
{
  KEditToolbar *dlg = new KEditToolbar(guiFactory(),this);

  if (dlg->exec()) {
    createGUI("knodeui.rc");
    v_iew->initPopups(this);
  }

  delete dlg;
}


void KNMainWindow::slotSettings()
{
  v_iew->c_fgManager->configure();
}


bool KNMainWindow::firstStart()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("GENERAL");
  QString ver = conf->readEntry("Version");
  if(!ver.isEmpty())
    return false;

  KConfig emailConf("emaildefaults");

  emailConf.setGroup("UserInfo");
  KNConfig::Identity *id=knGlobals.cfgManager->identity();
  id->setName(emailConf.readEntry("FullName"));
  id->setEmail(emailConf.readEntry("EmailAddress").latin1());
  id->setOrga(emailConf.readEntry("Organization"));
  id->setReplyTo(emailConf.readEntry("ReplyAddr"));
  id->save();

  emailConf.setGroup("ServerInfo");
  KNServerInfo *smtp=knGlobals.accManager->smtp();
  smtp->setServer(emailConf.readEntry("Outgoing").latin1());
  smtp->setPort(25);
  conf->setGroup("MAILSERVER");
  smtp->saveConf(conf);

  conf->setGroup("GENERAL");
  conf->writeEntry("Version", KNODE_VERSION);

  return true;
}


bool KNMainWindow::queryClose()
{
  if(b_lockInput)
    return false;
  if(!v_iew->cleanup())
    return false;

  return true;
}


void KNMainWindow::fontChange( const QFont & )
{
  p_rogBar->setFont(font());    // should be called automatically?
}


void KNMainWindow::paletteChange( const QPalette & )
{
  p_rogBar->setPalette(palette());    // should be called automatically?
}


//--------------------------------

#include "knode.moc"
