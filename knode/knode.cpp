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

#include <kmessagebox.h>
#include <kstdaction.h>

#include "knsettingsdialog.h"
#include "knhdrviewitem.h"
#include "kncollectionviewitem.h"
#include "knviewheader.h"
#include "knsavedarticlemanager.h"
#include "knfolder.h"
#include "utilities.h"
#include "resource.h"
#include "knode.h"

KNodeApp* xTop;
KNNetAccess *xNet;


//========================================================================
//=============================== KNPROGRESS =============================
//========================================================================


KNProgress::KNProgress (int desiredHeight, int minValue, int maxValue, int value, KProgress::Orientation orient, QWidget *parent=0, const char *name=0)
: KProgress(minValue,maxValue,value,orient,parent,name), desHeight(desiredHeight)
{}



KNProgress::~KNProgress()
{}



QSize KNProgress::sizeHint() const
{
  return QSize(KProgress::sizeHint().width(),desHeight);
}



//========================================================================
//=============================== KNODEAPP ===============================
//========================================================================


KNodeApp::KNodeApp()
{
  xTop=this;

  //init the GUI
  setPlainCaption("KNode " VERSION);

  initView();	
  initStatusBar();
  initActions();
  initPopups();

  //init Net
	NAcc=new KNNetAccess();
	xNet=NAcc;
			
	//init Filter Manager
	FiManager=new KNFilterManager(actSetFilters);
	
	//init Fetch-Article Manager
	FAManager=new KNFetchArticleManager(view->hdrView);
	connect(FiManager, SIGNAL(filterChanged(KNArticleFilter*)),
		FAManager, SLOT(slotFilterChanged(KNArticleFilter*)));	
  actSetShowThreads->setChecked(FAManager->threaded());
			
  //init Group Manager
	GManager=new KNGroupManager(FAManager);
	
	//init Account Manager
	AManager=new KNAccountManager(GManager, view->collectionView);
	
	//init Saved-Article Manager
	SAManager=new KNSavedArticleManager(view->hdrView, AManager);
	
	//init Folder Manager
	FoManager=new KNFolderManager(SAManager, view->collectionView);
	
	//read options
	readOptions();

  netIsActive(false);
  disableProgressBar();
  KNLVItemBase::initIcons();

  // set the keyboard focus indicator on the first item in the collectionView
	if(view->collectionView->firstChild())
	  view->collectionView->setCurrentItem(view->collectionView->firstChild());
	view->collectionView->setFocus();
}



KNodeApp::~KNodeApp()
{
 	KNLVItemBase::clearIcons();
 	
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



void KNodeApp::netIsActive(bool b)
{
	static bool status=true;
	if(status!=b) {
  	actNetStop->setEnabled(b);
		status=b;
	}	
}



// 0% and no text
void KNodeApp::disableProgressBar()
{
	progr=0;
	progBar->setFormat(QString::null);
	progBar->setValue(0);
}



// manual operation
void KNodeApp::setProgressBar(int value,const QString& text)
{
	progBar->setFormat(text);
	if (value>1000)
	  value = 1000;
	progBar->setValue(value);
}



// display 0%
void KNodeApp::initProgressBar()
{
	progr=0;
	progBar->setFormat("%p%");
	progBar->setValue(1);
}
								


// add 10%
void KNodeApp::stepProgressBar()
{
	progr+=100;
	if(progr>=1000) progr=1000;
	progBar->setValue(progr);
}
											


// display 100%
void KNodeApp::fullProgressBar()
{
	progBar->setValue(1000);
}



void KNodeApp::accountSelected(bool b)
{
	static bool status=true;
	
	if(status!=b) {
	  actAccProperties->setEnabled(b);
	  actAccSubscribeGrps->setEnabled(b);
	  actAccLoadHdrs->setEnabled(b);
	  actAccDelete->setEnabled(b);
    actArtPostNew->setEnabled(b);
		status=b;
	}
}



void KNodeApp::groupSelected(bool b)
{
	static bool status=true;
		
	if(status!=b) {
  	actGrpProperties->setEnabled(b);
  	actGrpUnsubscribe->setEnabled(b);
  	actSetExpandAll->setEnabled(b);
  	actSetCollapseAll->setEnabled(b);
		status=b;
	}
}



void KNodeApp::groupDisplayed(bool b)
{
	static bool status=true;

	if(status!=b) {
  	actGrpLoadHdrs->setEnabled(b);
  	actGrpExpire->setEnabled(b);
  	actGrpResort->setEnabled(b);
  	actGrpAllRead->setEnabled(b);
    actGrpAllUnread->setEnabled(b);
    actArtSearch->setEnabled(b);
		status=b;
	}	
}



void KNodeApp::fetchArticleSelected(bool b)
{
	static bool status=true;
	
	if(status!=b) {
	  actArtOwnWindow->setEnabled(b);
  	actArtRead->setEnabled(b);
  	actArtUnread->setEnabled(b);
	  actThreadRead->setEnabled(b);
	  actThreadUnread->setEnabled(b);
	  actThreadSetScore->setEnabled(b);
	  actThreadWatch->setEnabled(b);
	  actThreadIgnore->setEnabled(b);
	  actThreadToggle->setEnabled(b);
		status=b;
	}
}



void KNodeApp::fetchArticleDisplayed(bool b)
{
	static bool status=true;
	
	if(status!=b) {
    messageDisplayed(b);
    actArtPostReply->setEnabled(b);
    actArtMailReply->setEnabled(b);
    actArtForward->setEnabled(b);
		status=b;
	}
}



void KNodeApp::folderSelected(bool b)
{
	static bool status=true;

	if(status!=b) {
	  actFolderCompact->setEnabled(b);
	  actFolderEmpty->setEnabled(b);
		status=b;
	}	
}



void KNodeApp::folderDisplayed(bool b)
{
	/*static bool status=true;
	static int menus[]=	{	ID_SEARCH, -1 };
	if(status!=b) {
		updateMenus(menus, b);
		status=b;
	}*/	
}



void KNodeApp::savedArticleSelected(bool b)
{
	static bool status=true;

	if(status!=b) {
  	actArtDelete->setEnabled(b);
    actArtSendNow->setEnabled(b);
    actArtSendLater->setEnabled(b);
    actArtEdit->setEnabled(b);		
		status=b;
	}	
}



void KNodeApp::savedArticleDisplayed(bool b)
{
	static bool status=true;

	if(status!=b) {
  	actArtCancel->setEnabled(b);
		status=b;
	}	
}


void KNodeApp::messageDisplayed(bool b)
{
  static bool status=true;

  if(status!=b) {
   	actFileSave->setEnabled(b);
  	actFilePrint->setEnabled(b);
  	actEditFind->setEnabled(b);
  	actEditFindNext->setEnabled(b);
    status=b;
  }
}


//============================ INIT && UPDATE ============================


void KNodeApp::initView()
{
  KNArticleWidget::readConfig();
  KNViewHeader::loadAll();
  view = new KNodeView(this);
  setView(view);

  connect(view->collectionView, SIGNAL(selectionChanged(QListViewItem *)),
  	this, SLOT(slotCollectionSelected(QListViewItem *)));
    	
  connect(view->hdrView, SIGNAL(selectionChanged(QListViewItem *)),
  	this, SLOT(slotHeaderSelected(QListViewItem *)));
  	
  connect(view->hdrView, SIGNAL(doubleClicked(QListViewItem*)),
  	this, SLOT(slotHeaderDoubleClicked(QListViewItem*)));
    	
  connect(view->hdrView, SIGNAL(sortingChanged(int)),
  	this, SLOT(slotSortingChanged(int)));
  	
  connect(view->hdrView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
  	this, SLOT(slotArticlePopup(QListViewItem*, const QPoint&, int)));

  connect(view->collectionView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
  	this, SLOT(slotCollectionPopup(QListViewItem*, const QPoint&, int)));	
}


void KNodeApp::initStatusBar()
{
	KStatusBar *sb=statusBar();
	
	progBar = new KNProgress(sb->sizeHint().height()-4, 0, 1000, 0, KProgress::Horizontal, sb );
	progBar->setFixedWidth(110);
  progBar->setFrameStyle(QFrame::Box | QFrame::Raised);
  progBar->setLineWidth(1);
  progBar->setBackgroundMode(QWidget::PaletteBackground);
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
  // file menu
  actFileSave = KStdAction::save(this, SLOT(slotFileSave()),actionCollection());
  actFilePrint = KStdAction::print(this, SLOT(slotFilePrint()),actionCollection());
  actNetSendPending = new KAction(i18n("Sen&d pending messages"), 0, this, SLOT(slotNetSendPending()),
                                  actionCollection(), "net_sendPending");
  actNetStop = new KAction(i18n("Stop &Network"),"cancel",0, this, SLOT(slotNetStop()),
                           actionCollection(), "net_stop");
  KStdAction::quit(this, SLOT(slotFileQuit()),actionCollection());

  // edit menu
  actEditCopy = KStdAction::copy(this, SLOT(slotEditCopy()),actionCollection());
  actEditFind = KStdAction::find(this, SLOT(slotEditFind()),actionCollection());
  actEditFindNext = KStdAction::findNext(this, SLOT(slotEditFindNext()),actionCollection());

  // account menu
  actAccProperties = new KAction(i18n("&Properties..."), 0, this, SLOT(slotAccProperties()),
                                 actionCollection(), "account_properties");
  actAccSubscribeGrps = new KAction(i18n("&Subscribe to Newsgroups..."),"grpdlg", 0, this, SLOT(slotAccSubscribeGrps()),
                                    actionCollection(), "account_subscribe");
  actAccLoadHdrs = new KAction(i18n("&Get New Articles"), "dlall", 0, this, SLOT(slotAccLoadHdrs()),
                                    actionCollection(), "account_dnlHeaders");
  actAccDelete = new KAction(i18n("&Delete"), 0, this, SLOT(slotAccDelete()),
                                    actionCollection(), "account_delete");

  // group menu
  actGrpProperties = new KAction(i18n("&Properties..."), 0, this, SLOT(slotGrpProperties()),
                                 actionCollection(), "group_properties");
  actGrpLoadHdrs = new KAction(i18n("&Get New Articles"), 0, this, SLOT(slotGrpLoadHdrs()),
                               actionCollection(), "group_dnlHeaders");
  actGrpExpire = new KAction(i18n("E&xpire Now"), 0, this, SLOT(slotGrpExpire()),
                             actionCollection(), "group_expire");
  actGrpResort = new KAction(i18n("Res&ort"), 0, this, SLOT(slotGrpResort()),
                             actionCollection(), "group_resort");
  actGrpAllRead = new KAction(i18n("Mark all as &read"), 0, this, SLOT(slotGrpAllRead()),
                              actionCollection(), "group_allRead");
  actGrpAllUnread = new KAction(i18n("Mark all as u&nread"), 0, this, SLOT(slotGrpAllUnread()),
                                actionCollection(), "group_allUnread");
  actGrpUnsubscribe = new KAction(i18n("&Unsubscribe"), 0, this, SLOT(slotGrpUnsubscribe()),
                                  actionCollection(), "group_unsubscribe");
  actFolderCompact = new KAction(i18n("&Compact Folder"), 0, this, SLOT(slotFolderCompact()),
                                 actionCollection(), "folder_compact");
  actFolderEmpty = new KAction(i18n("&Empty Folder"), 0, this, SLOT(slotFolderEmpty()),
                               actionCollection(), "folder_empty");

  // article menu
  actArtPostNew = new KAction(i18n("&Post new article"), "newmsg", Key_P , this, SLOT(slotArtNew()),
                              actionCollection(), "article_postNew");
  actArtPostReply = new KAction(i18n("Post &reply"),"reply", Key_R , this, SLOT(slotArtReply()),
                                actionCollection(), "article_postReply");
  actArtMailReply = new KAction(i18n("&Mail reply"),"remail", Key_A , this, SLOT(slotArtRemail()),
                                actionCollection(), "article_mailReply");
  actArtForward = new KAction(i18n("&Forward"),"fwd", Key_F , this, SLOT(slotArtForward()),
                              actionCollection(), "article_forward");
  actArtRead = new KAction(i18n("Mark as &read"), Key_D , this, SLOT(slotArtMarkRead()),
                           actionCollection(), "article_read");
  actArtUnread = new KAction(i18n("Mark as &unread"), Key_U , this, SLOT(slotArtMarkUnread()),
                             actionCollection(), "article_unread");
  actThreadRead = new KAction(i18n("Mark as &read"), ALT+Key_D , this, SLOT(slotArtThrRead()),
                              actionCollection(), "thread_read");
  actThreadUnread = new KAction(i18n("Mark as &unread"), ALT+Key_U , this, SLOT(slotArtThrUnread()),
                                actionCollection(), "thread_unread");
  actThreadSetScore = new KAction(i18n("Set &score"), Key_S , this, SLOT(slotArtThrScore()),
                                  actionCollection(), "thread_setScore");
  actThreadWatch = new KAction(i18n("&Watch"), Key_W , this, SLOT(slotArtThrWatch()),
                               actionCollection(), "thread_watch");
  actThreadIgnore = new KAction(i18n("&Ignore"), Key_I , this, SLOT(slotArtThrIgnore()),
                                actionCollection(), "thread_ignore");
  actThreadToggle = new KAction(i18n("&Toggle Subthread"), Key_T , this, SLOT(slotArtThrToggle()),
                                actionCollection(), "thread_toggle");
  actArtOwnWindow = new KAction(i18n("&Open in own window"), Key_O , this, SLOT(slotArtOwnWindow()),
                                actionCollection(), "article_ownWindow");
  actArtEdit = new KAction(i18n("&Edit"), Key_E , this, SLOT(slotArtEdit()),
                           actionCollection(), "article_edit");
  actArtDelete = new KAction(i18n("&Delete"), Key_Delete , this, SLOT(slotArtDelete()),
                             actionCollection(), "article_delete");
  actArtCancel = new KAction(i18n("&Cancel post"), 0 , this, SLOT(slotArtCancel()),
                             actionCollection(), "article_cancel");
  actArtSendNow = new KAction(i18n("Send &now"), 0 , this, SLOT(slotArtSendNow()),
                              actionCollection(), "article_sendNow");
  actArtSendLater = new KAction(i18n("Send &later"), 0 , this, SLOT(slotArtSendLater()),
                                actionCollection(), "article_sendLater");
  actArtSearch = new KAction(i18n("&Search"),"search" , Key_F4 , this, SLOT(slotArtSearch()),
                             actionCollection(), "article_search");

  // go menu
  new KAction(i18n("&Next article"), Key_N , this, SLOT(slotGotoNextArt()),
              actionCollection(), "go_nextArticle");
  new KAction(i18n("&Previous article"), Key_B , this, SLOT(slotGotoPrevArt()),
              actionCollection(), "go_prevArticle");
  new KAction(i18n("Next unread &article"),"nextart", ALT+Key_Space , this, SLOT(slotGotoNextUnreadArt()),
              actionCollection(), "go_nextUnreadArticle");
  new KAction(i18n("Next unread &thread"),"nextthr", CTRL+Key_Space , this, SLOT(slotGotoNextThr()),
              actionCollection(), "go_nextUnreadThread");
  new KAction(i18n("Ne&xt group"), Key_Plus , this, SLOT(slotGotoNextGroup()),
              actionCollection(), "go_nextGroup");
  new KAction(i18n("Pre&vious group"), Key_Minus , this, SLOT(slotGotoPrevGroup()),
              actionCollection(), "go_prevGroup");
  new KAction(i18n("Read &through articles"), Key_Space , this, SLOT(slotReadThrough()),
              actionCollection(), "go_readThrough");

  // settings menu
  KStdAction::showToolbar(this, SLOT(slotToggleToolBar()), actionCollection());
  KStdAction::showStatusbar(this, SLOT(slotToggleStatusBar()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  KStdAction::preferences(this, SLOT(slotSettings()), actionCollection());
  actSetShowThreads = new KToggleAction(i18n("Show th&reads"), 0 , this, SLOT(slotToggleShowThreads()),
                                        actionCollection(), "settings_showThreads");
  actSetExpandAll = new KAction(i18n("&Expand all threads"), 0 , this, SLOT(slotViewExpand()),
              actionCollection(), "settings_ExpandAll");
  actSetCollapseAll = new KAction(i18n("&Collapse all threads"), 0 , this, SLOT(slotViewCollapse()),
              actionCollection(), "settings_CollapseAll");
  new KAction(i18n("&Refresh List"),"refresh", Key_F5 , this, SLOT(slotViewRefresh()),
              actionCollection(), "settings_CollapseAll");
  actSetShowAllHdrs = new KToggleAction(i18n("Show &all headers"), 0 , this, SLOT(slotToggleShowAllHdrs()),
                                        actionCollection(), "settings_showAllHdrs");
  actSetShowAllHdrs->setChecked(KNArticleWidget::fullHeaders());
  actSetFilters = new KSelectAction(i18n("&Filter"), "filter", 0 , actionCollection(), "settings_Filter");
  actSetSort = new KSelectAction(i18n("&Sort"), 0 , actionCollection(), "settings_Sort");
  actSetSort->popupMenu()->insertItem(i18n("By &Subject"), 0);
  actSetSort->popupMenu()->insertItem(i18n("By S&ender"), 1);
  actSetSort->popupMenu()->insertItem(i18n("By S&core"), 2);
  actSetSort->popupMenu()->insertItem(i18n("By &Date"), 3);
  connect(actSetSort, SIGNAL(activated (int)), this, SLOT(slotViewSort (int)));

  createGUI( "knodeui.rc" );
}


/*  old debug code...
#ifdef TEST
#include "kncomposer.h"
#include "knfilterconfigwidget.h"
#include "knsearchdialog.h"
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

/*
	QString aboutText=QString("KNode %1\
\n\nhttp://knode.sourceforge.net/\
\n\n(c) 1999-2000 Christian Thurner\
\n\nAuthors:\
\n  Christian Thurner <cthurner@freepage.de>\
\n  Christian Gebauer <gebauer@bigfoot.com>\
\n  Dirk Mueller <mueller@kde.org>\
\n\nThis is free software covered by the GPL.").arg(VERSION);
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
	QStrList lst;
	lst.setAutoDelete(true);
  KConfig *conf=CONF();
 	int id=-1;
     	
  conf->setGroup("APPEARANCE");

  //main window
  conf->writeEntry("WSize", this->size());

  //view
  QValueList<int> vert,horz;
  view->sepPos(vert,horz);
  conf->writeEntry("Vert_SepPos",vert);
  conf->writeEntry("Horz_SepPos",horz);
  view->headersSize(&lst);
  conf->writeEntry("Hdrs_Size", lst);
  conf->writeEntry("sortCol", view->hdrView->sortColumn());
  conf->writeEntry("sortAscending", view->hdrView->ascending());

  conf->setGroup("READNEWS");

  KNArticleFilter *f=FiManager->currentFilter();

  if(f) id=f->id();

  conf->writeEntry("lastFilterID", id);
  conf->writeEntry("fullHdrs", KNArticleWidget::fullHeaders());
}



void KNodeApp::readOptions()
{
  QStrList lst;
  lst.setAutoDelete(true);
  	
  KConfig *conf=CONF();   	
  conf->setGroup("APPEARANCE");
  	
  //main window
  QSize s(600,400);
  this->resize(conf->readSizeEntry("WSize", &s));
  	   	
  //view
  view->setSepPos(conf->readIntListEntry("Vert_SepPos"), conf->readIntListEntry("Horz_SepPos"));

  conf->readListEntry("Hdrs_Size", lst);
  if(lst.count()>0) view->setHeadersSize(&lst);
  int sortCol=conf->readNumEntry("sortCol",0);
  bool asc=conf->readBoolEntry("sortAscending", true);
  view->hdrView->setColAsc(sortCol, asc);
  view->hdrView->setSorting(sortCol, asc);	
  actSetSort->setCurrentItem(sortCol);
  	  	
  conf->setGroup("READNEWS");
  FiManager->setFilter(conf->readNumEntry("lastFilterID", 1));
  FAManager->slotFilterChanged(FiManager->currentFilter());	
}


//================================ SLOTS =================================

//======== FILE MENU =================

void KNodeApp::slotFileSave()
{
	if(FAManager->hasCurrentArticle())
		KNArticleManager::saveArticleToFile(FAManager->currentArticle());
		
	if(SAManager->hasCurrentArticle())
		KNArticleManager::saveArticleToFile(SAManager->currentArticle());	
}


void KNodeApp::slotFilePrint()
{
  #warning FIXME print not implemented
  //	view->artView->print();
}


void KNodeApp::slotNetSendPending()
{
  SAManager->sendOutbox();
}
 	

void KNodeApp::slotNetStop()
{
  NAcc->cancelAllJobs();
}

  	
void KNodeApp::slotFileQuit()
{
  cleanup();
  kapp->quit();
}

	
//======== EDIT MENU =================  	


void KNodeApp::slotEditCopy()
{
  #warning FIXME: stub  (copy current selection to the clipboard)
}

	
void KNodeApp::slotEditFind()
{
  #warning FIXME: stub  (initiate search in the html widget)
}
  	

void KNodeApp::slotEditFindNext()
{
  #warning FIXME: stub  (continue search in the html widget)
}

 	
//======== ACCOUNT MENU =================  	
  	  	
  	
void KNodeApp::slotAccProperties()
{
  #warning FIXME: stub (open conf dialog and show account properties)
}
  	

void KNodeApp::slotAccSubscribeGrps()
{
	view->artView->showBlankPage();
	GManager->showGroupDialog(AManager->currentAccount());
}

 	
void KNodeApp::slotAccLoadHdrs()
{
  GManager->checkAll(AManager->currentAccount());
}


void KNodeApp::slotAccDelete()
{
  #warning FIXME: stub (confirmation question, delete account)
}

 	
//======== GROUP MENU =================  	

  	
void KNodeApp::slotGrpProperties()
{
  GManager->showGroupProperties();
}

 	
void KNodeApp::slotGrpLoadHdrs()
{
  GManager->checkGroupForNewHeaders();
}

	
void KNodeApp::slotGrpExpire()
{
  GManager->expireGroupNow();
}
  	

void KNodeApp::slotGrpResort()
{
  GManager->resortGroup();
}

   	
void KNodeApp::slotGrpAllRead()
{
  FAManager->setAllRead(0, true);
}
  	

void KNodeApp::slotGrpAllUnread()
{
	FAManager->setAllRead(0, false);
}
   	

void KNodeApp::slotGrpUnsubscribe()
{
  if(KMessageBox::Yes == KMessageBox::questionYesNo(0, i18n("Do you really want to unsubscribe this group?"))) {
	  view->artView->showBlankPage();
  	GManager->unsubscribeGroup();
	}
}
  	
	
void KNodeApp::slotFolderCompact()
{
  FoManager->compactFolder();
}


void KNodeApp::slotFolderEmpty()
{
  FoManager->emptyFolder();
}

 	
//======== ARTICLE MENU =================  	

		
void KNodeApp::slotArtNew()
{
	if(GManager->hasCurrentGroup()) SAManager->post(GManager->currentGroup());
	else if(AManager->hasCurrentAccount()) SAManager->post(AManager->currentAccount());	
}



void KNodeApp::slotArtReply()
{
	SAManager->reply(FAManager->currentArticle(), GManager->currentGroup());	
}



void KNodeApp::slotArtRemail()
{
	SAManager->reply(FAManager->currentArticle(),0);	
}



void KNodeApp::slotArtForward()
{
	SAManager->forward(FAManager->currentArticle());	
}



void KNodeApp::slotArtOwnWindow()
{
	FAManager->articleWindow();
}



void KNodeApp::slotArtMarkRead()
{
	FAManager->setArticleRead(0, true);
}



void KNodeApp::slotArtMarkUnread()
{
	FAManager->setArticleRead(0, false);
}

		
void KNodeApp::slotArtEdit()
{
	SAManager->editArticle();	
}


void KNodeApp::slotArtDelete()
{
	SAManager->deleteArticle(0, true);	
}
		 	
 	
void KNodeApp::slotArtCancel()
{
  SAManager->cancel();
}
  	

void KNodeApp::slotArtSendNow()
{
  SAManager->sendArticle();
}
  	
  	
void KNodeApp::slotArtSendLater()
{
  SAManager->sendArticle(0, false);
}
  	

void KNodeApp::slotArtSearch()
{
	FAManager->search();
}
  	

void KNodeApp::slotArtThrRead()
{
	FAManager->setThreadRead(0, true);
}


void KNodeApp::slotArtThrUnread()
{
	FAManager->setThreadRead(0, false);
}


void KNodeApp::slotArtThrScore()
{
	FAManager->setThreadScore();
}


void KNodeApp::slotArtThrWatch()
{
	FAManager->toggleWatched();
}


void KNodeApp::slotArtThrIgnore()
{
	FAManager->toggleIgnored();
}


void KNodeApp::slotArtThrToggle()
{
	view->toggleThread();
}

 	  	
//======== GO MENU =================  	


void KNodeApp::slotGotoNextArt()
{
	view->nextArticle();
}


void KNodeApp::slotGotoPrevArt()
{
	view->prevArticle();
}


void KNodeApp::slotGotoNextUnreadArt()
{
	view->nextUnreadArticle();
}


void KNodeApp::slotReadThrough()
{
	view->readThrough();
}


void KNodeApp::slotGotoNextThr()
{
	view->nextUnreadThread();
}


void KNodeApp::slotGotoNextGroup()
{
	view->nextGroup();
}


void KNodeApp::slotGotoPrevGroup()
{
	view->prevGroup();
}

  	  	
//======== SETTINGS MENU =================
  	
  	
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


void KNodeApp::slotToggleShowThreads()
{
  FAManager->toggleThreaded();
}

  	
void KNodeApp::slotToggleShowAllHdrs()
{
  KNArticleWidget::toggleFullHeaders();
}


void KNodeApp::slotViewSort(int id)
{
	view->hdrView->slotSortList(id);
}


void KNodeApp::slotViewRefresh()
{
	FAManager->showHdrs();
}

  	
void KNodeApp::slotViewExpand()
{
  FAManager->expandAllThreads(true);
}


void KNodeApp::slotViewCollapse()
{
  FAManager->expandAllThreads(false);
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
	KNSettingsDialog *sdlg=new KNSettingsDialog();
	if(sdlg->exec()) {
	  sdlg->apply();
	  AManager->readConfig();
	  SAManager->readConfig();
		GManager->readConfig();
		FAManager->readConfig();
		KNArticleWidget::readConfig();
		KNArticleWidget::updateInstances();
	}		
	delete sdlg;
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
			xTop->setStatusMsg(QString::null, SB_FILTER);
		}
		else if(((KNCollectionViewItem*)it)->coll->type()==KNCollection::CTnntpAccount) {
		  it->setOpen(true);
			acc=(KNNntpAccount*)((KNCollectionViewItem*)it)->coll;
		  xTop->setStatusMsg(QString::null, SB_GROUP);
  	  xTop->setStatusMsg(QString::null, SB_FILTER);
    	xTop->setCaption(QString::null);
		}
		view->artView->showBlankPage();
	}
	
	AManager->setCurrentAccount(acc);
	GManager->setCurrentGroup(grp);
	FoManager->setCurrentFolder(fldr);
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



void KNodeApp::slotSortingChanged(int sortCol)
{
  actSetSort->setCurrentItem(sortCol);
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



//========================================================================
//================================ OTHERS ================================
//========================================================================


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



bool KNodeApp::queryExit()
{
	cleanup();	
	return true;
}



void KNodeApp::jobDone(KNJobData *j)
{
	if(!j) return;
	//qDebug("KNodeApp::jobDone() : job received"); too verbose

	switch(j->type()) {
		case KNJobData::JTlistGroups:
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
