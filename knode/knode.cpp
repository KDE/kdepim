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

#include "knuserentry.h"
#include "knjobdata.h"
#include "knnetaccess.h"
#include "knpurgeprogressdialog.h"
#include "knarticlewidget.h"
#include "knodeview.h"
#include "knsettingsdialog.h"
#include "knhdrviewitem.h"
#include "kncollectionviewitem.h"
#include "knviewheader.h"
#include "knfetcharticlemanager.h"
#include "knsavedarticlemanager.h"
#include "kngroupmanager.h"
#include "knaccountmanager.h"
#include "knfoldermanager.h"
#include "knfiltermanager.h"
#include "knappmanager.h"
#include "knfolder.h"
#include "kngroup.h"
#include "utilities.h"
#include "resource.h"
#include "knglobals.h"
#include "knarticle.h"
#include "knnntpaccount.h"
#include "knode.h"


KNGlobals knGlobals;


//========================================================================
//=============================== KNPROGRESS =============================
//========================================================================


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



//========================================================================
//=============================== KNODEAPP ===============================
//========================================================================


KNodeApp::KNodeApp()
  : KMainWindow(0,"mainWindow"), setDialog(0), blockInput(false)
{
  bool is_first_start = firstStart();

  knGlobals.top=this;
  knGlobals.topWidget=this;
  kapp->setMainWidget(this);  // this makes the external viewer windows close on shutdown...

  // load color&font settings
  AppManager = new KNAppManager();
  knGlobals.appManager = AppManager;

  //init the GUI
  setCaption(i18n("KDE News Reader"));
  initView();
  knGlobals.view = view;
  KNLVItemBase::initIcons();
  initStatusBar();

  //init Net
  NAcc=new KNNetAccess(actionCollection());
  knGlobals.netAccess = NAcc;

  //init filter manager
  FiManager=new KNFilterManager(actionCollection());
  knGlobals.fiManager = FiManager;

  //init Fetch-Article Manager
  FAManager=new KNFetchArticleManager(view->hdrView, FiManager, actionCollection());
  knGlobals.fArtManager = FAManager;

  //init Group Manager
  GManager=new KNGroupManager(FAManager, actionCollection());
  knGlobals.gManager = GManager;

  //init Account Manager
  AManager=new KNAccountManager(GManager, view->collectionView, actionCollection());
  knGlobals.accManager = AManager;

  //init Saved-Article Manager
  SAManager=new KNSavedArticleManager(view->hdrView, AManager, actionCollection());
  knGlobals.sArtManager = SAManager;

  //init Folder Manager
  FoManager=new KNFolderManager(SAManager, view->collectionView, actionCollection());
  knGlobals.foManager = FoManager;

  // all components that provide actions are created, now
  // build menu- & toolbar
  initActions();
  initPopups();

  KConfig *conf = KGlobal::config();
  conf->setGroup("mainWindow_options");
  resize(759,478);  // default optimized for 800x600
  applyMainWindowSettings(conf);
  actShowToolbar->setChecked(!toolBar()->isHidden());
  actShowStatusbar->setChecked(!statusBar()->isHidden());

  // set the keyboard focus indicator on the first item in the collectionView
  if(view->collectionView->firstChild())
    view->collectionView->setCurrentItem(view->collectionView->firstChild());
  view->collectionView->setFocus();

  if (is_first_start) {  // open the config dialog on the first start
    show();              // the settings dialog must appear in front of the main window!
    slotSettings();
  }
}



KNodeApp::~KNodeApp()
{
  KNLVItemBase::clearIcons();

  delete acc;
  delete setDialog;

  delete NAcc;
  kdDebug(5003) << "Net deleted" << endl;

  delete AManager;
  kdDebug(5003) << "AManager deleted" << endl;

  delete GManager;
  kdDebug(5003) << "GManager deleted" << endl;

  delete FAManager;
  kdDebug(5003) << "FAManager deleted" << endl;

  delete FoManager;
  kdDebug(5003) << "FoManager deleted" << endl;

  delete SAManager;
  kdDebug(5003) << "SAManager deleted" << endl;

  delete FiManager;
  kdDebug(5003) << "FiManager deleted" << endl;

  delete AppManager;
  kdDebug(5003) << "AppManager deleted" << endl;
}


//============================ URL handling ==============================


void KNodeApp::openURL(const KURL &url)
{
  QString host = url.host();
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
  }
}


//================================== GUI =================================


void KNodeApp::setStatusMsg(const QString& text, int id)
{
  statusBar()->clear();
  if (text.isEmpty() && (id==SB_MAIN))
    statusBar()->changeItem(i18n(" Ready"),SB_MAIN);
  else
    statusBar()->changeItem(text, id);
}



void KNodeApp::setStatusHelpMsg(const QString& text)
{
   statusBar()->message(text, 2000);
}



void KNodeApp::setCursorBusy(bool b)
{
  if(b) kapp->setOverrideCursor(waitCursor);
  else  kapp->restoreOverrideCursor();
}



void KNodeApp::blockUI(bool b)
{
  blockInput = b;
  menuBar()->setEnabled(!b);
  acc->setEnabled(!b);
  setCursorBusy(b);
}



// processEvents with some blocking
void KNodeApp::secureProcessEvents()
{
  blockInput = true;
  menuBar()->setEnabled(false);
  acc->setEnabled(false);

  kapp->processEvents();

  blockInput = false;
  menuBar()->setEnabled(true);
  acc->setEnabled(true);
}


//============================ INIT && UPDATE ============================


void KNodeApp::initView()
{
  KNArticleWidget::readOptions();
  KNViewHeader::loadAll();
  view = new KNodeView(actionCollection(),this,"knodeView");
  setCentralWidget(view);

  connect(view->collectionView, SIGNAL(clicked(QListViewItem *)),
    this, SLOT(slotCollectionClicked(QListViewItem *)));

  connect(view->collectionView, SIGNAL(selectionChanged(QListViewItem *)),
    this, SLOT(slotCollectionSelected(QListViewItem *)));

  connect(view->collectionView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotCollectionPopup(QListViewItem*, const QPoint&, int)));

  connect(view->hdrView, SIGNAL(selectionChanged(QListViewItem *)),
    this, SLOT(slotHeaderSelected(QListViewItem *)));

  connect(view->hdrView, SIGNAL(doubleClicked(QListViewItem*)),
    this, SLOT(slotHeaderDoubleClicked(QListViewItem*)));

  connect(view->hdrView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotArticlePopup(QListViewItem*, const QPoint&, int)));
}


void KNodeApp::initStatusBar()
{
  KStatusBar *sb=statusBar();

  progBar = new KNProgress(sb->sizeHint().height()-4,0,1000,0, KProgress::Horizontal,sb );
  knGlobals.progressBar = progBar;
  sb->addWidget(progBar);

  sb->insertItem(QString::null, SB_MAIN,2);
  sb->setItemAlignment (SB_MAIN,AlignLeft | AlignVCenter);
  sb->insertItem(QString::null, SB_FILTER,2);
  sb->setItemAlignment (SB_FILTER,AlignLeft | AlignVCenter);
  sb->insertItem(QString::null,SB_GROUP,3);
  sb->setItemAlignment (SB_GROUP,AlignLeft | AlignVCenter);
  setStatusMsg();
}


void KNodeApp::initActions()
{
  KStdAction::quit(kapp, SLOT(closeAllWindows()), actionCollection());

  acc=new KAccel(this);
  view->actReadThrough->plugAccel(acc);
  actShowAllHdrs = new KToggleAction(i18n("Show &all headers"), "text_block", 0 , this, SLOT(slotToggleShowAllHdrs()),
                                     actionCollection(), "view_showAllHdrs");
  actShowAllHdrs->setChecked(KNArticleWidget::fullHeaders());

  actCancel = new KAction(i18n("article","&Cancel"), 0 , this, SLOT(slotCancel()),
                          actionCollection(), "article_cancel");
  actCancel->setEnabled(false);
  actSupersede = new KAction(i18n("S&upersede"), 0 , this, SLOT(slotSupersede()),
                             actionCollection(), "article_supersede");
  actSupersede->setEnabled(false);
  connect(FAManager, SIGNAL(currentArticleChanged()), SLOT(slotCurrentArticleChanged()));
  connect(SAManager, SIGNAL(currentArticleChanged()), SLOT(slotCurrentArticleChanged()));

  actShowToolbar = KStdAction::showToolbar(this, SLOT(slotToggleToolBar()), actionCollection());
  actShowStatusbar = KStdAction::showStatusbar(this, SLOT(slotToggleStatusBar()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  KStdAction::preferences(this, SLOT(slotSettings()), actionCollection());

  createGUI("knodeui.rc");
}



void KNodeApp::initPopups()
{
  accPopup = static_cast<QPopupMenu *>(factory()->container("account_popup", this));
  if (!accPopup) accPopup = new QPopupMenu();
  groupPopup = static_cast<QPopupMenu *>(factory()->container("group_popup", this));
  if (!groupPopup) groupPopup = new QPopupMenu();
  folderPopup = static_cast<QPopupMenu *>(factory()->container("folder_popup", this));
  if (!folderPopup) folderPopup = new QPopupMenu();
  fetchPopup = static_cast<QPopupMenu *>(factory()->container("fetch_popup", this));
  if (!fetchPopup) fetchPopup = new QPopupMenu();
  savedPopup = static_cast<QPopupMenu *>(factory()->container("saved_popup", this));
  if (!savedPopup) savedPopup = new QPopupMenu();
}



void KNodeApp::saveSettings()
{
  KConfig *conf = KGlobal::config();
  conf->setGroup("mainWindow_options");
  saveMainWindowSettings(conf);

  view->saveOptions();
  FiManager->saveOptions();
  FAManager->saveOptions();
  KNArticleWidget::saveOptions();
  AppManager->saveOptions();

  conf->setGroup("GENERAL");
  conf->writeEntry("Version",KNODE_VERSION);
}



// checks if run for the first time, sets some global defaults (email configuration)
bool KNodeApp::firstStart()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("GENERAL");
  QString ver = conf->readEntry("Version");
  if (!ver.isEmpty())
    return false;

  KConfig emailConf("emaildefaults");

  emailConf.setGroup("UserInfo");
  KNUserEntry *user=new KNUserEntry();
  user->setName(emailConf.readEntry("FullName").local8Bit());
  user->setEmail(emailConf.readEntry("EmailAddress").local8Bit());
  user->setOrga(emailConf.readEntry("Organization").local8Bit());
  user->setReplyTo(emailConf.readEntry("ReplyAddr").local8Bit());
  conf->setGroup("IDENTITY");
  user->save(conf);
  delete user;

  emailConf.setGroup("ServerInfo");
  KNServerInfo *serverInfo=new KNServerInfo();
  serverInfo->setType(KNServerInfo::STsmtp);
  serverInfo->setServer(emailConf.readEntry("Outgoing").local8Bit());
  serverInfo->setPort(25);
  conf->setGroup("MAILSERVER");
  serverInfo->saveConf(conf);
  delete serverInfo;

  return true;
}


//================================ SLOTS =================================


void KNodeApp::slotToggleShowAllHdrs()
{
  KNArticleWidget::toggleFullHeaders();
}


void KNodeApp::slotCancel()
{
  if (FAManager->hasCurrentArticle())
    SAManager->cancel(FAManager->currentArticle(),FAManager->group());
  else
    SAManager->cancel();
}


void KNodeApp::slotSupersede()
{
  if (FAManager->hasCurrentArticle())
    SAManager->supersede(FAManager->currentArticle(),FAManager->group());
  else
    SAManager->supersede();
}



void KNodeApp::slotToggleToolBar()
{
  if(toolBar()->isVisible())
    toolBar()->hide();
  else
    toolBar()->show();
}



void KNodeApp::slotToggleStatusBar()
{
  if (statusBar()->isVisible())
    statusBar()->hide();
  else
    statusBar()->show();
}



void KNodeApp::slotConfKeys()
{
  KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}


void KNodeApp::slotConfToolbar()
{
  KEditToolbar *dlg = new KEditToolbar(guiFactory(),this);

  if (dlg->exec()) {
    createGUI("knodeui.rc");
    initPopups();
  }

  delete dlg;
}



void KNodeApp::slotSettings()
{
  if (!setDialog) {
    setDialog = new KNSettingsDialog(this);
    connect(setDialog, SIGNAL(finished()), this, SLOT(slotSettingsFinished()));
    setDialog->show();
  } else {
    KWin::setActiveWindow(setDialog->winId());
  }
}




void KNodeApp::slotSettingsFinished()
{
  setDialog->delayedDestruct();
  setDialog=0;
}



// enable/disable Cancel & Supersede actions
void KNodeApp::slotCurrentArticleChanged()
{
  bool b = FAManager->hasCurrentArticle() || SAManager->hasCurrentArticle();
  actCancel->setEnabled(b);
  actSupersede->setEnabled(b);
}


//==================== VIEW-SLOTS ======================



void KNodeApp::slotCollectionClicked(QListViewItem *it)
{
  if(it && (static_cast<KNCollectionViewItem*>(it)->coll->type()==KNCollection::CTnntpAccount))
    it->setOpen(!it->isOpen());
}



void KNodeApp::slotCollectionSelected(QListViewItem *it)
{
  if (blockInput)
    return;

  KNGroup *grp=0;
  KNFolder *fldr=0;
  KNNntpAccount *acc=0;
  view->hdrView->clear();

  if(it) {
    KNCollectionViewItem* collI = static_cast<KNCollectionViewItem*>(it);

    if(collI->coll->type()==KNCollection::CTgroup) {
      if (!(view->hdrView->hasFocus())&&!(view->artView->hasFocus()))
        view->hdrView->setFocus();
      grp=(KNGroup*)((KNCollectionViewItem*)it)->coll;
      acc=(KNNntpAccount*)grp->account();
      setCaption(grp->name());
    }
    else if(collI->coll->type()==KNCollection::CTfolder) {
      if (!(view->hdrView->hasFocus())&&!(view->artView->hasFocus()))
        view->hdrView->setFocus();
      fldr=(KNFolder*)((KNCollectionViewItem*)it)->coll;
      setStatusMsg(QString::null, SB_FILTER);
      setCaption(fldr->name());
    }
    else if(collI->coll->type()==KNCollection::CTnntpAccount) {
      acc=(KNNntpAccount*)((KNCollectionViewItem*)it)->coll;
      setStatusMsg(QString::null, SB_GROUP);
      setStatusMsg(QString::null, SB_FILTER);
      setCaption(acc->name());
      view->artView->showBlankPage();
    }
  }

  AManager->setCurrentAccount(acc);
  GManager->setCurrentGroup(grp);
  FiManager->setIsAGroup((grp));    // filters currently only work for groups
  FoManager->setCurrentFolder(fldr);
  view->setNotAFolder(!(fldr));
  view->setHeaderSelected(false);
}



void KNodeApp::slotHeaderSelected(QListViewItem *it)
{
  if (blockInput)
    return;

  KNFetchArticle *fart=0;
  KNSavedArticle *sart=0;
  if(it) {
    if(static_cast<KNHdrViewItem*>(it)->art->type()==KNArticleBase::ATfetch)
      fart=(KNFetchArticle*)(static_cast<KNHdrViewItem*>(it)->art);
    else
      sart=(KNSavedArticle*)(static_cast<KNHdrViewItem*>(it)->art);
  }
  FAManager->setCurrentArticle(fart);
  SAManager->setCurrentArticle(sart);
  view->setHeaderSelected((fart));
}



void KNodeApp::slotHeaderDoubleClicked(QListViewItem *it)
{
  if (blockInput)
    return;

  KNFetchArticle *fart=0;
  KNSavedArticle *sart=0;
  if(it) {
    if(static_cast<KNHdrViewItem*>(it)->art->type()==KNArticleBase::ATfetch) {
      fart=(KNFetchArticle*)((KNHdrViewItem*)it)->art;
      FAManager->articleWindow(fart);
    }
    else {
      sart=(KNSavedArticle*)((KNHdrViewItem*)it)->art;
      SAManager->editArticle(sart);
   }
  }
}



void KNodeApp::slotArticlePopup(QListViewItem *it, const QPoint &p, int)
{
  if (blockInput)
    return;

  if (it) {
    if (static_cast<KNHdrViewItem*>(it)->art->type()==KNArticleBase::ATfetch)
      fetchPopup->popup(p);
    else
      savedPopup->popup(p);
  }
}



void KNodeApp::slotCollectionPopup(QListViewItem *it, const QPoint &p, int)
{
  if (blockInput)
    return;

  if (it) {
    if ((static_cast<KNCollectionViewItem*>(it))->coll->type()==KNCollection::CTgroup) {
      groupPopup->popup(p);
    } else {
      if ((static_cast<KNCollectionViewItem*>(it))->coll->type()==KNCollection::CTfolder)
        folderPopup->popup(p);
      else
        accPopup->popup(p);
    }
  }
}


//================================ OTHERS ================================


void KNodeApp::cleanup()
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
}



bool KNodeApp::queryClose()
{
  if (blockInput)
    return false;
  if (!SAManager->closeComposeWindows())
    return false;
  cleanup();
  return true;
}



void KNodeApp::jobDone(KNJobData *j)
{
  if(!j) return;
  //kdDebug(5003) << "KNodeApp::jobDone() : job received" << endl;

  switch(j->type()) {
    case KNJobData::JTLoadGroups:
    case KNJobData::JTFetchGroups:
    case KNJobData::JTCheckNewGroups:
    case KNJobData::JTfetchNewHeaders:
      kdDebug(5003) << "KNodeApp::jobDone() : job sent to GManager" << endl;
      GManager->jobDone(j);
    break;
    case KNJobData::JTfetchArticle:
      kdDebug(5003) << "KNodeApp::jobDone() : job sent to FAManager" << endl;
      FAManager->jobDone(j);
    break;
    case KNJobData::JTpostArticle:
    case KNJobData::JTmail:
      kdDebug(5003) << "KNodeApp::jobDone() : job sent to SAManager" << endl;
      SAManager->jobDone(j);
    break;
  };
}


void KNodeApp::fontChange( const QFont & )
{
  progBar->setFont(font());    // should be called automatically?
  AppManager->updateVisualDefaults();
}


void KNodeApp::paletteChange( const QPalette & )
{
  progBar->setPalette(palette());    // should be called automatically?
  AppManager->updateVisualDefaults();
}


//--------------------------------

#include "knode.moc"
