/***************************************************************************
                     knode.cpp - description
 copyright            : (C) 1999 by Christian Thurner
 email                : cthurner@freepage.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
#include "utilities.h"
#include "knglobals.h"
#include "knconfigmanager.h"



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
  progVal=0;
  setFormat(QString::null);
  setValue(0);
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



// display 0%
void KNProgress::initProgressBar()
{
  progVal=0;
  setFormat("%p%");
  setValue(1);
}



// add 10%
void KNProgress::stepProgressBar()
{
  progVal+=100;
  if(progVal>=1000) progVal=1000;
  setFormat("%p%");
  setValue(progVal);
}



// display 100%
void KNProgress::fullProgressBar()
{
  setFormat("%p%");
  setValue(1000);
}



QSize KNProgress::sizeHint() const
{
  return QSize(KProgress::sizeHint().width(),desHeight);
}


//===============================================================================================



KNMainWindow::KNMainWindow() : KMainWindow(0,"mainWindow"), b_lockInput(false)
{
  //bool is_first_start = firstStart();

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
  setStatusMsg();

  //view
  setCaption(i18n("KDE News Reader"));
  v_iew=new KNodeView(this, "knodeView");
  setCentralWidget(v_iew);
  knGlobals.view=v_iew;

  //actions
  a_ccel=new KAccel(this);
  v_iew->a_ctNavReadThrough->plugAccel(a_ccel);

  KStdAction::quit(kapp, SLOT(closeAllWindows()), actionCollection());
  //KStdAction::saveOptions(this, SLOT(slotSaveOptions()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  KStdAction::preferences(this, SLOT(slotSettings()), actionCollection());

  a_ctWinToggleToolbar    = KStdAction::showToolbar(this, SLOT(slotWinToggleToolbar()), actionCollection());
  a_ctWinToggleStatusbar  = KStdAction::showStatusbar(this, SLOT(slotWinToggleStatusbar()), actionCollection());

  v_iew->slotCollectionSelected(0); //disable view-actions
  createGUI("knodeui.rc");
  v_iew->initPopups(this);

  //apply settings
  KConfig *conf = KGlobal::config();
  conf->setGroup("mainWindow_options");
  resize(759,478);  // default optimized for 800x600
  applyMainWindowSettings(conf);
  a_ctWinToggleToolbar->setChecked(!toolBar()->isHidden());
  a_ctWinToggleStatusbar->setChecked(!statusBar()->isHidden());


#warning FIXME
  /*if (is_first_start) {  // open the config dialog on the first start
    show();              // the settings dialog must appear in front of the main window!
    slotSettings();
  }*/
}



KNMainWindow::~KNMainWindow()
{
  delete a_ccel;
}


//============================ URL handling ==============================


void KNMainWindow::openURL(const KURL &url)
{
#warning FIXME
  /*QString host = url.host();
  unsigned short int port = url.port();
  KNNntpAccount *acc;

  // lets see if we already have an account for this host...
  for (acc = AManager->first(); acc; acc = AManager->next())
    if ((acc->server()==host)&&((port == 0) || (acc->port()==port)))
      break;

  if (!acc) {
    acc = new KNNntpAccount();
    acc->setName(host);
    acc->setServer(host);
    if (port != 0)
      acc->setPort(port);
    if (url.hasUser() && url.hasPass()) {
      acc->setNeedsLogon(true);
      acc->setUser(url.user());
      acc->setPass(url.pass());
    }
    if (!AManager->newAccount(acc))
      return;
  }

  QString groupname = url.path(-1);
  while (groupname.startsWith("/"))
    groupname.remove(0,1);

  QListViewItem *item = 0;
  if (groupname.isEmpty())
    item = acc->listItem();
  else {
    KNGroup *grp = GManager->group(groupname.local8Bit(),acc);
    if (!grp) {
      KNGroupInfo inf(groupname.local8Bit(),"");
      GManager->subscribeGroup(&inf,acc);
      grp = GManager->group(groupname.local8Bit(),acc);
      if (grp)
        item = grp->listItem();
    } else
      item = grp->listItem();
  }
  if (item) {
    view->collectionView->setCurrentItem(item);
    view->collectionView->ensureItemVisible(item);
    view->collectionView->setSelected(item, true);
  } */
}


//================================== GUI =================================


void KNMainWindow::setStatusMsg(const QString& text, int id)
{
  statusBar()->clear();
  if (text.isEmpty() && (id==SB_MAIN))
    statusBar()->changeItem(i18n(" Ready"),SB_MAIN);
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
  KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
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


/*void KNMainWindow::cleanup()
{
  KNPurgeProgressDialog *ppdlg=0;

  saveSettings();

  if(GManager->timeToExpire()) {
    ppdlg=new KNPurgeProgressDialog();
    ppdlg->show();
    GManager->expireAll(ppdlg);
  }
  else
    GManager->syncGroups();

  if(FoManager->timeToCompact()) {
    if(!ppdlg) {
      ppdlg=new KNPurgeProgressDialog();
      ppdlg->show();
    }
    FoManager->compactAll(ppdlg);
  }
  else
    FoManager->syncFolders();

  AManager->saveYourself();

  KNViewHeader::clear();
  KNArticleManager::deleteTempFiles();

  if(ppdlg) delete ppdlg;
}*/



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
