/***************************************************************************
                     knodeview.cpp - description
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

#include <qheader.h>
#include <stdlib.h>

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kapp.h>
#include <kdebug.h>
#include <kmessagebox.h>

//GUI
#include "knodeview.h"
#include "knode.h"
#include "knarticlewidget.h"
#include "knarticlewindow.h"
#include "knscoredialog.h"
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"
#include "knfocuswidget.h"

//Core
#include "knglobals.h"
#include "knconfigmanager.h"
#include "knarticlemanager.h"
#include "knarticlefactory.h"
#include "kngroup.h"
#include "kngroupmanager.h"
#include "knnntpaccount.h"
#include "knaccountmanager.h"
#include "knnetaccess.h"
#include "knfiltermanager.h"
#include "knfoldermanager.h"
#include "knfolder.h"
#include "kncleanup.h"



KNodeView::KNodeView(KNMainWindow *w, const char * name)
  : QSplitter(w, name), l_ongView(true), b_lockui(false), s_electedAccount(0), s_electedGroup(0),
    s_electedFolder(0), s_electedArticle(0), a_ctions(w->actionCollection())
{

  //------------------------------- <CONFIG> ----------------------------------
  c_fgManager=new KNConfigManager();
  knGlobals.cfgManager=c_fgManager;
  //------------------------------- </CONFIG> ----------------------------------

  //-------------------------------- <GUI> ------------------------------------
  setOpaqueResize(true);

  //collection view
  c_olFocus=new KNFocusWidget(this,"colFocus");
  c_olView=new KNListView(c_olFocus,"collectionView");
  c_olFocus->setWidget(c_olView);
  setResizeMode(c_olFocus, QSplitter::KeepSize);

  c_olView->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  c_olView->setTreeStepSize(12);
  c_olView->setRootIsDecorated(true);
  c_olView->setShowSortIndicator(true);
  c_olView->addColumn(i18n("Name"),162);
  c_olView->addColumn(i18n("Total"),36);
  c_olView->addColumn(i18n("Unread"),48);
  c_olView->setColumnAlignment(1,AlignCenter);
  c_olView->setColumnAlignment(2,AlignCenter);

  connect(c_olView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotCollectionSelected(QListViewItem*)));
  connect(c_olView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotCollectionRMB(QListViewItem*, const QPoint&, int)));

  //secondary splitter
  s_ecSplitter=new QSplitter(QSplitter::Vertical,this,"secSplitter");
  s_ecSplitter->setOpaqueResize(true);

  //header view
  h_drFocus=new KNFocusWidget(s_ecSplitter,"hdrFocus");
  h_drView=new KNListView(h_drFocus,"hdrView");
	h_drFocus->setWidget(h_drView);
  s_ecSplitter->setResizeMode(h_drFocus, QSplitter::KeepSize);

  h_drView->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  h_drView->setShowSortIndicator(true);
  h_drView->setRootIsDecorated(true);
  h_drView->addColumn(i18n("Subject"),207);
  h_drView->addColumn(i18n("From"),115);
  h_drView->addColumn(i18n("Score"),42);
  h_drView->addColumn(i18n("Date (Time)"),102);
  h_drView->setColumnAlignment(2, AlignCenter);

	connect(h_drView, SIGNAL(selectionChanged(QListViewItem*)),
	  this, SLOT(slotArticleSelected(QListViewItem*)));
	connect(h_drView, SIGNAL(doubleClicked(QListViewItem*)),
	  this, SLOT(slotArticleDoubleClicked(QListViewItem*)));
  connect(h_drView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotArticleRMB(QListViewItem*, const QPoint&, int)));
  connect(h_drView, SIGNAL(sortingChanged(int)),
    this, SLOT(slotHdrViewSortingChanged(int)));

  //article view
  a_rtFocus=new KNFocusWidget(s_ecSplitter,"artFocus");
  a_rtView=new KNArticleWidget(a_ctions, a_rtFocus,"artView");
  a_rtFocus->setWidget(a_rtView);

  //tab order
  setTabOrder(h_drView, a_rtView);
  setTabOrder(a_rtView, c_olView);

  //actions
  initActions();

  //-------------------------------- </GUI> ------------------------------------



  //-------------------------------- <CORE> ------------------------------------

  //Network
  n_etAccess=new KNNetAccess();
  connect(n_etAccess, SIGNAL(netActive(bool)), this, SLOT(slotNetworkActive(bool)));
  knGlobals.netAccess=n_etAccess;

  //Filter Manager
  f_ilManager=new KNFilterManager(a_ctArtFilter);
  knGlobals.filManager=f_ilManager;

  //Article Manager
  a_rtManager=new KNArticleManager(h_drView, f_ilManager);
  knGlobals.artManager=a_rtManager;

  //Group Manager
  g_rpManager=new KNGroupManager(a_rtManager);
  knGlobals.grpManager=g_rpManager;

  //Folder Manager
  f_olManager=new KNFolderManager(c_olView, a_rtManager);
  knGlobals.folManager=f_olManager;

  //Account Manager
  a_ccManager=new KNAccountManager(g_rpManager, c_olView);
  knGlobals.accManager=a_ccManager;

  //Article Factory
  a_rtFactory=new KNArticleFactory(f_olManager, g_rpManager);
  knGlobals.artFactory=a_rtFactory;

  //-------------------------------- </CORE> -----------------------------------

  //apply saved options
  readOptions();

  //apply configuration
  configChanged();

  //set the keyboard focus indicator on the first item in the Collection View
  if(c_olView->firstChild())
    c_olView->setCurrentItem(c_olView->firstChild());
  c_olView->setFocus();
}



KNodeView::~KNodeView()
{
  saveOptions();

  h_drView->clear(); //avoid some random crashes in KNHdrViewItem::~KNHdrViewItem()

  delete n_etAccess;
  kdDebug(5003) << "KNodeView::~KNodeView() : Net deleted" << endl;

  delete a_rtManager;
  kdDebug(5003) << "KNodeView::~KNodeView() : Article Manager deleted" << endl;

  delete a_rtFactory;
  kdDebug(5003) << "KNodeView::~KNodeView() : Article Factory deleted" << endl;

  delete g_rpManager;
  kdDebug(5003) << "KNodeView::~KNodeView() : Group Manager deleted" << endl;

  delete f_olManager;
  kdDebug(5003) << "KNodeView::~KNodeView() : Folder Manager deleted" << endl;

  delete f_ilManager;
  kdDebug(5003) << "KNodeView::~KNodeView() : Filter Manager deleted" << endl;

  delete a_ccManager;
  kdDebug(5003) << "KNodeView::~KNodeView() : Account Manager deleted" << endl;

  delete c_fgManager;
  kdDebug(5003) << "KNodeView::~KNodeView() : Config deleted" << endl;

}



void KNodeView::readOptions()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("APPEARANCE");

  QValueList<int> lst = conf->readIntListEntry("Vert_SepPos");
  if (lst.count()!=2)
    lst << 266 << 487;
  if (l_ongView)
    setSizes(lst);
  else
    s_ecSplitter->setSizes(lst);

  lst = conf->readIntListEntry("Horz_SepPos");
  if (lst.count()!=2)
    lst << 153 << 234;
  if (l_ongView)
    s_ecSplitter->setSizes(lst);
  else
    setSizes(lst);

  lst = conf->readIntListEntry("Hdrs_Size");
  if (lst.count()==7) {
    QValueList<int>::Iterator it = lst.begin();

    QHeader *h=c_olView->header();
    for (int i=0; i<3; i++) {
      h->resizeSection(i,(*it));
      ++it;
    }

    h=h_drView->header();
    for (int i=0; i<4; i++) {
      h->resizeSection(i,(*it));
      ++it;
    }
  }


  int sortCol=conf->readNumEntry("sortCol",3);
  bool sortAsc=conf->readBoolEntry("sortAscending", false);
  h_drView->setColAsc(sortCol, sortAsc);
  h_drView->setSorting(sortCol, sortAsc);
  a_ctArtSortHeaders->setCurrentItem(sortCol);

  sortCol = conf->readNumEntry("account_sortCol", 0);
  sortAsc = conf->readBoolEntry("account_sortAscending", true);
  c_olView->setColAsc(sortCol, sortAsc);
  c_olView->setSorting(sortCol, sortAsc);

  a_rtManager->setShowThreads( conf->readBoolEntry("showThreads", true) );
  a_ctArtToggleShowThreads->setChecked( a_rtManager->showThreads() );
  a_rtView->setShowFullHdrs( conf->readBoolEntry("fullHdrs", false) );
}



void KNodeView::saveOptions()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("APPEARANCE");

  if (l_ongView) {
    conf->writeEntry("Vert_SepPos",sizes());
    conf->writeEntry("Horz_SepPos",s_ecSplitter->sizes());
  } else {
    conf->writeEntry("Vert_SepPos",s_ecSplitter->sizes());
    conf->writeEntry("Horz_SepPos",sizes());
  }

  QValueList<int> lst;
  QHeader *h=c_olView->header();
  for (int i=0; i<3; i++)
    lst << h->sectionSize(i);

  h=h_drView->header();
  for (int i=0; i<4; i++)
    lst << h->sectionSize(i);
  conf->writeEntry("Hdrs_Size", lst);

  conf->writeEntry("sortCol", h_drView->sortColumn());
  conf->writeEntry("sortAscending", h_drView->ascending());
  conf->writeEntry("account_sortCol", c_olView->sortColumn());
  conf->writeEntry("account_sortAscending", c_olView->ascending());

  conf->writeEntry("showThreads", a_rtManager->showThreads());
  conf->writeEntry("fullHdrs", a_rtView->showFullHdrs());
}


bool KNodeView::cleanup()
{
  if(!a_rtFactory->closeComposeWindows())
    return false;

  //expire & compact
  KNConfig::Cleanup *conf=c_fgManager->cleanup();
  KNCleanUp *cup=0;

  if(conf->expireToday()) {
    cup=new KNCleanUp(conf);
    g_rpManager->expireAll(cup);
    cup->start();
    conf->setLastExpireDate();
  }

  if(conf->compactToday()) {
    if(!cup)
      cup=new KNCleanUp(conf);
    else
      cup->reset();
    f_olManager->compactAll(cup);
    cup->start();
    conf->setLastCompactDate();
  }

  if(cup)
    delete cup;

  return true;
}


// switch between long & short group list, update fonts and colors
void KNodeView::configChanged()
{
  KNConfig::Appearance *app=c_fgManager->appearance();
  KNConfig::ReadNewsGeneral *rng=c_fgManager->readNewsGeneral();

  if(l_ongView != app->longGroupList()) {
    l_ongView = app->longGroupList();
    QValueList<int> size1 = sizes();
    QValueList<int> size2 = s_ecSplitter->sizes();
    if(l_ongView) {
      setOrientation(Qt::Horizontal);
      s_ecSplitter->setOrientation(Qt::Vertical);
      a_rtFocus->reparent(s_ecSplitter,0,QPoint(0,0),true);
      c_olFocus->reparent(this,0,QPoint(0,0),true);
      moveToFirst(c_olFocus);
      moveToLast(s_ecSplitter);
      setResizeMode(c_olFocus, QSplitter::KeepSize);
      setResizeMode(s_ecSplitter, QSplitter::Stretch);
      s_ecSplitter->moveToFirst(h_drFocus);
      s_ecSplitter->moveToLast(a_rtFocus);
      s_ecSplitter->setResizeMode(h_drFocus, QSplitter::KeepSize);
      s_ecSplitter->setResizeMode(a_rtFocus, QSplitter::Stretch);
    } else {
      setOrientation(Qt::Vertical);
      s_ecSplitter->setOrientation(Qt::Horizontal);
      a_rtFocus->reparent(this,0,QPoint(0,0),true);
      c_olFocus->reparent(s_ecSplitter,0,QPoint(0,0),true);
      moveToFirst(s_ecSplitter);
      moveToLast(a_rtFocus);
      setResizeMode(s_ecSplitter, QSplitter::KeepSize);
      setResizeMode(a_rtFocus, QSplitter::Stretch);
      s_ecSplitter->moveToFirst(c_olFocus);
      s_ecSplitter->moveToLast(h_drFocus);
      s_ecSplitter->setResizeMode(c_olFocus, QSplitter::KeepSize);
      s_ecSplitter->setResizeMode(h_drFocus, QSplitter::Stretch);
    }
    setSizes(size2);
    s_ecSplitter->setSizes(size1);
  }

  c_olView->setFont(app->groupListFont());
  a_rtManager->setViewFont();

  QPalette p = palette();
  p.setColor(QColorGroup::Base, app->backgroundColor());
  c_olView->setPalette(p);
  h_drView->setPalette(p);

}


void KNodeView::initActions()
{

  //navigation
  a_ctNavNextArt            = new KAction(i18n("&Next article"), "next", Key_N , this,
                              SLOT(slotNavNextArt()), a_ctions, "go_nextArticle");
  a_ctNavPrevArt            = new KAction(i18n("&Previous article"), "previous", Key_B , this,
                              SLOT(slotNavPrevArt()), a_ctions, "go_prevArticle");
  a_ctNavNextUnreadArt      = new KAction(i18n("Next unread &article"), "1rightarrow", ALT+Key_Space , this,
                              SLOT(slotNavNextUnreadArt()), a_ctions, "go_nextUnreadArticle");
  a_ctNavNextUnreadThread   = new KAction(i18n("Next unread &thread"),"2rightarrow", CTRL+Key_Space , this,
                              SLOT(slotNavNextUnreadThread()), a_ctions, "go_nextUnreadThread");
  a_ctNavNextGroup          = new KAction(i18n("Ne&xt group"), "down", Key_Plus , this,
                              SLOT(slotNavNextGroup()), a_ctions, "go_nextGroup");
  a_ctNavPrevGroup          = new KAction(i18n("Pre&vious group"), "up", Key_Minus , this,
                              SLOT(slotNavPrevGroup()), a_ctions, "go_prevGroup");
  a_ctNavReadThrough        = new KAction(i18n("Read &through articles"), Key_Space , this,
                              SLOT(slotNavReadThrough()), a_ctions, "go_readThrough");


  //collection-view - accounts
  a_ctAccProperties         = new KAction(i18n("&Properties..."), 0, this,
                              SLOT(slotAccProperties()), a_ctions, "account_properties");
  a_ctAccSubscribe          = new KAction(i18n("&Subscribe to Newsgroups..."),"news_subscribe", 0, this,
                              SLOT(slotAccSubscribe()), a_ctions, "account_subscribe");
  a_ctAccGetNewHdrs         = new KAction(i18n("&Get New Articles"), "mail_get", 0, this,
                              SLOT(slotAccGetNewHdrs()), a_ctions, "account_dnlHeaders");
  a_ctAccDelete             = new KAction(i18n("&Delete"), 0, this,
                              SLOT(slotAccDelete()), a_ctions, "account_delete");
  a_ctAccPostNewArticle     = new KAction(i18n("&Post new article"), "filenew", Key_P , this,
                              SLOT(slotAccPostNewArticle()), a_ctions, "article_postNew");


  //collection-view - groups
  a_ctGrpProperties         = new KAction(i18n("&Properties..."), 0, this,
                              SLOT(slotGrpProperties()), a_ctions, "group_properties");
  a_ctGrpGetNewHdrs         = new KAction(i18n("&Get New Articles"), "mail_get" , 0, this,
                              SLOT(slotGrpGetNewHdrs()), a_ctions, "group_dnlHeaders");
  a_ctGrpExpire             = new KAction(i18n("E&xpire Now"), "wizard", 0, this,
                              SLOT(slotGrpExpire()), a_ctions, "group_expire");
  a_ctGrpResort             = new KAction(i18n("Res&ort"), 0, this,
                              SLOT(slotGrpResort()), a_ctions, "group_resort");
  a_ctGrpUnsubscribe        = new KAction(i18n("&Unsubscribe"), 0, this,
                              SLOT(slotGrpUnsubscribe()), a_ctions, "group_unsubscribe");
  a_ctGrpSetAllRead         = new KAction(i18n("Mark all as &read"), "goto", 0, this,
                              SLOT(slotGrpSetAllRead()), a_ctions, "group_allRead");
	a_ctGrpSetAllUnread       = new KAction(i18n("Mark all as u&nread"), 0, this,
	                            SLOT(slotGrpSetAllUnread()), a_ctions, "group_allUnread");
	
	
	//collection-view - folder
  a_ctFolCompact            = new KAction(i18n("&Compact Folder"), "wizard", 0, this,
                              SLOT(slotFolCompact()), a_ctions, "folder_compact");
  a_ctFolEmpty              = new KAction(i18n("&Empty Folder"), 0, this,
                              SLOT(slotFolEmpty()), a_ctions, "folder_empty");
  a_ctFolProperties         = 0;
	
  					
  //header-view - list-handling
  a_ctArtSortHeaders        = new KSelectAction(i18n("&Sort"), 0, a_ctions, "view_Sort");
  QStringList items;
  items += i18n("By &Subject");
  items += i18n("By S&ender");
  items += i18n("By S&core");
  items += i18n("By &Date");
  a_ctArtSortHeaders->setItems(items);
	connect(a_ctArtSortHeaders, SIGNAL(activated(int)), this, SLOT(slotArtSortHeaders(int)));
	
	a_ctArtFilter             = new KNFilterSelectAction(i18n("&Filter"), "filter",
	                            a_ctions, "view_Filter");
	a_ctArtSearch             = new KAction(i18n("&Search..."),"find" , Key_F4 , this,
	                            SLOT(slotArtSearch()), a_ctions, "article_search");
	a_ctArtRefreshList        = new KAction(i18n("&Refresh List"),"reload", KStdAccel::key(KStdAccel::Reload), this,
	                            SLOT(slotArtRefreshList()), a_ctions, "view_Refresh");
	a_ctArtCollapseAll        = new KAction(i18n("&Collapse all threads"), 0 , this,
	                            SLOT(slotArtCollapseAll()), a_ctions, "view_CollapseAll");
	a_ctArtExpandAll          = new KAction(i18n("&Expand all threads"), 0 , this,
	                            SLOT(slotArtExpandAll()), a_ctions, "view_ExpandAll");
	a_ctArtToggleThread       = new KAction(i18n("&Toggle Subthread"), Key_T, this,
	                            SLOT(slotArtToggleThread()), a_ctions, "thread_toggle");
	a_ctArtToggleShowThreads	= new KToggleAction(i18n("Show T&hreads"), 0 , this,
	                            SLOT(slotArtToggleShowThreads()), a_ctions, "view_showThreads");			
		                            	
	//header-view - remote articles
	a_ctArtSetArtRead         = new KAction(i18n("M&ark as read"), Key_D , this,
	                            SLOT(slotArtSetArtRead()), a_ctions, "article_read");
	a_ctArtSetArtUnread       = new KAction(i18n("Mar&k as unread"), Key_U , this,
	                            SLOT(slotArtSetArtUnread()), a_ctions, "article_unread");
	a_ctArtSetThreadRead      = new KAction(i18n("Mark thread as r&ead"), CTRL+Key_D , this,
	                            SLOT(slotArtSetThreadRead()), a_ctions, "thread_read");
	a_ctArtSetThreadUnread    = new KAction(i18n("Mark thread as u&nread"), CTRL+Key_U , this,
	                            SLOT(slotArtSetThreadUnread()), a_ctions, "thread_unread");
	a_ctSetArtScore           =0;
	a_ctArtSetThreadScore     = new KAction(i18n("Set &Score..."), "rotate", Key_S , this,
	                            SLOT(slotArtSetThreadScore()), a_ctions, "thread_setScore");
	a_ctArtToggleIgnored      = new KAction(i18n("&Ignore"), "bottom", Key_I , this,
	                            SLOT(slotArtToggleIgnored()), a_ctions, "thread_ignore");
	a_ctArtToggleWatched      = new KAction(i18n("&Watch"), "top", Key_W , this,
	                            SLOT(slotArtToggleWatched()), a_ctions, "thread_watch");
	a_ctArtOpenNewWindow      = new KAction(i18n("&Open in own window"), "viewmag+", Key_O , this,
	                            SLOT(slotArtOpenNewWindow()), a_ctions, "article_ownWindow");
							
							
  //header-view local articles
  a_ctArtSendOutbox         = new KAction(i18n("Sen&d pending messages"), "mail_send", 0, this,
                              SLOT(slotArtSendOutbox()), a_ctions, "net_sendPending");
  a_ctArtDelete             = new KAction(i18n("&Delete"), "editdelete", Key_Delete, this,
                              SLOT(slotArtDelete()), a_ctions, "article_delete");
  a_ctArtSendNow            = new KAction(i18n("Send &now"),"mail_send", 0 , this,
                              SLOT(slotArtSendNow()), a_ctions, "article_sendNow");
  a_ctArtSendLater          = new KAction(i18n("Send &later"), "queue", 0 , this,
                              SLOT(slotArtSendLater()), a_ctions, "article_sendLater");

  //network
  a_ctNetCancel             = new KAction(i18n("Stop &Network"),"stop",0, this,
                              SLOT(slotNetCancel()), a_ctions, "net_stop");
  a_ctNetCancel->setEnabled(false);

}


//called in KNodeApp's constructor
void KNodeView::initPopups(KNMainWindow *w)
{
  a_ccPopup = static_cast<QPopupMenu *>(w->factory()->container("account_popup", w));
  if (!a_ccPopup) a_ccPopup = new QPopupMenu();

  g_roupPopup = static_cast<QPopupMenu *>(w->factory()->container("group_popup", w));
  if (!g_roupPopup) g_roupPopup = new QPopupMenu();

  f_olderPopup = static_cast<QPopupMenu *>(w->factory()->container("folder_popup", w));
  if (!f_olderPopup) f_olderPopup = new QPopupMenu();

  r_emotePopup = static_cast<QPopupMenu *>(w->factory()->container("remote_popup", w));
  if (!r_emotePopup) r_emotePopup = new QPopupMenu();

  l_ocalPopup = static_cast<QPopupMenu *>(w->factory()->container("local_popup", w));
  if (!l_ocalPopup) l_ocalPopup = new QPopupMenu();
}


void KNodeView::paletteChange ( const QPalette & )
{
  knGlobals.cfgManager->appearance()->updateHexcodes();
  KNArticleWidget::configChanged();
  configChanged();
}


void KNodeView::fontChange ( const QFont & )
{
  knGlobals.artFactory->configChanged();
  KNArticleWidget::configChanged();
  configChanged();
}


void KNodeView::slotArticleSelected(QListViewItem *i)
{
  kdDebug(5003) << "KNodeView::slotArticleSelected(QListViewItem *i)" << endl;
  if(b_lockui)
    return;
  s_electedArticle=0;

  if(i)
    s_electedArticle=(static_cast<KNHdrViewItem*>(i))->art;

  a_rtView->setArticle(s_electedArticle);

  //actions
  bool enabled;

  enabled=( s_electedArticle && s_electedArticle->type()==KNMimeBase::ATremote );
  if(a_ctArtSetArtRead->isEnabled() != enabled) {
    a_ctArtSetArtRead->setEnabled(enabled);
  	a_ctArtSetArtUnread->setEnabled(enabled);
  	a_ctArtSetThreadRead->setEnabled(enabled);
  	a_ctArtSetThreadUnread->setEnabled(enabled);
  	//a_ctSetArtScore->setEnabled(enabled);
  	a_ctArtSetThreadScore->setEnabled(enabled);
  	a_ctArtToggleIgnored->setEnabled(enabled);
  	a_ctArtToggleWatched->setEnabled(enabled);
  	a_ctArtOpenNewWindow->setEnabled(enabled);
  }

  enabled=( s_electedArticle && s_electedArticle->type()==KNMimeBase::ATlocal );
  if(a_ctArtDelete->isEnabled() != enabled) {
    a_ctArtDelete->setEnabled(enabled);
    a_ctArtSendNow->setEnabled(enabled);
    a_ctArtSendLater->setEnabled(enabled);
  }
}


void KNodeView::slotArticleDoubleClicked(QListViewItem *it)
{
  if(!it)
    return;

  KNArticle *art=(static_cast<KNHdrViewItem*>(it))->art;

  if(art->type()==KNMimeBase::ATremote) {
    KNArticleWindow *w=new KNArticleWindow(art);
    w->show();
  }
  else if(art->type()==KNMimeBase::ATlocal) {
    a_rtFactory->edit( static_cast<KNLocalArticle*>(art) );
  }
}


void KNodeView::slotCollectionSelected(QListViewItem *i)
{
  kdDebug(5003) << "KNodeView::slotCollectionSelected(QListViewItem *i)" << endl;
  if(b_lockui)
    return;
  KNCollection *c=0;
  s_electedAccount=0;
  s_electedGroup=0;
  s_electedFolder=0;

  h_drView->clear();
  slotArticleSelected(0);

  if(i) {
    c=(static_cast<KNCollectionViewItem*>(i))->coll;
    switch(c->type()) {
      case KNCollection::CTnntpAccount :
        s_electedAccount=static_cast<KNNntpAccount*>(c);
        if(!i->isOpen())
          i->setOpen(true);
      break;

      case KNCollection::CTgroup :
        s_electedGroup=static_cast<KNGroup*>(c);
        s_electedAccount=s_electedGroup->account();
      break;

      case KNCollection::CTfolder :
        s_electedFolder=static_cast<KNFolder*>(c);
      break;

      default: break;
    }

  }

  a_ccManager->setCurrentAccount(s_electedAccount);
  g_rpManager->setCurrentGroup(s_electedGroup);
  f_olManager->setCurrentFolder(s_electedFolder);

  //actions
  bool enabled;

  enabled=(s_electedGroup) || (s_electedFolder);
  if(a_ctNavNextArt->isEnabled() != enabled) {
    a_ctNavNextArt->setEnabled(enabled);
    a_ctNavPrevArt->setEnabled(enabled);
  }

  enabled=( s_electedGroup!=0 );
  if(a_ctNavNextUnreadArt->isEnabled() != enabled) {
    a_ctNavNextUnreadArt->setEnabled(enabled);
    a_ctNavNextUnreadThread->setEnabled(enabled);
    a_ctNavReadThrough->setEnabled(enabled);
  }

  enabled=( s_electedAccount!=0 );
  if(a_ctAccProperties->isEnabled() != enabled) {
    a_ctAccProperties->setEnabled(enabled);
    a_ctAccSubscribe->setEnabled(enabled);
    a_ctAccGetNewHdrs->setEnabled(enabled);
    a_ctAccDelete->setEnabled(enabled);
    a_ctAccPostNewArticle->setEnabled(enabled);
  }

  enabled=( s_electedGroup!=0 );
  if(a_ctGrpProperties->isEnabled() != enabled) {
    a_ctGrpProperties->setEnabled(enabled);
    a_ctGrpGetNewHdrs->setEnabled(enabled);
    a_ctGrpExpire->setEnabled(enabled);
    a_ctGrpResort->setEnabled(enabled);
    a_ctGrpUnsubscribe->setEnabled(enabled);
    a_ctGrpSetAllRead->setEnabled(enabled);
  	a_ctGrpSetAllUnread->setEnabled(enabled);
  	a_ctArtSortHeaders->setEnabled(enabled);
		a_ctArtFilter->setEnabled(enabled);
		a_ctArtSearch->setEnabled(enabled);
		a_ctArtRefreshList->setEnabled(enabled);
		a_ctArtCollapseAll->setEnabled(enabled);
		a_ctArtExpandAll->setEnabled(enabled);
		a_ctArtToggleShowThreads->setEnabled(enabled);
  }
	
	enabled=( s_electedFolder!=0 );
	if(a_ctFolCompact->isEnabled() != enabled) {
  	a_ctFolCompact->setEnabled(enabled);
    a_ctFolEmpty->setEnabled(enabled);
    //a_ctFolProperties->setEnabled( (s_electedFolder) );
  }

}


void KNodeView::slotArticleRMB(QListViewItem *i, const QPoint &p, int)
{
  if(b_lockui)
    return;

  if(i) {
    if( (static_cast<KNHdrViewItem*>(i))->art->type()==KNMimeBase::ATremote)
      r_emotePopup->popup(p);
    else
      l_ocalPopup->popup(p);
  }
}


void KNodeView::slotCollectionRMB(QListViewItem *i, const QPoint &p, int)
{
  if(b_lockui)
    return;

  if(i) {
    if( (static_cast<KNCollectionViewItem*>(i))->coll->type()==KNCollection::CTgroup)
      g_roupPopup->popup(p);
    else if ((static_cast<KNCollectionViewItem*>(i))->coll->type()==KNCollection::CTfolder)
      f_olderPopup->popup(p);
    else
      a_ccPopup->popup(p);
  }
}


void KNodeView::slotHdrViewSortingChanged(int i)
{
  a_ctArtSortHeaders->setCurrentItem(i);
}


void KNodeView::slotNetworkActive(bool b)
{
  a_ctNetCancel->setEnabled(b);
}


//------------------------------ <Actions> --------------------------------


void KNodeView::slotNavNextArt()
{
  kdDebug(5003) << "KNodeView::slotNavNextArt()" << endl;
  QListViewItem *it=h_drView->currentItem();

  if(it) it=it->itemBelow();
  else it=h_drView->firstChild();

  if(it) {
    h_drView->setSelected(it, true);
    h_drView->setCurrentItem(it);
    h_drView->ensureItemVisible(it);
  }
}



void KNodeView::slotNavPrevArt()
{
  kdDebug(5003) << "KNodeView::slotNavPrevArt()" << endl;
  QListViewItem *it=h_drView->currentItem();

  if(it) it=it->itemAbove();
  else it=h_drView->firstChild();

  if(it) {
    h_drView->setSelected(it, true);
    h_drView->setCurrentItem(it);
    h_drView->ensureItemVisible(it);
  }
}



void KNodeView::slotNavNextUnreadArt()
{
  kdDebug(5003) << "KNodeView::slotNavNextUnreadArt()" << endl;

  if(!s_electedGroup)
    return;

  KNHdrViewItem *next, *current;
  KNRemoteArticle *art;

  current=static_cast<KNHdrViewItem*>(h_drView->currentItem());
  if(!current)
    current=static_cast<KNHdrViewItem*>(h_drView->firstChild());

  if(!current) {               // no articles in the current group switch to next....
    slotNavNextGroup();
    return;
  }

  art=static_cast<KNRemoteArticle*>(current->art);

  if ((!current->isSelected())&&(!art->isRead()))   // take current article, if unread & not selected
    next=current;
  else {
    if(current->isExpandable() && !current->isOpen())
        h_drView->setOpen(current, true);
    next=static_cast<KNHdrViewItem*>(current->itemBelow());
  }

  while(next) {
    art=static_cast<KNRemoteArticle*>(next->art);
    if(!art->isRead()) break;
    else {
      if(next->isExpandable() && !next->isOpen())
        h_drView->setOpen(next, true);
      next=static_cast<KNHdrViewItem*>(next->itemBelow());
    }
  }

  if(next) {
    h_drView->setSelected(next, true);
    h_drView->setCurrentItem(next);
    h_drView->ensureItemVisible(next);
  }
  else
    slotNavNextGroup();

}




void KNodeView::slotNavNextUnreadThread()
{
  kdDebug(5003) << "KNodeView::slotNavNextUnreadThread()" << endl;

  KNHdrViewItem *next, *current;
  KNRemoteArticle *art;

  if(!s_electedGroup)
    return;

  current=static_cast<KNHdrViewItem*>(h_drView->currentItem());
  if(!current)
    current=static_cast<KNHdrViewItem*>(h_drView->firstChild());

  if(!current) {               // no articles in the current group switch to next....
    slotNavNextGroup();
    return;
  }

  art=static_cast<KNRemoteArticle*>(current->art);

  if((current->depth()==0)&&((!current->isSelected())&&(!art->isRead() || art->hasUnreadFollowUps())))
    next=current;                           // take current article, if unread & not selected
  else
    next=static_cast<KNHdrViewItem*>(current->itemBelow());

  while(next) {
    art=static_cast<KNRemoteArticle*>(next->art);

    if(next->depth()==0) {
      if(!art->isRead() || art->hasUnreadFollowUps()) break;
    }
    next=static_cast<KNHdrViewItem*>(next->itemBelow());
  }

  if(next) {
    h_drView->setCurrentItem(next);
    if(art->isRead()) slotNavNextUnreadArt();
    else {
      h_drView->setSelected(next, true);
      h_drView->ensureItemVisible(next);
    }
  }
  else
    slotNavNextGroup();

}



void KNodeView::slotNavNextGroup()
{
  kdDebug(5003) << "KNodeView::slotNavNextGroup()" << endl;
  KNCollectionViewItem *current=static_cast<KNCollectionViewItem*>(c_olView->currentItem());
  KNCollectionViewItem *next=0;

  if(!current) current=(KNCollectionViewItem*)c_olView->firstChild();
  if(!current) return;

  next=current;
  while(next) {
    if(!next->isSelected())
      break;
    if(next->childCount()>0 && !next->isOpen()) {
      next->setOpen(true);
      knGlobals.top->secureProcessEvents();
      next=static_cast<KNCollectionViewItem*>(next->firstChild());
    }
    else next=static_cast<KNCollectionViewItem*>(next->itemBelow());
  }

  if(next) {
    c_olView->setCurrentItem(next);
    c_olView->ensureItemVisible(next);
    c_olView->setSelected(next, true);
  }
}



void KNodeView::slotNavPrevGroup()
{
  kdDebug(5003) << "KNodeView::slotNavPrevGroup()" << endl;
  KNCollectionViewItem *current=static_cast<KNCollectionViewItem*>(c_olView->currentItem());
  KNCollectionViewItem *prev;

  if(!current) current=static_cast<KNCollectionViewItem*>(c_olView->firstChild());
  if(!current) return;

  prev=current;
  while(prev) {
    if(!prev->isSelected())
      break;
    prev=static_cast<KNCollectionViewItem*>(prev->itemAbove());
  }

  if(prev) {
    c_olView->setCurrentItem(prev);
    c_olView->ensureItemVisible(prev);
    c_olView->setSelected(prev, true);
  }
}


void KNodeView::slotNavReadThrough()
{
  kdDebug(5003) << "KNodeView::slotNavReadThrough()" << endl;
  if (a_rtView->scrollingDownPossible())
    a_rtView->scrollDown();
  else if(s_electedGroup != 0)
    slotNavNextUnreadArt();
}


void KNodeView::slotAccProperties()
{
  kdDebug(5003) << "KNodeView::slotAccProperties()" << endl;
  if(s_electedAccount)
    a_ccManager->editProperties(s_electedAccount);
}


void KNodeView::slotAccSubscribe()
{
  kdDebug(5003) << "KNodeView::slotAccSubscribe()" << endl;
  if(s_electedAccount)
    g_rpManager->showGroupDialog(s_electedAccount);
}


void KNodeView::slotAccGetNewHdrs()
{
  kdDebug(5003) << "KNodeView::slotAccGetNewHdrs()" << endl;
  if(s_electedAccount)
    g_rpManager->checkAll(s_electedAccount);
}


void KNodeView::slotAccDelete()
{
  kdDebug(5003) << "KNodeView::slotAccDelete()" << endl;
  if(s_electedAccount)
    a_ccManager->removeAccount(s_electedAccount);
}


void KNodeView::slotAccPostNewArticle()
{
  kdDebug(5003) << "KNodeView::slotAccPostNewArticle()" << endl;
  if(s_electedGroup)
    a_rtFactory->createPosting(s_electedGroup);
  else if(s_electedAccount)
    a_rtFactory->createPosting(s_electedAccount);
}


void KNodeView::slotGrpProperties()
{
  kdDebug(5003) << "slotGrpProperties()" << endl;
  if(s_electedGroup)
    g_rpManager->showGroupProperties(s_electedGroup);
}


void KNodeView::slotGrpGetNewHdrs()
{
  kdDebug(5003) << "KNodeView::slotGrpGetNewHdrs()" << endl;
  if(s_electedGroup)
    g_rpManager->checkGroupForNewHeaders(s_electedGroup);
}


void KNodeView::slotGrpExpire()
{
  kdDebug(5003) << "KNodeView::slotGrpExpire()" << endl;
  if(s_electedGroup)
    g_rpManager->expireGroupNow(s_electedGroup);
}


void KNodeView::slotGrpResort()
{
  kdDebug(5003) << "KNodeView::slotGrpResort()" << endl;
  if(s_electedGroup)
    g_rpManager->resortGroup(s_electedGroup);
}


void KNodeView::slotGrpUnsubscribe()
{
  kdDebug(5003) << "KNodeView::slotGrpUnsubscribe()" << endl;
  if(s_electedGroup)
    if(KMessageBox::Yes==KMessageBox::questionYesNo(knGlobals.topWidget,
      i18n("Do you really want to unsubscribe from %1?").arg(s_electedGroup->groupname())))
    g_rpManager->unsubscribeGroup(s_electedGroup);
}


void KNodeView::slotGrpSetAllRead()
{
  kdDebug(5003) << "KNodeView::slotGrpSetAllRead()" << endl;
  a_rtManager->setAllRead(true);
}


void KNodeView::slotGrpSetAllUnread()
{
  kdDebug(5003) << "KNodeView::slotGrpSetAllUnread()" << endl;
  a_rtManager->setAllRead(false);
}


void KNodeView::slotFolCompact()
{
  kdDebug(5003) << "KNodeView::slotFolCompact()" << endl;
  if(s_electedFolder)
    f_olManager->compactFolder(s_electedFolder);
}


void KNodeView::slotFolEmpty()
{
  kdDebug(5003) << "KNodeView::slotFolEmpty()" << endl;
  if(s_electedFolder) {
    if(s_electedFolder->lockedArticles()>0) {
      KMessageBox::sorry(knGlobals.topWidget,
      i18n("This Folder cannot be emptied at the moment\nbecause some of it's articles are currently in use.") );
      return;
    }
    if( KMessageBox::Yes == KMessageBox::questionYesNo(
        knGlobals.topWidget, i18n("Do you really want to empty this folder?")) )
      s_electedFolder->deleteAll();
  }
}


void KNodeView::slotFolProperties()
{
  kdDebug(5003) << "KNodeView::slotFolProperties()" << endl;
}


void KNodeView::slotArtSortHeaders(int i)
{
  kdDebug(5003) << "KNodeView::slotArtSortHeaders(int i)" << endl;
  h_drView->slotSortList(i);
}


void KNodeView::slotArtSearch()
{
  kdDebug(5003) << "KNodeView::slotArtSearch()" << endl;
  a_rtManager->search();
}


void KNodeView::slotArtRefreshList()
{
  kdDebug(5003) << "KNodeView::slotArtRefreshList()" << endl;
  a_rtManager->showHdrs(true);
}


void KNodeView::slotArtCollapseAll()
{
  kdDebug(5003) << "KNodeView::slotArtCollapseAll()" << endl;
  a_rtManager->setAllThreadsOpen(false);
}


void KNodeView::slotArtExpandAll()
{
  kdDebug(5003) << "KNodeView::slotArtExpandAll()" << endl;
  a_rtManager->setAllThreadsOpen(true);
}


void KNodeView::slotArtToggleThread()
{
  kdDebug(5003) << "KNodeView::slotArtToggleThread()" << endl;
  if(s_electedArticle && s_electedArticle->listItem()->isExpandable()) {
    bool o=!(s_electedArticle->listItem()->isOpen());
    s_electedArticle->listItem()->setOpen(o);
  }
}


void KNodeView::slotArtToggleShowThreads()
{
  kdDebug(5003) << "KNodeView::slotArtToggleShowThreads()" << endl;
  if(s_electedGroup) {
    a_rtManager->toggleShowThreads();
  }
}


void KNodeView::slotArtSetArtRead()
{
  kdDebug(5003) << "KNodeView::slotArtSetArtRead()" << endl;
  if(!s_electedGroup)
    return;

  KNRemoteArticle::List l;

  for(QListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected())
      l.append( static_cast<KNRemoteArticle*> ((static_cast<KNHdrViewItem*>(i))->art) );

  a_rtManager->setRead(&l, true);
}


void KNodeView::slotArtSetArtUnread()
{
  kdDebug(5003) << "KNodeView::slotArtSetArtUnread()" << endl;

  if(!s_electedGroup)
    return;

  KNRemoteArticle::List l;

  for(QListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected())
      l.append( static_cast<KNRemoteArticle*> ((static_cast<KNHdrViewItem*>(i))->art) );

  a_rtManager->setRead(&l, false);
}


void KNodeView::slotArtSetThreadRead()
{
  kdDebug(5003) << "slotArtSetThreadRead()" << endl;

  if( !s_electedArticle || !s_electedGroup )
    return;

  KNRemoteArticle::List l;
  (static_cast<KNRemoteArticle*>(s_electedArticle))->thread(&l);
  a_rtManager->setRead(&l, true);
}


void KNodeView::slotArtSetThreadUnread()
{
  kdDebug(5003) << "KNodeView::slotArtSetThreadUnread()" << endl;

  if( !s_electedArticle || !s_electedGroup )
    return;

  KNRemoteArticle::List l;
  (static_cast<KNRemoteArticle*>(s_electedArticle))->thread(&l);
  a_rtManager->setRead( &l, false);
}


void KNodeView::slotArtSetArtScore()
{
  kdDebug(5003) << "KNodeView::slotArtSetArtScore()" << endl;
}


void KNodeView::slotArtSetThreadScore()
{
  kdDebug(5003) << "KNodeView::slotArtSetThreadScore()" << endl;

  if( !s_electedArticle || !s_electedGroup )
    return;

  KNRemoteArticle::List l;
  (static_cast<KNRemoteArticle*>(s_electedArticle))->thread(&l);
  int score=l.first()->score();

  KNScoreDialog *sd= new KNScoreDialog(score, knGlobals.topWidget);
  if(sd->exec()) {
    score=sd->score();
    delete sd;
    a_rtManager->setScore(&l, score);
  }
  else
    delete sd;
}


void KNodeView::slotArtToggleIgnored()
{
  kdDebug(5003) << "KNodeView::slotArtToggleIgnored()" << endl;

  if( !s_electedArticle || !s_electedGroup )
    return;

  KNRemoteArticle::List l;
  (static_cast<KNRemoteArticle*>(s_electedArticle))->thread(&l);
  a_rtManager->toggleIgnored(&l);
}


void KNodeView::slotArtToggleWatched()
{
  kdDebug(5003) << "KNodeView::slotArtToggleWatched()" << endl;

  if( !s_electedArticle || !s_electedGroup )
    return;

  KNRemoteArticle::List l;
  (static_cast<KNRemoteArticle*>(s_electedArticle))->thread(&l);
  a_rtManager->toggleWatched(&l);
}


void KNodeView::slotArtOpenNewWindow()
{
  kdDebug(5003) << "KNodeView::slotArtOpenNewWindow()" << endl;

  if(s_electedArticle) {
    KNArticleWindow *win=new KNArticleWindow(s_electedArticle);
    win->show();
  }
}


void KNodeView::slotArtSendOutbox()
{
  kdDebug(5003) << "KNodeView::slotArtSendOutbox()" << endl;
  a_rtFactory->sendOutbox();
}


void KNodeView::slotArtDelete()
{
  kdDebug(5003) << "KNodeView::slotArtDelete()" << endl;

  if(s_electedArticle && s_electedArticle->type()==KNMimeBase::ATlocal) {
    KNLocalArticle::List lst;
    lst.append(static_cast<KNLocalArticle*>(s_electedArticle));
    a_rtFactory->deleteArticles(&lst);
  }
}


void KNodeView::slotArtSendNow()
{
  kdDebug(5003) << "KNodeView::slotArtSendNow()" << endl;

  if(s_electedArticle && s_electedArticle->type()==KNMimeBase::ATlocal) {
    KNLocalArticle::List lst;
    lst.append(static_cast<KNLocalArticle*>(s_electedArticle));
    a_rtFactory->sendArticles(&lst, true);
  }
}


void KNodeView::slotArtSendLater()
{
  kdDebug(5003) << "KNodeView::slotArtSendLater()" << endl;

  if(s_electedArticle && s_electedArticle->type()==KNMimeBase::ATlocal) {
    KNLocalArticle::List lst;
    lst.append(static_cast<KNLocalArticle*>(s_electedArticle));
    a_rtFactory->sendArticles(&lst, false);
  }
}


void KNodeView::slotNetCancel()
{
  kdDebug(5003) << "KNodeView::slotNetCancel()" << endl;
  n_etAccess->cancelAllJobs();
}

//-------------------------------- </Actions> ----------------------------------


//==============================================================================


KNFilterSelectAction::KNFilterSelectAction( const QString& text, const QString& pix,
                                            QObject* parent, const char *name )
 : KActionMenu(text,pix,parent,name), currentItem(-42)
{
  popupMenu()->setCheckable(true);
  connect(popupMenu(),SIGNAL(activated(int)),this,SLOT(slotMenuActivated(int)));
  setDelayed(false);
}



KNFilterSelectAction::~KNFilterSelectAction()
{
}

void KNFilterSelectAction::setCurrentItem(int id)
{
  popupMenu()->setItemChecked(currentItem, false);
  popupMenu()->setItemChecked(id, true);
  currentItem = id;
}


void KNFilterSelectAction::slotMenuActivated(int id)
{
  setCurrentItem(id);
  emit(activated(id));
}


#include "knodeview.moc"










