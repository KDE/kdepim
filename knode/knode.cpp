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
#include <khtml_part.h>
#include <kwin.h>

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
  if (value>1000)
    value = 1000;
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
  : KMainWindow(0), setDialog(0)
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
  NAcc=new KNNetAccess();
  knGlobals.netAccess = NAcc;

  //init filter manager
  FiManager=new KNFilterManager();
  knGlobals.fiManager = FiManager;

  //init Fetch-Article Manager
  FAManager=new KNFetchArticleManager(view->hdrView, FiManager);
  knGlobals.fArtManager = FAManager;

  //init Group Manager
  GManager=new KNGroupManager(FAManager);
  knGlobals.gManager = GManager;

  //init Account Manager
  AManager=new KNAccountManager(GManager, view->collectionView);
  knGlobals.accManager = AManager;

  //init Saved-Article Manager
  SAManager=new KNSavedArticleManager(view->hdrView, AManager);
  knGlobals.sArtManager = SAManager;

  //init Folder Manager
  FoManager=new KNFolderManager(SAManager, view->collectionView);
  knGlobals.foManager = FoManager;

  // all components that provide actions are created, now
  // build menu- & toolbar
  initActions();
  initPopups();

  restoreWindowSize("main", this, QSize(759,478));    // default optimized for 800x600

  // set the keyboard focus indicator on the first item in the collectionView
  if(view->collectionView->firstChild())
    view->collectionView->setCurrentItem(view->collectionView->firstChild());
  view->collectionView->setFocus();

  if (is_first_start)  // open the config dialog on the first start
    slotSettings();
}



KNodeApp::~KNodeApp()
{
  KNLVItemBase::clearIcons();

  delete acc;
  delete setDialog;

  delete NAcc;
  qDebug("Net deleted\n");

  delete AManager;
  qDebug("AManager deleted\n");

  delete GManager;
  qDebug("GManager deleted\n");

  delete FAManager;
  qDebug("FAManager deleted\n");

  delete FoManager;
  qDebug("FoManager deleted\n");

  delete SAManager;
  qDebug("SAManager deleted\n");

  delete FiManager;
  qDebug("FiManager deleted\n");

  delete AppManager;
  qDebug("AppManager deleted\n");
}


//================================== GUI =================================


void KNodeApp::setStatusMsg(const QString& text, int id)
{
  statusBar()->clear();
  if (text.isEmpty() && (id==SB_MAIN))
    statusBar()->changeItem(i18n(" Ready"),SB_MAIN);
  else
    statusBar()->changeItem(text, id);
  kapp->processEvents();
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



void KNodeApp::blockEvents()
{
  kapp->installEventFilter(this);
  setCursorBusy(true);
}



void KNodeApp::unblockEvents()
{
  kapp->removeEventFilter(this);
  setCursorBusy(false);
}



//============================ INIT && UPDATE ============================


void KNodeApp::initView()
{
  KNArticleWidget::readOptions();
  KNViewHeader::loadAll();
  view = new KNodeView(this);
  setCentralWidget(view);

  connect(view->collectionView, SIGNAL(clicked(QListViewItem *)),
    this, SLOT(slotCollectionClicked(QListViewItem *)));

  connect(view->collectionView, SIGNAL(selectionChanged(QListViewItem *)),
    this, SLOT(slotCollectionSelected(QListViewItem *)));

  connect(view->hdrView, SIGNAL(selectionChanged(QListViewItem *)),
    this, SLOT(slotHeaderSelected(QListViewItem *)));

  connect(view->hdrView, SIGNAL(doubleClicked(QListViewItem*)),
    this, SLOT(slotHeaderDoubleClicked(QListViewItem*)));

  connect(view->hdrView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotArticlePopup(QListViewItem*, const QPoint&, int)));

  connect(view->collectionView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotCollectionPopup(QListViewItem*, const QPoint&, int)));
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

  actShowAllHdrs = new KToggleAction(i18n("Show &all headers"), 0 , this, SLOT(slotToggleShowAllHdrs()),
                                     actionCollection(), "view_showAllHdrs");
  actShowAllHdrs->setChecked(KNArticleWidget::fullHeaders());


  actCancel = new KAction(i18n("article","&Cancel"), 0 , this, SLOT(slotCancel()),
                          actionCollection(), "article_cancel");
  actCancel->setEnabled(false);
  actSupersede = new KAction(i18n("S&upersede"), 0 , this, SLOT(slotSupersede()),
                             actionCollection(), "article_supersede");
  actSupersede->setEnabled(false);

  KStdAction::showToolbar(this, SLOT(slotToggleToolBar()), actionCollection());
  KStdAction::showStatusbar(this, SLOT(slotToggleStatusBar()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  KStdAction::preferences(this, SLOT(slotSettings()), actionCollection());

  // add all external actions...
  *actionCollection() += AManager->actions();
  *actionCollection() += FoManager->actions();
  *actionCollection() += GManager->actions();
  *actionCollection() += FAManager->actions();
  *actionCollection() += SAManager->actions();
  *actionCollection() += FiManager->actions();
  *actionCollection() += NAcc->actions();
  *actionCollection() += view->actions();

  createGUI("knodeui.rc",false);
  guiFactory()->addClient(view->artView->part());
  conserveMemory();
}



void KNodeApp::initPopups()
{
  accPopup = static_cast<QPopupMenu *>(factory()->container("account_popup", this));
  groupPopup = static_cast<QPopupMenu *>(factory()->container("group_popup", this));
  folderPopup = static_cast<QPopupMenu *>(factory()->container("folder_popup", this));
  fetchPopup = static_cast<QPopupMenu *>(factory()->container("fetch_popup", this));
  savedPopup = static_cast<QPopupMenu *>(factory()->container("saved_popup", this));
}



void KNodeApp::saveOptions()
{
  saveWindowSize("main", size());
  view->saveOptions();
  FiManager->saveOptions();
  FAManager->saveOptions();
  KNArticleWidget::saveOptions();
  AppManager->saveOptions();

  KConfig *conf=KGlobal::config();
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
  if(toolBar("mainToolBar")->isVisible())
    toolBar("mainToolBar")->hide();
  else
    toolBar("mainToolBar")->show();
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
    guiFactory()->removeClient(view->artView->part());
    createGUI("knodeui.rc",false);
    guiFactory()->addClient(view->artView->part());
    conserveMemory();
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



//==================== VIEW-SLOTS ======================



void KNodeApp::slotCollectionClicked(QListViewItem *it)
{
  if(it && (static_cast<KNCollectionViewItem*>(it)->coll->type()==KNCollection::CTnntpAccount))
    it->setOpen(!it->isOpen());
}



void KNodeApp::slotCollectionSelected(QListViewItem *it)
{
  KNGroup *grp=0;
  KNFolder *fldr=0;
  KNNntpAccount *acc=0;
  view->hdrView->clear();
  kapp->processEvents();

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
    }
    view->artView->showBlankPage();
  }

  AManager->setCurrentAccount(acc);
  GManager->setCurrentGroup(grp);
  FoManager->setCurrentFolder(fldr);
  actCancel->setEnabled(false);
  actSupersede->setEnabled(false);
}



void KNodeApp::slotHeaderSelected(QListViewItem *it)
{
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
  actCancel->setEnabled(true);
  actSupersede->setEnabled(true);
}



void KNodeApp::slotHeaderDoubleClicked(QListViewItem *it)
{
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
  if (it) {
    if (static_cast<KNHdrViewItem*>(it)->art->type()==KNArticleBase::ATfetch)
      fetchPopup->popup(p);
    else
      savedPopup->popup(p);
  }
}



void KNodeApp::slotCollectionPopup(QListViewItem *it, const QPoint &p, int c)
{
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

  saveOptions();

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
  if (!SAManager->closeComposeWindows())
    return false;
  cleanup();
  return true;
}



void KNodeApp::jobDone(KNJobData *j)
{
  if(!j) return;
  //qDebug("KNodeApp::jobDone() : job received"); too verbose

  switch(j->type()) {
    case KNJobData::JTLoadGroups:
    case KNJobData::JTFetchGroups:
    case KNJobData::JTCheckNewGroups:
    case KNJobData::JTfetchNewHeaders:
      qDebug("KNodeApp::jobDone() : job sent to GManager");
      GManager->jobDone(j);
    break;
    case KNJobData::JTfetchArticle:
      qDebug("KNodeApp::jobDone() : job sent to FAManager");
      FAManager->jobDone(j);
    break;
    case KNJobData::JTpostArticle:
    case KNJobData::JTmail:
      qDebug("KNodeApp::jobDone() : job sent to SAManager");
      SAManager->jobDone(j);
    break;
  };
}



void KNodeApp::paletteChange( const QPalette & )
{
  progBar->setPalette(palette());    // should be called automatically?
}




bool KNodeApp::eventFilter(QObject* o, QEvent *e)
{
/*  if( (e->type()>1 && e->type()<10)  || //mouse and key events
       e->type()==30 || //accel events
       e->type()==31 // wheel events
    )
    return true;
*/

  return KMainWindow::eventFilter( o, e );
}



//--------------------------------

#include "knode.moc"
