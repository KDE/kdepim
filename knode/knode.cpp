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

#include <kmessagebox.h>
#include <kstdaction.h>
#include <kaccel.h>
#include <kurl.h>
#include <klocale.h>
#include <khtml_part.h>

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
#include "knfolder.h"
#include "kngroup.h"
#include "utilities.h"
#include "resource.h"
#include "knglobals.h"
#include "knode.h"
#include "knarticle.h"


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
  : setDialog()
{
  knGlobals.top=this;
  kapp->setMainWidget(this);  // this makes the external viewer windows close on shutdown...

  //init the GUI
  setPlainCaption("KNode " KNODE_VERSION);
  initView();
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

	restoreWindowSize("main", this, QSize(600,400));

  // set the keyboard focus indicator on the first item in the collectionView
	if(view->collectionView->firstChild())
	  view->collectionView->setCurrentItem(view->collectionView->firstChild());
	view->collectionView->setFocus();
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
  setView(view);

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
  actSupersede = new KAction(i18n("&Supersede"), 0 , this, SLOT(slotSupersede()),
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


/*  old debug code...
#ifdef TEST
#include "kncomposer.h"
#include "knfilterconfigwidget.h"
#include "knsearchdialog.h"ü
#include "knpurgeprogressdialog.h"
#endif

#ifdef TEST
  test_menu=new QPopupMenu();
  test_menu->insertItem("Composer", 10);
  test_menu->insertItem("compact list", 20);
  test_menu->insertItem("filter config", 30);
  test_menu->insertItem("search", 40);
  test_menu->insertItem("settings", 50);
  test_menu->insertItem("purge progress", 60);
  file_menu->insertItem("Test", test_menu);
  connect(test_menu, SIGNAL(activated (int)), SLOT(slotMainCallback (int)));
#endif

#ifdef TEST
	KNComposer *composer;
	KNSavedArticle *sart;
	KNFilterConfigWidget *fconf;
	KNSearchDialog *sdl;
	KNSettingsDialog *set;
	KNPurgeProgressDialog *ppdlg;

		case 	10:
			\*sart=new KNSavedArticle();
			sart->setStatus(KNArticleBase::AStoPost);
			sart->setDestination("abc,def,ghi");
			composer=new KNComposer(sart);
			composer->show(); *\
		break;
		case 	20:
			GManager->currentGroup()->compactList();
		break;
		case 30:
			fconf=new KNFilterConfigWidget();
			fconf->show();
		break;
		case 40:
			sdl=new KNSearchDialog();
			sdl->show();
		break;
		case 50:
			set=new KNSettingsDialog();
			set->exec();
			delete set;
		break;
		case 60:
			ppdlg=new KNPurgeProgressDialog();
			ppdlg->init("Deleting expired articles ...", 10);
			ppdlg->setInfo("Group : de.alt.comp.kde");
			ppdlg->show();
		break;
#endif

*/


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
  KNArticleWidget::saveOptions();
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
  #warning FIXME: stub  (open conf dialog and show keyboard config widget)
}



void KNodeApp::slotConfToolbar()
{
  #warning FIXME: stub  (open conf dialog and show toolbar config widget)
}



void KNodeApp::slotSettings()
{
  if (!setDialog) {
    setDialog = new KNSettingsDialog(this);
    connect(setDialog, SIGNAL(finished()), this, SLOT(slotSettingsFinished()));
  }
  setDialog->show();
}



void KNodeApp::slotSettingsFinished()
{
  setDialog->delayedDestruct();
  setDialog=0;
}



//==================== VIEW-SLOTS ======================


void KNodeApp::slotCollectionSelected(QListViewItem *it)
{
	KNGroup *grp=0;
	KNFolder *fldr=0;
	KNNntpAccount *acc=0;
	view->hdrView->clear();
	kapp->processEvents();

	if(it) {
		if(((KNCollectionViewItem*)it)->coll->type()==KNCollection::CTgroup) {
  		if (!(view->hdrView->hasFocus())&&!(view->artView->hasFocus()))
	     	view->hdrView->setFocus();
			grp=(KNGroup*)((KNCollectionViewItem*)it)->coll;
			acc=(KNNntpAccount*)grp->account();
		}
		else if(((KNCollectionViewItem*)it)->coll->type()==KNCollection::CTfolder) {
   		if (!(view->hdrView->hasFocus())&&!(view->artView->hasFocus()))
	     	view->hdrView->setFocus();
			fldr=(KNFolder*)((KNCollectionViewItem*)it)->coll;
			setStatusMsg(QString::null, SB_FILTER);
		}
		else if(((KNCollectionViewItem*)it)->coll->type()==KNCollection::CTnntpAccount) {
		  it->setOpen(true);
			acc=(KNNntpAccount*)((KNCollectionViewItem*)it)->coll;
		  setStatusMsg(QString::null, SB_GROUP);
  	  setStatusMsg(QString::null, SB_FILTER);
    	setCaption(QString::null);
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
		if(((KNHdrViewItem*)it)->art->type()==KNArticleBase::ATfetch)
			fart=(KNFetchArticle*)((KNHdrViewItem*)it)->art;
		else sart=(KNSavedArticle*)((KNHdrViewItem*)it)->art;
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
		if(((KNHdrViewItem*)it)->art->type()==KNArticleBase::ATfetch) {
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



bool KNodeApp::eventFilter(QObject* o, QEvent *e)
{
/*  if( (e->type()>1 && e->type()<10)  || //mouse and key events
       e->type()==30 || //accel events
       e->type()==31 // wheel events
    )
    return true;
*/

  return KTMainWindow::eventFilter( o, e );
}



//--------------------------------

#include "knode.moc"
