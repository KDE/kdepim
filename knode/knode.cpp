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

//#define TEST

#include <stdio.h>
#include <qheader.h>
#include <qdir.h>
#include <qlabel.h>

#include <kmessagebox.h>
#include <kmenubar.h>
#include <kkeydialog.h>

#include "knsettingsdialog.h"
#include "knhdrviewitem.h"
#include "kncollectionviewitem.h"
#include "knviewheader.h"
#include "knsavedarticlemanager.h"
#include "knfolder.h"
#include "utilities.h"
#include "resource.h"
#include "menu_ids.h"
#include "knode.h"

#ifdef TEST
#include "kncomposer.h"
#include "knfilterconfigwidget.h"
#include "knsearchdialog.h"
#include "knpurgeprogressdialog.h"
#endif


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

	//init Filter Manager
	FiManager=new KNFilterManager();

  //init the GUI
  setPlainCaption("KNode " VERSION);

  initView();	
  initMenuBar();
  initPopups();
  initToolBar();
  initStatusBar();
  initAccel();

  //init Net
	NAcc=new KNNetAccess();
	xNet=NAcc;
			
	//init Fetch-Article Manager
	FAManager=new KNFetchArticleManager(view->hdrView);
	connect(FiManager, SIGNAL(filterChanged(KNArticleFilter*)),
		FAManager, SLOT(slotFilterChanged(KNArticleFilter*)));	
	view_menu->setItemChecked(VIEW_SHOW_THR, FAManager->threaded());		
		
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
	if (!bViewToolbar) enableToolBar(KToolBar::Hide,0);
  if (!bViewStatusbar) enableStatusBar(KStatusBar::Hide);

//  menuBar()->setMenuBarPos(menu_bar_pos);
//  tb0->setBarPos(tool_bar_pos);

  view_menu->setItemChecked(VIEW_SHOW_STATB,bViewStatusbar);
  view_menu->setItemChecked(VIEW_SHOW_TOOLB0,bViewToolbar);

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
 	delete accPopup;
 	delete groupPopup;
	delete fetchPopup;
	delete folderPopup;
	delete savedPopup;
	delete acc;
 	
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




//========================================================================
//================================== GUI =================================
//========================================================================


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
		status=b;
		toolBar()->setItemEnabled(FILE_CANCEL, b);
		file_menu->setItemEnabled(FILE_CANCEL, b);
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
	static int menus[]=			{ GROUP_SUBSCRIBE, ART_POST, FILE_DLACC, -1 };
	static const char *accels[]=	{ "post", 0 };
	
	if(status!=b) {
		updateMenus(menus, b);
		updateAccels(accels, b);
		status=b;
	}
}



void KNodeApp::groupSelected(bool b)
{
	static bool status=true;
	static int menus[]=			{	GROUP_PROP, GROUP_UNSUBSCRIBE,
														VIEW_EXP_ALL, VIEW_COL_ALL, -1 };
		
	if(status!=b) {
		updateMenus(menus, b);
		status=b;
	}
}



void KNodeApp::groupDisplayed(bool b)
{
	static bool status=true;
	static int menus[]=			{	GROUP_DL_HDRS, GROUP_EXPNOW, GROUP_RESORT,
														GROUP_ALL_READ, GROUP_ALL_UNREAD,
														ART_SEARCH, -1 };
  static const char *accels[]=	{ "search", 0 };
	if(status!=b) {
		updateMenus(menus, b);
		updateAccels(accels, b);
		status=b;
	}	
}



void KNodeApp::fetchArticleSelected(bool b)
{
	static bool status=true;
	static int menus[]=			{ ART_OWINDOW, ART_MARK, ART_THREAD, -1 };
	static const char *accels[]=	{ "openwin", "mread", "muread", "mtread", "mturead",
														"score", "watch", "ignore", "togglethr", 0 };
	
	if(status!=b) {
		updateMenus(menus, b);
		updateAccels(accels, b);
		status=b;
	}
}



void KNodeApp::fetchArticleDisplayed(bool b)
{
	static bool status=true;
	static int menus[]=			{	FILE_SAVEAS	, ART_POST_REPL, ART_MAIL_REPL,
														ART_FORWARD, -1 };
	static const char *accels[]=	{ "reply", "remail", "forwd", 0 };
	
	if(status!=b) {
		updateMenus(menus, b);
		updateAccels(accels, b);
		status=b;
	}
}



void KNodeApp::folderSelected(bool b)
{
	static bool status=true;
	static int menus[]=	{	GROUP_FOLDERS, -1 };
	if(status!=b) {
		updateMenus(menus, b);
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
	static int menus[]=	{	ART_DEL, ART_SEND_NOW, ART_SEND_LAT,
												ART_EDIT, -1 };
	if(status!=b) {
		updateMenus(menus, b);
		status=b;
	}	
}



void KNodeApp::savedArticleDisplayed(bool b)
{
	static bool status=true;
	static int 	menus[]=		{	FILE_SAVEAS, ART_CANCEL, -1 };
	if(status!=b) {
		updateMenus(menus, b);
		status=b;
	}	
}



//========================================================================
//============================ INIT && UPDATE ============================
//========================================================================

void KNodeApp::initMenuBar()
{
  //file
  file_menu = new QPopupMenu();
  file_menu->insertItem(i18n("&Save as.."), FILE_SAVEAS);
 	file_menu->insertItem(i18n("&Print"), FILE_PRINT);
  file_menu->insertSeparator();
  file_menu->insertItem(i18n("&Download account"), FILE_DLACC);
  file_menu->insertItem(i18n("Sen&d pending"), FILE_SENDP);
  file_menu->insertItem(i18n("&Cancel"), FILE_CANCEL);
  file_menu->insertSeparator();
  file_menu->insertItem(i18n("Se&ttings"), FILE_SET);
  file_menu->insertItem(i18n("E&xit"), FILE_EXIT);

  //groups
  groups_menu=new QPopupMenu();
 	groups_menu->insertItem(i18n("&Properties"), GROUP_PROP);
 	groups_menu->insertItem(i18n("&Get new articles"), GROUP_DL_HDRS);
 	groups_menu->insertItem(i18n("E&xpire now"), GROUP_EXPNOW);
 	groups_menu->insertItem(i18n("Res&ort"), GROUP_RESORT);
 	groups_menu->insertSeparator();
 	groups_menu->insertItem(i18n("&Subscribe"), GROUP_SUBSCRIBE);
 	groups_menu->insertItem(i18n("&Unsubscribe"), GROUP_UNSUBSCRIBE);
 	groups_menu->insertSeparator();
 	groups_menu->insertItem(i18n("Mark all as &read"), GROUP_ALL_READ);
 	groups_menu->insertItem(i18n("Mark all as u&nread"), GROUP_ALL_UNREAD);
 	groups_menu->insertSeparator();

 	//folders
 	groups_menu_folders=new QPopupMenu();
 	groups_menu_folders->insertItem(i18n("&Compact"), FOLDERS_COMPACT);
 	groups_menu_folders->insertItem(i18n("Em&pty folder"), FOLDERS_EMPTY);
 	groups_menu->insertItem(i18n("&Folders"), groups_menu_folders, GROUP_FOLDERS);

  //article
  message_menu=new QPopupMenu();
  message_menu->insertItem(i18n("&Post new article"), ART_POST);
  message_menu->insertItem(i18n("Post &reply"), ART_POST_REPL);
  message_menu->insertItem(i18n("&Mail reply"), ART_MAIL_REPL);
  message_menu->insertItem(i18n("&Forward"), ART_FORWARD);
  message_menu->insertSeparator();

  //mark as
  message_menu_mark=new QPopupMenu();
  message_menu_mark->insertItem(i18n("as &read"), MARK_READ);
  message_menu_mark->insertItem(i18n("as &unread"), MARK_UNREAD);
  message_menu->insertItem(i18n("Mar&k"), message_menu_mark, ART_MARK);
  	
 	//thread
 	message_menu_thread=new QPopupMenu();
 	message_menu_thread->insertItem(i18n("Mark as &read"), THR_READ);
 	message_menu_thread->insertItem(i18n("Mark as &unread"), THR_UREAD);
 	message_menu_thread->insertItem(i18n("Set &score"), THR_SCORE);
 	message_menu_thread->insertItem(i18n("&Watch"), THR_WATCH);
 	message_menu_thread->insertItem(i18n("&Ignore"), THR_IGNORE);
 	message_menu_thread->insertSeparator();
 	message_menu_thread->insertItem(i18n("&Toggle"), THR_TOGGLE); // CG
 	message_menu->insertItem(i18n("T&hread"), message_menu_thread, ART_THREAD);
 	message_menu->insertSeparator();
  message_menu->insertItem(i18n("&Open in own window"), ART_OWINDOW);

 			
 	//own messages
 	message_menu->insertItem(i18n("&Edit"), ART_EDIT);
 	message_menu->insertItem(i18n("&Delete"), ART_DEL);
 	message_menu->insertItem(i18n("&Cancel post"), ART_CANCEL);
 	message_menu->insertSeparator();
 	message_menu->insertItem(i18n("Send &now"), ART_SEND_NOW);
 	message_menu->insertItem(i18n("Send &later"), ART_SEND_LAT);
 	message_menu->insertSeparator();
 	message_menu->insertItem(i18n("&Search"), ART_SEARCH);
 	  	
 	//goto
 	goto_menu=new QPopupMenu();
 	goto_menu->insertItem(i18n("&Next article"), GOTO_NEXT_ART);
 	goto_menu->insertItem(i18n("&Previous article"), GOTO_PREV_ART);
 	goto_menu->insertSeparator(); 	
 	goto_menu->insertItem(i18n("Next unread &article"), GOTO_NEXT_UNRART);
 	goto_menu->insertItem(i18n("Next unread &thread"), GOTO_NEXT_THR);
 	goto_menu->insertSeparator();
 	goto_menu->insertItem(i18n("Ne&xt group"), GOTO_NEXT_GROUP);
 	goto_menu->insertItem(i18n("Pre&vious group"), GOTO_PREV_GROUP);

  //view
  view_menu = new QPopupMenu();
  view_menu->setCheckable(true);
  view_menu->insertItem(i18n("Show th&reads"), VIEW_SHOW_THR);
  view_menu->insertItem(i18n("&Expand all"), VIEW_EXP_ALL);
  view_menu->insertItem(i18n("&Collapse all"), VIEW_COL_ALL);
  view_menu->insertItem(i18n("&Refresh"), VIEW_REFRSH);
  view_menu->insertSeparator();
  view_menu->insertItem(i18n("Show &all headers"), VIEW_ALL_HDRS);
  view_menu->setItemChecked(VIEW_ALL_HDRS, KNArticleWidget::fullHeaders());
  view_menu->insertItem(i18n("&Filter"), FiManager->pUpMenu(), VIEW_FILTER);
  view_menu_sort=new QPopupMenu();
  view_menu_sort->setCheckable(true);
  view_menu_sort->insertItem(i18n("Subject"), 0);
  view_menu_sort->insertItem(i18n("From"), 1);
  view_menu_sort->insertItem(i18n("Score"), 2);
  view_menu_sort->insertItem(i18n("Date"), 3);
  view_menu->insertItem(i18n("So&rt"), view_menu_sort, VIEW_SORT);
  view_menu->insertSeparator();
  view_menu->insertItem(i18n("&Statusbar"), VIEW_SHOW_STATB);
  view_menu->insertItem(i18n("&Toolbar"), VIEW_SHOW_TOOLB0);

  //DEBUG
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

	QString aboutText=QString("KNode %1\
\n\nhttp://knode.sourceforge.net/\
\n\n(c) 1999-2000 Christian Thurner\
\n\nAuthors:\
\n  Christian Thurner <cthurner@freepage.de>\
\n  Christian Gebauer <gebauer@bigfoot.com>\
\n  Dirk Mueller <mueller@kde.org>\
\n\nThis is free software covered by the GPL.").arg(VERSION);

	//init
	menuBar()->insertItem(i18n("&File"), file_menu);
  menuBar()->insertItem(i18n("&Group"), groups_menu);
  menuBar()->insertItem(i18n("&Article"), message_menu);
  menuBar()->insertItem(i18n("Go&to"), goto_menu);
  menuBar()->insertItem(i18n("&View"), view_menu);
  menuBar()->insertSeparator();
	menuBar()->insertItem(i18n("&Help"), helpMenu(aboutText,false));

	//connect
	connect(file_menu, SIGNAL(activated (int)), SLOT(slotMainCallback (int)));
  connect(groups_menu, SIGNAL(activated (int)), SLOT(slotMainCallback (int)));
  connect(groups_menu_folders, SIGNAL(activated (int)), SLOT(slotMainCallback (int)));
  connect(message_menu, SIGNAL(activated (int)), SLOT(slotMainCallback (int)));
  connect(message_menu_thread, SIGNAL(activated (int)), SLOT(slotMainCallback (int)));
  connect(message_menu_mark, SIGNAL(activated (int)), SLOT(slotMainCallback (int)));
 	connect(goto_menu, SIGNAL(activated (int)), SLOT(slotMainCallback (int)));
  connect(view_menu, SIGNAL(activated (int)), SLOT(slotMainCallback (int)));
  connect(view_menu_sort, SIGNAL(activated (int)), SLOT(slotViewSort (int)));

}



void KNodeApp::initPopups()
{
	accPopup=new QPopupMenu();
	accPopup->insertItem(i18n("&Download new headers"), FILE_DLACC);
	
	groupPopup=new QPopupMenu();
	groupPopup->insertItem(i18n("&Properties"), GROUP_PROP);
	groupPopup->insertSeparator();
	groupPopup->insertItem(i18n("Mark all articles as &read"), GROUP_ALL_READ);
	groupPopup->insertItem(i18n("Mark all articles as &unread"), GROUP_ALL_UNREAD);
	groupPopup->insertSeparator();
	groupPopup->insertItem(i18n("U&nsubscribe"), GROUP_UNSUBSCRIBE);
	
	fetchPopup=new QPopupMenu();
	fetchPopup->insertItem(i18n("&Post reply"), ART_POST_REPL);
	fetchPopup->insertItem(i18n("&Mail reply"), ART_MAIL_REPL);
	fetchPopup->insertItem(i18n("&Forward"), ART_FORWARD);
	fetchPopup->insertSeparator();
	fetchPopup->insertItem(i18n("Mark as &read"), MARK_READ);
	fetchPopup->insertItem(i18n("Mark as &unread"), MARK_UNREAD);
	fetchPopup->insertSeparator();
	fetchPopup->insertItem(i18n("&Open in own window"), ART_OWINDOW);
	
	folderPopup=new QPopupMenu();
	folderPopup->insertItem(i18n("&Empty"), FOLDERS_EMPTY);
	folderPopup->insertItem(i18n("&Compact"), FOLDERS_COMPACT);
	
	savedPopup=new QPopupMenu();
	savedPopup->insertItem(i18n("&Edit"), ART_EDIT);
	savedPopup->insertItem(i18n("&Delete"), ART_DEL);
	savedPopup->insertSeparator();
	savedPopup->insertItem(i18n("&Send now"), ART_SEND_NOW);
	savedPopup->insertItem(i18n("Send &later"), ART_SEND_LAT);	
	savedPopup->insertSeparator();
	savedPopup->insertItem(i18n("&Cancel post"), ART_CANCEL);	
}



void KNodeApp::initToolBar()
{
    tb0=new KToolBar(this,0,32);
    addToolBar(tb0);

    tb0->insertButton(BarIcon("dlall"), FILE_DLACC, true, i18n("download account"));
    tb0->insertButton(BarIcon("cancel"), FILE_CANCEL, true, i18n("stop all network activity"));
    tb0->insertSeparator();
    tb0->insertSeparator();

    tb0->insertButton(BarIcon("newmsg"), ART_POST, true, i18n("post new article"));
    tb0->insertButton(BarIcon("reply"), ART_POST_REPL, true, i18n("post reply"));
    tb0->insertButton(BarIcon("remail"), ART_MAIL_REPL, true, i18n("mail reply"));
    tb0->insertButton(BarIcon("fwd"), ART_FORWARD, true, i18n("forward"));
    tb0->insertSeparator();
    tb0->insertButton(BarIcon("nextart"), GOTO_NEXT_UNRART, true,
                      i18n("next unread article"));
    tb0->insertButton(BarIcon("nextthr"), GOTO_NEXT_THR, true,
                      i18n("next unread thread"));

    tb0->insertSeparator();
    tb0->insertButton(BarIcon("filter"),
                      VIEW_FILTER, FiManager->pUpMenu(), true, i18n("selects a filter"),-1);
    tb0->insertButton(BarIcon("srch"), ART_SEARCH, true, i18n("search for articles"));
    tb0->insertButton(BarIcon("refresh"), VIEW_REFRSH,
                      true, i18n("refresh the article list"));

    tb0->insertButton(BarIcon("grpdlg"), GROUP_SUBSCRIBE, true, i18n("newsgroups"));
    tb0->insertSeparator();
    tb0->insertSeparator();
    tb0->insertButton(BarIcon("settings"), FILE_SET, true, i18n("preferences"));

    connect(tb0,SIGNAL(clicked(int)),this,SLOT(slotMainCallback(int)));
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
    	
  connect(view->hdrView, SIGNAL(sortingChanged(int,int)),
  	this, SLOT(slotSortingChanged(int,int)));
  	
  connect(view->hdrView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
  	this, SLOT(slotArticlePopup(QListViewItem*, const QPoint&, int)));

  connect(view->collectionView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
  	this, SLOT(slotCollectionPopup(QListViewItem*, const QPoint&, int)));	
}



void KNodeApp::initAccel()
{
	acc=new KAccel(this);
	
	//stdAccels
	acc->connectItem(KStdAccel::Print, this, SLOT(slotFilePrint()));
	acc->connectItem(KStdAccel::Quit, this, SLOT(slotFileQuit()));
	acc->connectItem(KStdAccel::Save, this, SLOT(slotFileSaveAs()));	
	
	//customAccels
	acc->insertItem(i18n("post new article"), "post", "P");
	acc->connectItem("post", this, SLOT(slotArtNew()));
	
	acc->insertItem(i18n("post reply"), "reply", "R");
	acc->connectItem("reply", this, SLOT(slotArtReply()));
	
	acc->insertItem(i18n("mail reply"), "remail", "A");
	acc->connectItem("remail", this, SLOT(slotArtRemail()));
	
	acc->insertItem(i18n("forward"), "forwd", "F");
	acc->connectItem("forwd", this, SLOT(slotArtForward()));
	
	acc->insertItem(i18n("open in own window"), "openwin", "O");
	acc->connectItem("openwin", this, SLOT(slotArtOwnWindow()));
	
	acc->insertItem(i18n("mark article as read"), "mread", "D");
	acc->connectItem("mread", this, SLOT(slotArtMarkRead()));
	
	acc->insertItem(i18n("mark article as unread"), "muread", "U");
	acc->connectItem("muread", this, SLOT(slotArtMarkUnread()));
	
	acc->insertItem(i18n("mark thread as read"), "mtread", "ALT+D");
	acc->connectItem("mtread", this, SLOT(slotArtThrRead()));
	
	acc->insertItem(i18n("mark thread as uread"), "mturead", "ALT+U");
	acc->connectItem("mturead", this, SLOT(slotArtThrUnread()));
	
	acc->insertItem(i18n("set score"), "score", "S");
	acc->connectItem("score", this, SLOT(slotArtThrScore()));
	
	acc->insertItem(i18n("watch thread"), "watch", "W");
	acc->connectItem("watch", this, SLOT(slotArtThrWatch()));
	
	acc->insertItem(i18n("ignore thread"), "ignore", "I");
	acc->connectItem("ignore", this, SLOT(slotArtThrIgnore()));
		
	acc->insertItem(i18n("toggle subthread"), "togglethr", "T");   //CG
	acc->connectItem("togglethr", this, SLOT(slotArtThrToggle())); //CG
	
	acc->insertItem(i18n("edit article"), "edit", "E");
	acc->connectItem("edit", this, SLOT(slotArtEdit()));
	
	acc->insertItem(i18n("delete article"), "del", "Delete");
	acc->connectItem("del", this, SLOT(slotArtDelete()));
	
	acc->insertItem(i18n("next article"), "nxtart", "N");
	acc->connectItem("nxtart", this, SLOT(slotGotoNextArt()));
	
	acc->insertItem(i18n("previous article"), "prevart", "B");
	acc->connectItem("prevart", this, SLOT(slotGotoPrevArt()));
		
	acc->insertItem(i18n("next unread article"), "nxtunrart", "ALT+Space");
	acc->connectItem("nxtunrart", this, SLOT(slotGotoNextUnreadArt()));
	
	acc->insertItem(i18n("next unread thread"), "nxtthr", "CTRL+Space");
	acc->connectItem("nxtthr", this, SLOT(slotGotoNextThr()));
	
	acc->insertItem(i18n("read through articles"), "readthrough", "Space"); //CG
	acc->connectItem("readthrough", this, SLOT(slotReadThrough())); //CG
	
	acc->insertItem(i18n("next group"), "nxtgrp", "Plus");
	acc->connectItem("nxtgrp", this, SLOT(slotGotoNextGroup()));
	
	acc->insertItem(i18n("previous group"), "prevgrp", "Minus");
	acc->connectItem("prevgrp", this, SLOT(slotGotoPrevGroup()));
	
	acc->insertItem(i18n("refresh the article list"), "refresh", "F5");
	acc->connectItem("refresh", this, SLOT(slotViewRefresh()));
	
	/*acc->insertItem(i18n("Zoom"), "zoom", "Z");
	acc->connectItem("zoom", this, SLOT(slotViewZoom())); */
	
	acc->insertItem(i18n("search for article"), "search", "CTRL+F");
	acc->connectItem("search", this, SLOT(slotArtSearch()));
		
	acc->readSettings();

	setMenuAccels();
}



void KNodeApp::ViewToolBar()
{
  bViewToolbar=!bViewToolbar;
  menuBar()->setItemChecked(VIEW_SHOW_TOOLB0, bViewToolbar);
  enableToolBar(KToolBar::Toggle,0);
  setStatusMsg();
}



void KNodeApp::ViewStatusBar()
{
  bViewStatusbar=!bViewStatusbar;
  menuBar()->setItemChecked(VIEW_SHOW_STATB, bViewStatusbar);
  enableStatusBar();
  setStatusMsg();
}



void KNodeApp::setMenuAccels()
{
	//std Accels
	acc->changeMenuAccel(file_menu, FILE_PRINT, KStdAccel::Print);
	acc->changeMenuAccel(file_menu, FILE_EXIT, KStdAccel::Quit);
	acc->changeMenuAccel(file_menu, FILE_SAVEAS, KStdAccel::Save);
	
	//custom Accels
	acc->changeMenuAccel(file_menu, ART_SEARCH, "search");
	acc->changeMenuAccel(message_menu, ART_POST, "post");
	acc->changeMenuAccel(message_menu, ART_POST_REPL, "reply");
	acc->changeMenuAccel(message_menu, ART_MAIL_REPL, "remail");
	acc->changeMenuAccel(message_menu, ART_FORWARD, "forwd");
	acc->changeMenuAccel(message_menu, ART_OWINDOW, "openwin");
	acc->changeMenuAccel(message_menu_mark, MARK_READ, "mread");
	acc->changeMenuAccel(message_menu_mark, MARK_UNREAD, "muread");
	acc->changeMenuAccel(message_menu_thread, THR_READ, "mtread");
	acc->changeMenuAccel(message_menu_thread, THR_UREAD, "mturead");
	acc->changeMenuAccel(message_menu_thread, THR_SCORE, "score");
	acc->changeMenuAccel(message_menu_thread, THR_WATCH, "watch");
	acc->changeMenuAccel(message_menu_thread, THR_IGNORE, "ignore");
	acc->changeMenuAccel(message_menu_thread, THR_TOGGLE, "togglethr");
	acc->changeMenuAccel(message_menu, ART_EDIT, "edit");
	acc->changeMenuAccel(message_menu, ART_DEL, "del");
	acc->changeMenuAccel(goto_menu, GOTO_NEXT_ART, "nxtart");
	acc->changeMenuAccel(goto_menu, GOTO_PREV_ART, "prevart");
	acc->changeMenuAccel(goto_menu, GOTO_NEXT_UNRART, "nxtunrart");
	acc->changeMenuAccel(goto_menu, GOTO_NEXT_THR, "nxtthr");
	acc->changeMenuAccel(goto_menu, GOTO_NEXT_GROUP, "nxtgrp");
	acc->changeMenuAccel(goto_menu, GOTO_PREV_GROUP, "prevgrp");
	acc->changeMenuAccel(view_menu, VIEW_REFRSH, "refresh");
}



void KNodeApp::updateMenus(int *idArr, bool e)
{
	int idx=0;
	while(idArr[idx]!=-1) {
		menuBar()->setItemEnabled(idArr[idx], e);
		toolBar()->setItemEnabled(idArr[idx], e);
		idx++;
	}
}



void KNodeApp::updateAccels(const char **idArr, bool e)
{
#if 0
	int idx=0;
	while(idArr[idx]!=0) {
		acc->setItemEnabled(idArr[idx], e);
		idx++;
	}
#endif
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


  conf->writeEntry("ShowTBar0",tb0->isVisible());
  conf->writeEntry("ShowStatusbar",statusBar()->isVisible());
#if 0
  conf->writeEntry("MenuBarPos", (int)menuBar()->menuBarPos());
  conf->writeEntry("TBar0_Pos", (int)tb0->barPos());
#endif

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
//  acc->writeSettings();

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
  	   	
  	bViewToolbar = conf->readBoolEntry("ShowTBar0", true);
  	bViewStatusbar = conf->readBoolEntry("ShowStatusbar", true);
  	
#if 0  	
  	menu_bar_pos = (KMenuBar::menuPosition)
  		conf->readNumEntry("MenuBarPos", KMenuBar::Top);
  	
  	tool_bar_pos = (KToolBar::BarPosition)
  		conf->readNumEntry("TBar0_Pos", KToolBar::Top);
#endif  	
  		
  	//view
  	view->setSepPos(conf->readIntListEntry("Vert_SepPos"), conf->readIntListEntry("Horz_SepPos"));

  	conf->readListEntry("Hdrs_Size", lst);
  	if(lst.count()>0) view->setHeadersSize(&lst);
  	int sortCol=conf->readNumEntry("sortCol",0);
  	bool asc=conf->readBoolEntry("sortAscending", true);
  	view->hdrView->setColAsc(sortCol, asc);
  	view->hdrView->setSorting(sortCol, asc);
  	
  	view_menu_sort->setItemChecked(sortCol, true);
  	  	
  	conf->setGroup("READNEWS");
  	FiManager->setFilter(conf->readNumEntry("lastFilterID", 1));
  	FAManager->slotFilterChanged(FiManager->currentFilter());	
}



void KNodeApp::showSettings()
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


//========================================================================
//================================ SLOTS =================================
//========================================================================


//============== CALLBACK ==============

void KNodeApp::slotMainCallback(int id_)
{
#ifdef TEST
	KNComposer *composer;
	KNSavedArticle *sart;
	KNFilterConfigWidget *fconf;
	KNSearchDialog *sdl;
	KNSettingsDialog *set;
	KNPurgeProgressDialog *ppdlg;
#endif	

	
	switch(id_)
	{
		
		//file
		case FILE_EXIT:	 		slotFileQuit();	break;
		case FILE_SAVEAS: 		slotFileSaveAs(); break;
		
		case FILE_DLACC: 		GManager->checkAll(AManager->currentAccount()); break;
		case FILE_CANCEL:		NAcc->cancelAllJobs(); break;
		case FILE_PRINT: 		slotFilePrint(); break;		
		case FILE_SET:				showSettings(); break;
		case FILE_SENDP:			SAManager->sendOutbox(); break;
		
		//groups
		case	GROUP_PROP:			GManager->showGroupProperties(); break;		
		case 	GROUP_DL_HDRS:		GManager->checkGroupForNewHeaders(); break;
		case 	GROUP_EXPNOW:		GManager->expireGroupNow(); break;
		case 	GROUP_RESORT:   	GManager->resortGroup(); break;
		case	GROUP_SUBSCRIBE:			
			view->artView->showBlankPage();
			GManager->showGroupDialog(AManager->currentAccount());
		break;
		case 	GROUP_UNSUBSCRIBE:		
			if(KMessageBox::Yes == KMessageBox::questionYesNo(0, i18n("Do you really want to unsubscribe this group?"))) {
				view->artView->showBlankPage();
				GManager->unsubscribeGroup();
			}
		break;
		case	GROUP_ALL_READ:			  FAManager->setAllRead(0, true); break;
		case 	GROUP_ALL_UNREAD:		  FAManager->setAllRead(0, false); break;
				
		//folders
		case FOLDERS_COMPACT:	FoManager->compactFolder(); break;
		case FOLDERS_EMPTY:		FoManager->emptyFolder(); break;
				
		
		//article
		case	ART_POST:					slotArtNew(); 				break;
		case	ART_POST_REPL:			slotArtReply();				break;
		case 	ART_MAIL_REPL:			slotArtRemail();			break;
		case 	ART_FORWARD:				slotArtForward();			break;
		case 	ART_OWINDOW:				slotArtOwnWindow();		break;
		case 	MARK_READ:					slotArtMarkRead(); 		break;
		case 	MARK_UNREAD:				slotArtMarkUnread(); 	break;
		case 	THR_READ:					slotArtThrRead(); 		break;
		case 	THR_UREAD:					slotArtThrUnread(); 	break;
		case 	THR_SCORE:					slotArtThrScore(); 		break;
		case 	THR_WATCH:					slotArtThrWatch(); 		break;
		case 	THR_IGNORE:				slotArtThrIgnore(); 	break;
		case 	THR_TOGGLE:				slotArtThrToggle();		break;		
		case	ART_EDIT:					slotArtEdit();			  break;
		case 	ART_DEL:						slotArtDelete();      break;
		case	ART_CANCEL:				SAManager->cancel();	break;		
		case 	ART_SEND_NOW:			SAManager->sendArticle(); break;
		case	ART_SEND_LAT:			SAManager->sendArticle(0, false); break;
		case 	ART_SEARCH:				slotArtSearch();	break;
			
		//goto
		case GOTO_NEXT_ART:				slotGotoNextArt(); 				break;
		case GOTO_PREV_ART:				slotGotoPrevArt(); 				break;
		case GOTO_NEXT_UNRART:			slotGotoNextUnreadArt(); 	break;
		case GOTO_NEXT_THR:				slotGotoNextThr();				break;
		case GOTO_NEXT_GROUP:			slotGotoNextGroup(); 			break;
		case GOTO_PREV_GROUP:			slotGotoPrevGroup();			break;
								
		//view
		case VIEW_SHOW_THR:
			FAManager->toggleThreaded();
			view_menu->setItemChecked(VIEW_SHOW_THR, FAManager->threaded());	
		break;
		case VIEW_EXP_ALL:				FAManager->expandAllThreads(true); break;
		case VIEW_COL_ALL:				FAManager->expandAllThreads(false); break;
		case VIEW_ALL_HDRS:			
			KNArticleWidget::toggleFullHeaders();
			view_menu->setItemChecked(VIEW_ALL_HDRS, KNArticleWidget::fullHeaders());
		break;
		case VIEW_SHOW_STATB:		ViewStatusBar(); break;
		case VIEW_SHOW_TOOLB0:		ViewToolBar(); break;		
	  case VIEW_REFRSH:				slotViewRefresh(); break;
		//case VIEW_ZOOM:					slotViewZoom(); break;		
				
	
#ifdef TEST
		case 	10:
			/*sart=new KNSavedArticle();
			sart->setStatus(KNArticleBase::AStoPost);
			sart->setDestination("abc,def,ghi");
			composer=new KNComposer(sart);
			composer->show(); */
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
	
	
	} 	
}		                   		



//============= FILE-MENU ===============

void KNodeApp::slotFileSaveAs()
{
	if(FAManager->hasCurrentArticle())
		KNArticleManager::saveArticleToFile(FAManager->currentArticle());
		
	if(SAManager->hasCurrentArticle())
		KNArticleManager::saveArticleToFile(SAManager->currentArticle());	
}



void KNodeApp::slotFileQuit()
{
  cleanup();
//  KTMainWindow::deleteAll();
  kapp->quit();
}



void KNodeApp::slotFilePrint()
{
#warning FIXME print not implemented
//	view->artView->print();
}



//============= MESSAGE-MENU ===============

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



void KNodeApp::slotArtThrToggle()    //CG
{
	view->toggleThread(); //CG
}



void KNodeApp::slotArtEdit()
{
	SAManager->editArticle();	
}



void KNodeApp::slotArtDelete()
{
	SAManager->deleteArticle(0, true);	
}



void KNodeApp::slotArtSearch()
{
	FAManager->search();
}



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



void KNodeApp::slotReadThrough()  //CG
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



void KNodeApp::slotViewSort(int id)
{
	view->hdrView->slotSortList(id);
}



void KNodeApp::slotViewRefresh()
{
	FAManager->showHdrs();
}



void KNodeApp::slotViewZoom()
{
	
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



void KNodeApp::slotSortingChanged(int oldCol, int newCol )
{
	view_menu_sort->setItemChecked(oldCol, false);
	view_menu_sort->setItemChecked(newCol, true);
}



void KNodeApp::slotArticlePopup(QListViewItem *it, const QPoint &p, int)
{
	KNFetchArticle *fArt=0;
	KNSavedArticle *sArt=0;
	int id;
	
	if(it) {
		if(((KNHdrViewItem*)it)->art->type()==KNArticleBase::ATfetch) {
			fArt=(KNFetchArticle*)((KNHdrViewItem*)it)->art;
			fetchPopup->setItemEnabled(ART_POST_REPL, fArt->hasContent());
			fetchPopup->setItemEnabled(ART_MAIL_REPL, fArt->hasContent());
			fetchPopup->setItemEnabled(ART_FORWARD, fArt->hasContent());
			id=fetchPopup->exec(p);
		}
	  else {
	  	sArt=(KNSavedArticle*)((KNHdrViewItem*)it)->art;
	  	savedPopup->setItemEnabled(ART_CANCEL, sArt->hasContent() );
	  	id=savedPopup->exec(p);
	  }
		
  	switch(id) {
  		case ART_POST_REPL:
  			SAManager->reply(fArt, GManager->currentGroup());
  		break;
  		case ART_MAIL_REPL:
  			SAManager->reply(fArt, 0);
  		break;
  		case ART_FORWARD:
  			SAManager->forward(fArt);
  		break;
  		case MARK_READ:
  			FAManager->setArticleRead(fArt, true);
  		break;
  		case MARK_UNREAD:
  			FAManager->setArticleRead(fArt, false);
  		break;
  		case ART_OWINDOW:
  			FAManager->articleWindow(fArt);
  		break;
  		
  		case ART_EDIT:
  			SAManager->editArticle(sArt);
  		break;
  		case ART_DEL:
  			SAManager->deleteArticle(sArt, true);
  		break;
  		case ART_SEND_NOW:
  			SAManager->sendArticle(sArt, true);
  		break;
  		case ART_SEND_LAT:
  			SAManager->sendArticle(sArt, false);
  		break;
  		case ART_CANCEL:
  			SAManager->cancel(sArt);
  		break;  		
  	}
  }
}



void KNodeApp::slotCollectionPopup(QListViewItem *it, const QPoint &p, int c)
{
	KNGroup *grp=0;
	KNFolder *fld=0;
	KNNntpAccount *acc=0;
	int id;
	
	if(it) {
		if(((KNCollectionViewItem*)it)->coll->type()==KNCollection::CTgroup) {
			grp=(KNGroup*)((KNCollectionViewItem*)it)->coll;
			groupPopup->setItemEnabled(GROUP_ALL_READ, grp->isFilled());
			groupPopup->setItemEnabled(GROUP_ALL_UNREAD, grp->isFilled());	
			id=groupPopup->exec(p);
		}
		else if(((KNCollectionViewItem*)it)->coll->type()==KNCollection::CTfolder) {
			fld=(KNFolder*)((KNCollectionViewItem*)it)->coll;	
			folderPopup->setItemEnabled(FOLDERS_EMPTY, !fld->isEmpty());
			id=folderPopup->exec(p);
		}
		else if(((KNCollectionViewItem*)it)->coll->type()==KNCollection::CTnntpAccount) {
			acc=(KNNntpAccount*)((KNCollectionViewItem*)it)->coll;
			id=accPopup->exec(p);
		}
		else return;		
	  switch(id) {
  		case FILE_DLACC:
  			GManager->checkAll(acc);
  		break;
  		case GROUP_PROP:
  			GManager->showGroupProperties(grp);
  		break;
  		case GROUP_ALL_READ:
  			FAManager->setAllRead(grp, true);
  		break;
  		case GROUP_ALL_UNREAD:
  			FAManager->setAllRead(grp, false);
  		break;
  		case GROUP_UNSUBSCRIBE:
  			if(KMessageBox::Yes==KMessageBox::questionYesNo(0, i18n("Do you really want to unsubscribe this group?"))) {
  				view->artView->showBlankPage();
  				GManager->unsubscribeGroup(grp);
  			}
  		break;
		
			case FOLDERS_EMPTY:
				FoManager->emptyFolder(fld);
			break;
			case FOLDERS_COMPACT:
				FoManager->compactFolder(fld);
			break;
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
