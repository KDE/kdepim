/*
    knodeview.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qheader.h>
#include <qhbox.h>
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
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"
#include "knfocuswidget.h"
#include "kndnd.h"

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
#include "utilities.h"
#include "knscoring.h"
#include "knpgp.h"


KNodeView::KNodeView(KNMainWindow *w, const char * name)
  : QSplitter(w, name), l_ongView(true), b_lockui(false), a_ctions(w->actionCollection())
{
  //------------------------------- <CONFIG> ----------------------------------
  c_fgManager=new KNConfigManager();
  knGlobals.cfgManager=c_fgManager;
  //------------------------------- </CONFIG> ----------------------------------

  //-------------------------------- <GUI> ------------------------------------
  setOpaqueResize(true);

  //collection view
  c_olFocus=new KNFocusWidget(this,"colFocus");
  c_olView=new KNListView(c_olFocus,"collectionView", new KNCollectionDragHandler() );
  c_olView->setAcceptDrops(true);
  c_olView->viewport()->setAcceptDrops(true);
  c_olView->setSelectionMode(QListView::Single);
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

  connect(c_olView, SIGNAL(itemSelected(QListViewItem*)),
    this, SLOT(slotCollectionSelected(QListViewItem*)));
  connect(c_olView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotCollectionRMB(QListViewItem*, const QPoint&, int)));
  connect(c_olView, SIGNAL(dropReceived(const char*, QListViewItem*)),
    this, SLOT(slotCollectionViewDrop(const char*, QListViewItem*)));

  //secondary splitter
  s_ecSplitter=new QSplitter(QSplitter::Vertical,this,"secSplitter");
  s_ecSplitter->setOpaqueResize(true);

  //header view
  h_drFocus=new KNFocusWidget(s_ecSplitter,"hdrFocus");
  h_drView=new KNListView(h_drFocus,"hdrView", new KNArticleDragHandler());
  h_drFocus->setWidget(h_drView);
  s_ecSplitter->setResizeMode(h_drFocus, QSplitter::KeepSize);

  h_drView->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  h_drView->setShowSortIndicator(true);
  h_drView->setRootIsDecorated(true);
  h_drView->addColumn(i18n("Subject"),207);
  h_drView->addColumn(i18n("From"),115);
  h_drView->addColumn(i18n("Score"),42);
  h_drView->addColumn(i18n("Lines"),42);
  h_drView->addColumn(i18n("Date"),102);

  connect(h_drView, SIGNAL(itemSelected(QListViewItem*)),
    this, SLOT(slotArticleSelected(QListViewItem*)));
  connect(h_drView, SIGNAL(selectionChanged()),
    this, SLOT(slotArticleSelectionChanged()));
  connect(h_drView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotArticleRMB(QListViewItem*, const QPoint&, int)));
  connect(h_drView, SIGNAL(mouseButtonPressed(int, QListViewItem *, const QPoint &, int)),
    this, SLOT(slotArticleMousePressed(int, QListViewItem *, const QPoint &, int)));
  connect(h_drView, SIGNAL(sortingChanged(int)),
    this, SLOT(slotHdrViewSortingChanged(int)));
  connect(h_drView, SIGNAL(dropReceived(const char*, QListViewItem*)),
    this, SLOT(slotHeaderViewDrop(const char*, QListViewItem*)));


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
  f_ilManager=new KNFilterManager(a_ctArtFilter, a_ctArtFilterKeyb);
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
  a_rtFactory=new KNArticleFactory();
  knGlobals.artFactory=a_rtFactory;

  // Score Manager
  s_coreManager = new KNScoringManager();
  knGlobals.scoreManager = s_coreManager;

  // create a global pgp instance
  new KNpgp();

  //-------------------------------- <GUI again> -------------------------------
  connect(s_coreManager, SIGNAL(changedRules()), SLOT(slotReScore()));
  //-------------------------------- </GUI again> ------------------------------

  //-------------------------------- </CORE> -----------------------------------

  //apply saved options
  readOptions();

  //apply configuration
  configChanged();

  // open all accounts that were open on the last shutdown
  KNNntpAccount *acc;
  for (acc=knGlobals.accManager->first(); acc; acc=knGlobals.accManager->next())
    if( acc->wasOpen() && acc->listItem() )
      acc->listItem()->setOpen(true);
}



KNodeView::~KNodeView()
{
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

  delete s_coreManager;
  kdDebug(5003) << "KNodeView::~KNodeView() : Score Manager deleted" << endl;
}



void KNodeView::readOptions()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("APPEARANCE");

  QValueList<int> lst = conf->readIntListEntry("Vert_SepPos");
  if (lst.count()!=2) {
    lst.clear();
    lst << 251 << 530;
  }
  if (l_ongView)
    setSizes(lst);
  else
    s_ecSplitter->setSizes(lst);

  lst = conf->readIntListEntry("Horz_SepPos");
  if (lst.count()!=2) {
    lst.clear();
    lst << 153 << 234;
  }
  if (l_ongView)
    s_ecSplitter->setSizes(lst);
  else
    setSizes(lst);

  lst = conf->readIntListEntry("Hdrs_Size3");
  if (lst.count()==8) {
    QValueList<int>::Iterator it = lst.begin();
    QHeader *h=c_olView->header();
    for (int i=0; i<3; i++) {
      h->resizeSection(i,(*it));
      ++it;
    }

    h=h_drView->header();
    for (int i=0; i<5; i++) {
      h->resizeSection(i,(*it));
      ++it;
    }
  }

  lst = conf->readIntListEntry("Hdr_Order");
  if (lst.count()==8) {
    QValueList<int>::Iterator it = lst.begin();

    QHeader *h=c_olView->header();
    for (int i=0; i<3; i++) {
      h->moveSection(i,(*it));
      ++it;
    }

    h=h_drView->header();
    for (int i=0; i<5; i++) {
      h->moveSection(i,(*it));
      ++it;
    }
  }

  int sortCol=conf->readNumEntry("sortCol",4);
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

  // store section sizes
  QValueList<int> lst;
  QHeader *h=c_olView->header();
  for (int i=0; i<3; i++)
    lst << h->sectionSize(i);

  h=h_drView->header();
  for (int i=0; i<5; i++)
    lst << h->sectionSize(i);
  conf->writeEntry("Hdrs_Size3", lst);

  // store section order
  lst.clear();
  h=c_olView->header();
  for (int i=0; i<3; i++)
    lst << h->mapToIndex(i);

  h=h_drView->header();
  for (int i=0; i<5; i++)
    lst << h->mapToIndex(i);
  conf->writeEntry("Hdr_Order", lst);

  // store sorting setup
  conf->writeEntry("sortCol", h_drView->sortColumn());
  conf->writeEntry("sortAscending", h_drView->ascending());
  conf->writeEntry("account_sortCol", c_olView->sortColumn());
  conf->writeEntry("account_sortAscending", c_olView->ascending());

  conf->writeEntry("showThreads", a_rtManager->showThreads());
  conf->writeEntry("fullHdrs", a_rtView->showFullHdrs());
}


bool KNodeView::requestShutdown()
{
  kdDebug(5003) << "KNodeView::requestShutdown()" << endl;

  if( a_rtFactory->jobsPending() &&
      KMessageBox::No==KMessageBox::warningYesNo(knGlobals.top, i18n(
"KNode is currently sending articles. If you quit now you might loose these \
articles.\nDo you want to continue anyway?"))
    )
    return false;

  if(!a_rtFactory->closeComposeWindows())
    return false;

  return true;
}


void KNodeView::prepareShutdown()
{
  kdDebug(5003) << "KNodeView::prepareShutdown()" << endl;

  //cleanup article-views
  KNArticleWidget::cleanup();

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

  saveOptions();
  a_rtManager->deleteTempFiles();
  g_rpManager->syncGroups();
  f_olManager->syncFolders();
  f_ilManager->prepareShutdown();
  a_ccManager->prepareShutdown();
  s_coreManager->save();
}


// switch between long & short group list, update fonts and colors
void KNodeView::configChanged()
{
  KNConfig::Appearance *app=c_fgManager->appearance();

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
  KNHdrViewItem::clearFontCache();
  h_drView->setFont(app->articleListFont());

  QPalette p = palette();
  p.setColor(QColorGroup::Base, app->backgroundColor());
  p.setColor(QColorGroup::Text, app->textColor());
  c_olView->setPalette(p);
  h_drView->setPalette(p);

  if (knGlobals.cfgManager->readNewsGeneral()->showScore()) {
    if (!h_drView->header()->isResizeEnabled(2)) {
      h_drView->header()->setResizeEnabled(true,2);
      h_drView->header()->setLabel(2,i18n("Score"),42);
    }
  } else {
    h_drView->header()->setLabel(2,QString::null,0);
    h_drView->header()->setResizeEnabled(false,2);
  }
  if (knGlobals.cfgManager->readNewsGeneral()->showLines()) {
    if (!h_drView->header()->isResizeEnabled(3)) {
      h_drView->header()->setResizeEnabled(true,3);
      h_drView->header()->setLabel(3,i18n("Lines"),42);
    }
  } else {
    h_drView->header()->setLabel(3,QString::null,0);
    h_drView->header()->setResizeEnabled(false,3);
  }

  a_rtManager->updateListViewItems();
}


void KNodeView::initActions()
{

  //navigation
  a_ctNavNextArt            = new KAction(i18n("&Next Article"), "next", Key_N , this,
                              SLOT(slotNavNextArt()), a_ctions, "go_nextArticle");
  a_ctNavPrevArt            = new KAction(i18n("&Previous Article"), "previous", Key_B , this,
                              SLOT(slotNavPrevArt()), a_ctions, "go_prevArticle");
  a_ctNavNextUnreadArt      = new KAction(i18n("Next Unread &Article"), "1rightarrow", ALT+Key_Space , this,
                              SLOT(slotNavNextUnreadArt()), a_ctions, "go_nextUnreadArticle");
  a_ctNavNextUnreadThread   = new KAction(i18n("Next Unread &Thread"),"2rightarrow", CTRL+Key_Space , this,
                              SLOT(slotNavNextUnreadThread()), a_ctions, "go_nextUnreadThread");
  a_ctNavNextGroup          = new KAction(i18n("Ne&xt Group"), "down", Key_Plus , this,
                              SLOT(slotNavNextGroup()), a_ctions, "go_nextGroup");
  a_ctNavPrevGroup          = new KAction(i18n("Pre&vious Group"), "up", Key_Minus , this,
                              SLOT(slotNavPrevGroup()), a_ctions, "go_prevGroup");
  a_ctNavReadThrough        = new KAction(i18n("Read &Through Articles"), Key_Space , this,
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
  a_ctAccPostNewArticle     = new KAction(i18n("&Post to Newsgroup..."), "filenew", Key_P , this,
                              SLOT(slotAccPostNewArticle()), a_ctions, "article_postNew");

  //collection-view - groups
  a_ctGrpProperties         = new KAction(i18n("&Properties..."), 0, this,
                              SLOT(slotGrpProperties()), a_ctions, "group_properties");
  a_ctGrpGetNewHdrs         = new KAction(i18n("&Get New Articles"), "mail_get" , 0, this,
                              SLOT(slotGrpGetNewHdrs()), a_ctions, "group_dnlHeaders");
  a_ctGrpExpire             = new KAction(i18n("E&xpire Now"), "wizard", 0, this,
                              SLOT(slotGrpExpire()), a_ctions, "group_expire");
  a_ctGrpReorganize         = new KAction(i18n("Re&organize"), 0, this,
                              SLOT(slotGrpReorganize()), a_ctions, "group_reorg");
  a_ctGrpUnsubscribe        = new KAction(i18n("&Unsubscribe"), "news_unsubscribe", 0, this,
                              SLOT(slotGrpUnsubscribe()), a_ctions, "group_unsubscribe");
  a_ctGrpSetAllRead         = new KAction(i18n("Mark All as &Read"), "goto", 0, this,
                              SLOT(slotGrpSetAllRead()), a_ctions, "group_allRead");
  a_ctGrpSetAllUnread       = new KAction(i18n("Mark All as U&nread"), 0, this,
                              SLOT(slotGrpSetAllUnread()), a_ctions, "group_allUnread");
  
  //collection-view - folder
  a_ctFolNew                = new KAction(i18n("&New"), 0, this,
                              SLOT(slotFolNew()), a_ctions, "folder_new");
  a_ctFolNewChild           = new KAction(i18n("New &Subfolder"), 0, this,
                              SLOT(slotFolNewChild()), a_ctions, "folder_newChild");
  a_ctFolDelete             = new KAction(i18n("&Delete"), 0, this,
                              SLOT(slotFolDelete()), a_ctions, "folder_delete");
  a_ctFolRename             = new KAction(i18n("&Rename"), 0, this,
                              SLOT(slotFolRename()), a_ctions, "folder_rename");
  a_ctFolMakeTopLevel       = new KAction(i18n("&Move to Toplevel"), 0, this,
                              SLOT(slotFolMakeTopLevel()), a_ctions, "folder_maketoplevel");
  a_ctFolCompact            = new KAction(i18n("C&ompact"), "wizard", 0, this,
                              SLOT(slotFolCompact()), a_ctions, "folder_compact");
  a_ctFolEmpty              = new KAction(i18n("&Empty"), 0, this,
                              SLOT(slotFolEmpty()), a_ctions, "folder_empty");
  a_ctFolMboxImport         = new KAction(i18n("&Import MBox Folder"), 0, this,
                              SLOT(slotFolMBoxImport()), a_ctions, "folder_MboxImport");
  a_ctFolMboxExport         = new KAction(i18n("E&xport as MBox Folder"), 0, this,
                              SLOT(slotFolMBoxExport()), a_ctions, "folder_MboxExport");

  //header-view - list-handling
  a_ctArtSortHeaders        = new KSelectAction(i18n("S&ort"), 0, a_ctions, "view_Sort");
  QStringList items;
  items += i18n("By &Subject");
  items += i18n("By S&ender");
  items += i18n("By S&core");
  items += i18n("By &Lines");
  items += i18n("By &Date");
  a_ctArtSortHeaders->setItems(items);
  connect(a_ctArtSortHeaders, SIGNAL(activated(int)), this, SLOT(slotArtSortHeaders(int)));
  a_ctArtSortHeadersKeyb   = new KAction(i18n("Sort"), 0, Key_F7 , this,
                             SLOT(slotArtSortHeadersKeyb()), a_ctions, "view_Sort_Keyb");
  
  a_ctArtFilter             = new KNFilterSelectAction(i18n("&Filter"), "filter",
                              a_ctions, "view_Filter");
  a_ctArtFilterKeyb         = new KAction(i18n("Filter"), Key_F6, a_ctions, "view_Filter_Keyb");
  a_ctArtSearch             = new KAction(i18n("&Search Articles..."),"find" , Key_F4 , this,
                              SLOT(slotArtSearch()), a_ctions, "article_search");
  a_ctArtRefreshList        = new KAction(i18n("&Refresh List"),"reload", KStdAccel::key(KStdAccel::Reload), this,
                              SLOT(slotArtRefreshList()), a_ctions, "view_Refresh");
  a_ctArtCollapseAll        = new KAction(i18n("&Collapse All Threads"), 0 , this,
                              SLOT(slotArtCollapseAll()), a_ctions, "view_CollapseAll");
  a_ctArtExpandAll          = new KAction(i18n("E&xpand All Threads"), 0 , this,
                              SLOT(slotArtExpandAll()), a_ctions, "view_ExpandAll");
  a_ctArtToggleThread       = new KAction(i18n("&Toggle Subthread"), Key_T, this,
                              SLOT(slotArtToggleThread()), a_ctions, "thread_toggle");
  a_ctArtToggleShowThreads  = new KToggleAction(i18n("Show T&hreads"), 0 , this,
                              SLOT(slotArtToggleShowThreads()), a_ctions, "view_showThreads");      
                                  
  //header-view - remote articles
  a_ctArtSetArtRead         = new KAction(i18n("Mark as &Read"), Key_D , this,
                              SLOT(slotArtSetArtRead()), a_ctions, "article_read");
  a_ctArtSetArtUnread       = new KAction(i18n("Mar&k as Unread"), Key_U , this,
                              SLOT(slotArtSetArtUnread()), a_ctions, "article_unread");
  a_ctArtSetThreadRead      = new KAction(i18n("Mark &Thread as Read"), CTRL+Key_D , this,
                              SLOT(slotArtSetThreadRead()), a_ctions, "thread_read");
  a_ctArtSetThreadUnread    = new KAction(i18n("Mark T&hread as Unread"), CTRL+Key_U , this,
                              SLOT(slotArtSetThreadUnread()), a_ctions, "thread_unread");
  a_ctArtOpenNewWindow      = new KAction(i18n("Open in own &window"), "viewmag+", Key_O , this,
                              SLOT(slotArtOpenNewWindow()), a_ctions, "article_ownWindow");

  // scoring
  a_ctScoresEdit            = new KAction(i18n("&Edit Scoring Rules..."), CTRL+Key_E, this,
                              SLOT(slotScoreEdit()), a_ctions, "scoreedit");
  a_ctReScore               = new KAction(i18n("Recalculate &Scores..."), 0, this,
                              SLOT(slotReScore()),a_ctions,"rescore");
  a_ctScoreLower            = new KAction(i18n("&Lower Score for Author..."), CTRL+Key_L, this,
                              SLOT(slotScoreLower()), a_ctions, "scorelower");
  a_ctScoreRaise            = new KAction(i18n("&Raise Score for Author..."), CTRL+Key_I, this,
                              SLOT(slotScoreRaise()),a_ctions,"scoreraise");
  a_ctArtToggleIgnored      = new KAction(i18n("&Ignore Thread"), "bottom", Key_I , this,
                              SLOT(slotArtToggleIgnored()), a_ctions, "thread_ignore");
  a_ctArtToggleWatched      = new KAction(i18n("&Watch Thread"), "top", Key_W , this,
                              SLOT(slotArtToggleWatched()), a_ctions, "thread_watch");

  //header-view local articles
  a_ctArtSendOutbox         = new KAction(i18n("Sen&d Pending Messages"), "mail_send", 0, this,
                              SLOT(slotArtSendOutbox()), a_ctions, "net_sendPending");
  a_ctArtDelete             = new KAction(i18n("&Delete"), "editdelete", Key_Delete, this,
                              SLOT(slotArtDelete()), a_ctions, "article_delete");
  a_ctArtSendNow            = new KAction(i18n("Send &Now"),"mail_send", 0 , this,
                              SLOT(slotArtSendNow()), a_ctions, "article_sendNow");
  a_ctArtEdit               = new KAction(i18n("edit article","&Edit..."), "signature", Key_E , this,
                              SLOT(slotArtEdit()), a_ctions, "article_edit");

  //network
  a_ctNetCancel             = new KAction(i18n("Stop &Network"),"stop",0, this,
                              SLOT(slotNetCancel()), a_ctions, "net_stop");
  a_ctNetCancel->setEnabled(false);

  a_ctFetchArticleWithID    = new KAction(i18n("&Fetch Article with ID..."), 0, this,
                              SLOT(slotFetchArticleWithID()), a_ctions, "fetch_article_with_id");
  a_ctFetchArticleWithID->setEnabled(false);

}


// called after createGUI()
void KNodeView::initPopups(KNMainWindow *w)
{
  a_ccPopup = static_cast<QPopupMenu *>(w->factory()->container("account_popup", w));
  if (!a_ccPopup) a_ccPopup = new QPopupMenu(w);

  g_roupPopup = static_cast<QPopupMenu *>(w->factory()->container("group_popup", w));
  if (!g_roupPopup) g_roupPopup = new QPopupMenu(w);

  f_olderPopup = static_cast<QPopupMenu *>(w->factory()->container("folder_popup", w));
  if (!f_olderPopup) f_olderPopup = new QPopupMenu(w);

  r_emotePopup = static_cast<QPopupMenu *>(w->factory()->container("remote_popup", w));
  if (!r_emotePopup) r_emotePopup = new QPopupMenu(w);

  l_ocalPopup = static_cast<QPopupMenu *>(w->factory()->container("local_popup", w));
  if (!l_ocalPopup) l_ocalPopup = new QPopupMenu(w);

  QPopupMenu *pop = static_cast<QPopupMenu *>(w->factory()->container("body_popup", w));
  if (!pop) pop = new QPopupMenu(w);
  a_rtView->setBodyPopup(pop);
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


void KNodeView::getSelectedArticles(KNRemoteArticle::List &l)
{
  if(!g_rpManager->currentGroup()) return;

  for(QListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected())
      l.append( static_cast<KNRemoteArticle*> ((static_cast<KNHdrViewItem*>(i))->art) );
}


void KNodeView::getSelectedThreads(KNRemoteArticle::List &l)
{
  KNRemoteArticle *art;
  for(QListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected()) {
      art=static_cast<KNRemoteArticle*> ((static_cast<KNHdrViewItem*>(i))->art);
      // ignore the article if it is already in the list
      // (multiple aritcles are selected in one thread)
      if (l.findRef(art)==-1)
        art->thread(l);
    }
}


void KNodeView::getSelectedArticles(QList<KNLocalArticle> &l)
{
  if(!f_olManager->currentFolder()) return;

  for(QListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected())
      l.append( static_cast<KNLocalArticle*> ((static_cast<KNHdrViewItem*>(i))->art) );
}


void KNodeView::slotArticleSelected(QListViewItem *i)
{
  kdDebug(5003) << "KNodeView::slotArticleSelected(QListViewItem *i)" << endl;
  if(b_lockui)
    return;
  KNArticle *selectedArticle=0;

  if(i)
    selectedArticle=(static_cast<KNHdrViewItem*>(i))->art;

  a_rtView->setArticle(selectedArticle);

  //actions
  bool enabled;

  enabled=( selectedArticle && selectedArticle->type()==KNMimeBase::ATremote );
  if(a_ctArtSetArtRead->isEnabled() != enabled) {
    a_ctArtSetArtRead->setEnabled(enabled);
    a_ctArtSetArtUnread->setEnabled(enabled);
    a_ctArtSetThreadRead->setEnabled(enabled);
    a_ctArtSetThreadUnread->setEnabled(enabled);
    a_ctArtToggleIgnored->setEnabled(enabled);
    a_ctArtToggleWatched->setEnabled(enabled);
    a_ctScoreLower->setEnabled(enabled);
    a_ctScoreRaise->setEnabled(enabled);
  }

  a_ctArtOpenNewWindow->setEnabled( selectedArticle && (f_olManager->currentFolder()!=f_olManager->outbox())
                                                    && (f_olManager->currentFolder()!=f_olManager->drafts()));

  enabled=( selectedArticle && selectedArticle->type()==KNMimeBase::ATlocal );
  a_ctArtDelete->setEnabled(enabled);
  a_ctArtSendNow->setEnabled(enabled && (f_olManager->currentFolder()==f_olManager->outbox()));
  a_ctArtEdit->setEnabled(enabled && ((f_olManager->currentFolder()==f_olManager->outbox())||
                                      (f_olManager->currentFolder()==f_olManager->drafts())));
}


void KNodeView::slotArticleSelectionChanged()
{
  // enable all actions that work with multiple selection

  //actions
  bool enabled = (g_rpManager->currentGroup()!=0);

  if(a_ctArtSetArtRead->isEnabled() != enabled) {
    a_ctArtSetArtRead->setEnabled(enabled);
    a_ctArtSetArtUnread->setEnabled(enabled);
    a_ctArtSetThreadRead->setEnabled(enabled);
    a_ctArtSetThreadUnread->setEnabled(enabled);
    a_ctArtToggleIgnored->setEnabled(enabled);
    a_ctArtToggleWatched->setEnabled(enabled);
    a_ctScoreLower->setEnabled(enabled);
    a_ctScoreRaise->setEnabled(enabled);
  }

  enabled = (f_olManager->currentFolder()!=0);
  a_ctArtDelete->setEnabled(enabled);
  a_ctArtSendNow->setEnabled(enabled && (f_olManager->currentFolder()==f_olManager->outbox()));
}


void KNodeView::slotCollectionSelected(QListViewItem *i)
{
  kdDebug(5003) << "KNodeView::slotCollectionSelected(QListViewItem *i)" << endl;
  if(b_lockui)
    return;
  KNCollection *c=0;
  KNNntpAccount *selectedAccount=0;
  KNGroup *selectedGroup=0;
  KNFolder *selectedFolder=0;
  QString caption;

  h_drView->clear();
  slotArticleSelected(0);

  if(i) {
    c=(static_cast<KNCollectionViewItem*>(i))->coll;
    switch(c->type()) {
      case KNCollection::CTnntpAccount :
        selectedAccount=static_cast<KNNntpAccount*>(c);
        if(!i->isOpen())
          i->setOpen(true);
        knGlobals.top->setCaption(selectedAccount->name());
      break;
      case KNCollection::CTgroup :
        if (!(h_drView->hasFocus())&&!(a_rtView->hasFocus()))
          h_drView->setFocus();
        selectedGroup=static_cast<KNGroup*>(c);
        selectedAccount=selectedGroup->account();
        caption = selectedGroup->name();
        if (selectedGroup->status()==KNGroup::moderated)
          caption += i18n(" (moderated)");
        knGlobals.top->setCaption(caption);
      break;

      case KNCollection::CTfolder :
        if (!(h_drView->hasFocus())&&!(a_rtView->hasFocus()))
          h_drView->setFocus();
        selectedFolder=static_cast<KNFolder*>(c);
        knGlobals.top->setCaption(selectedFolder->name());
      break;

      default: break;
    }
  }
  else
    knGlobals.top->setCaption(QString::null);

  a_ccManager->setCurrentAccount(selectedAccount);
  g_rpManager->setCurrentGroup(selectedGroup);
  f_olManager->setCurrentFolder(selectedFolder);
  if (!selectedGroup && !selectedFolder)         // called from showHeaders() otherwise
    a_rtManager->updateStatusString();

  //actions
  bool enabled;

  enabled=(selectedGroup) || (selectedFolder);
  if(a_ctNavNextArt->isEnabled() != enabled) {
    a_ctNavNextArt->setEnabled(enabled);
    a_ctNavPrevArt->setEnabled(enabled);
  }

  enabled=( selectedGroup!=0 );
  if(a_ctNavNextUnreadArt->isEnabled() != enabled) {
    a_ctNavNextUnreadArt->setEnabled(enabled);
    a_ctNavNextUnreadThread->setEnabled(enabled);
    a_ctNavReadThrough->setEnabled(enabled);
    a_ctFetchArticleWithID->setEnabled(enabled);
  }

  enabled=( selectedAccount!=0 );
  if(a_ctAccProperties->isEnabled() != enabled) {
    a_ctAccProperties->setEnabled(enabled);
    a_ctAccSubscribe->setEnabled(enabled);
    a_ctAccGetNewHdrs->setEnabled(enabled);
    a_ctAccDelete->setEnabled(enabled);
    a_ctAccPostNewArticle->setEnabled(enabled);
  }

  enabled=( selectedGroup!=0 );
  if(a_ctGrpProperties->isEnabled() != enabled) {
    a_ctGrpProperties->setEnabled(enabled);
    a_ctGrpGetNewHdrs->setEnabled(enabled);
    a_ctGrpExpire->setEnabled(enabled);
    a_ctGrpReorganize->setEnabled(enabled);
    a_ctGrpUnsubscribe->setEnabled(enabled);
    a_ctGrpSetAllRead->setEnabled(enabled);
    a_ctGrpSetAllUnread->setEnabled(enabled);
    a_ctArtFilter->setEnabled(enabled);
    a_ctArtFilterKeyb->setEnabled(enabled);
    a_ctArtSearch->setEnabled(enabled);
    a_ctArtRefreshList->setEnabled(enabled);
    a_ctArtCollapseAll->setEnabled(enabled);
    a_ctArtExpandAll->setEnabled(enabled);
    a_ctArtToggleShowThreads->setEnabled(enabled);
    a_ctReScore->setEnabled(enabled);
  }
  
  enabled=( selectedFolder!=0 );
  if(a_ctFolNewChild->isEnabled() != enabled) {
    a_ctFolNewChild->setEnabled(enabled);
    a_ctFolRename->setEnabled(enabled);
    a_ctFolMakeTopLevel->setEnabled(enabled);
    a_ctFolDelete->setEnabled(enabled);
    a_ctFolCompact->setEnabled(enabled);
    a_ctFolEmpty->setEnabled(enabled);
    a_ctFolMboxImport->setEnabled(enabled);
    a_ctFolMboxExport->setEnabled(enabled);
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


void KNodeView::slotArticleMousePressed(int button, QListViewItem *item, const QPoint &, int)
{
  if(b_lockui)
    return;

  if (item && (button == MidButton)) {
    KNArticle *art=(static_cast<KNHdrViewItem*>(item))->art;

    if ((art->type()==KNMimeBase::ATlocal) && ((f_olManager->currentFolder()==f_olManager->outbox())||
                                               (f_olManager->currentFolder()==f_olManager->drafts()))) {
      a_rtFactory->edit( static_cast<KNLocalArticle*>(art) );
    } else {
      if (!KNArticleWindow::raiseWindowForArticle(art)) {
        KNArticleWindow *w=new KNArticleWindow(art);
        w->show();
      }
    }
  }
}


void KNodeView::slotHdrViewSortingChanged(int i)
{
  a_ctArtSortHeaders->setCurrentItem(i);
}


void KNodeView::slotCollectionViewDrop(const char *type, QListViewItem *i)
{
  kdDebug(5003) << "KNodeView::slotCollectionViewDrop() : type = " << type << endl;
  KNCollectionViewItem *cvi=static_cast<KNCollectionViewItem*>(i);
  KNFolder *dest=cvi ? static_cast<KNFolder*>(cvi->coll) : 0;

  if( strcmp(type, "x-knode-drag/folder")==0 && f_olManager->currentFolder() ) {
    if( !f_olManager->moveFolder(f_olManager->currentFolder(), dest) )
      c_olView->triggerDropError(i18n("This folder cannot be moved here."));
  }
  else if(dest && strcmp(type, "x-knode-drag/article")==0) {
    if(f_olManager->currentFolder()) {
      KNLocalArticle::List l;
      getSelectedArticles(l);
      a_rtManager->saveInFolder(l, dest);
    }
    else if(g_rpManager->currentGroup()) {
      KNRemoteArticle::List l;
      getSelectedArticles(l);
      a_rtManager->saveInFolder(l, dest);
    }
  }

}


void KNodeView::slotHeaderViewDrop(const char *type, QListViewItem *)
{
  kdDebug(5003) << "KNodeView::slotHeaderViewDrop() : type = " << type << endl;
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
    h_drView->setActive(it, true);
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
    h_drView->setActive(it, true);
    h_drView->setCurrentItem(it);
    h_drView->ensureItemVisible(it);
  }
}



void KNodeView::slotNavNextUnreadArt()
{
  kdDebug(5003) << "KNodeView::slotNavNextUnreadArt()" << endl;

  if(!g_rpManager->currentGroup())
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

  if ((!current->isActive())&&(!art->isRead()))   // take current article, if unread & not selected
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
    h_drView->setActive(next, true);
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

  if(!g_rpManager->currentGroup())
    return;

  current=static_cast<KNHdrViewItem*>(h_drView->currentItem());
  if(!current)
    current=static_cast<KNHdrViewItem*>(h_drView->firstChild());

  if(!current) {               // no articles in the current group switch to next....
    slotNavNextGroup();
    return;
  }

  art=static_cast<KNRemoteArticle*>(current->art);

  if((current->depth()==0)&&((!current->isActive())&&(!art->isRead() || art->hasUnreadFollowUps())))
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
      h_drView->setActive(next, true);
      h_drView->setCurrentItem(next);
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
    if(!next->isActive())
      break;
    if(next->childCount()>0 && !next->isOpen()) {
      next->setOpen(true);
      knGlobals.top->secureProcessEvents();
      next=static_cast<KNCollectionViewItem*>(next->firstChild());
    }
    else next=static_cast<KNCollectionViewItem*>(next->itemBelow());
  }

  if(next) {
    c_olView->setActive(next, true);
    c_olView->setCurrentItem(next);
    c_olView->ensureItemVisible(next);
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
    if(!prev->isActive())
      break;
    prev=static_cast<KNCollectionViewItem*>(prev->itemAbove());
  }

  if(prev) {
    c_olView->setActive(prev, true);
    c_olView->setCurrentItem(prev);
    c_olView->ensureItemVisible(prev);
  }
}


void KNodeView::slotNavReadThrough()
{
  kdDebug(5003) << "KNodeView::slotNavReadThrough()" << endl;
  if (a_rtView->scrollingDownPossible())
    a_rtView->scrollDown();
  else if(g_rpManager->currentGroup() != 0)
    slotNavNextUnreadArt();
}


void KNodeView::slotAccProperties()
{
  kdDebug(5003) << "KNodeView::slotAccProperties()" << endl;
  if(a_ccManager->currentAccount())
    a_ccManager->editProperties(a_ccManager->currentAccount());
}


void KNodeView::slotAccSubscribe()
{
  kdDebug(5003) << "KNodeView::slotAccSubscribe()" << endl;
  if(a_ccManager->currentAccount())
    g_rpManager->showGroupDialog(a_ccManager->currentAccount());
}


void KNodeView::slotAccGetNewHdrs()
{
  kdDebug(5003) << "KNodeView::slotAccGetNewHdrs()" << endl;
  if(a_ccManager->currentAccount())
    g_rpManager->checkAll(a_ccManager->currentAccount());
}


void KNodeView::slotAccDelete()
{
  kdDebug(5003) << "KNodeView::slotAccDelete()" << endl;
  if(a_ccManager->currentAccount())
    a_ccManager->removeAccount(a_ccManager->currentAccount());
}


void KNodeView::slotAccPostNewArticle()
{
  kdDebug(5003) << "KNodeView::slotAccPostNewArticle()" << endl;
  if(g_rpManager->currentGroup())
    a_rtFactory->createPosting(g_rpManager->currentGroup());
  else if(a_ccManager->currentAccount())
    a_rtFactory->createPosting(a_ccManager->currentAccount());
}


void KNodeView::slotGrpProperties()
{
  kdDebug(5003) << "slotGrpProperties()" << endl;
  if(g_rpManager->currentGroup())
    g_rpManager->showGroupProperties(g_rpManager->currentGroup());
}


void KNodeView::slotGrpGetNewHdrs()
{
  kdDebug(5003) << "KNodeView::slotGrpGetNewHdrs()" << endl;
  if(g_rpManager->currentGroup())
    g_rpManager->checkGroupForNewHeaders(g_rpManager->currentGroup());
}


void KNodeView::slotGrpExpire()
{
  kdDebug(5003) << "KNodeView::slotGrpExpire()" << endl;
  if(g_rpManager->currentGroup())
    g_rpManager->expireGroupNow(g_rpManager->currentGroup());
}


void KNodeView::slotGrpReorganize()
{
  kdDebug(5003) << "KNodeView::slotGrpReorganize()" << endl;
  g_rpManager->reorganizeGroup(g_rpManager->currentGroup());
}


void KNodeView::slotGrpUnsubscribe()
{
  kdDebug(5003) << "KNodeView::slotGrpUnsubscribe()" << endl;
  if(g_rpManager->currentGroup())
    if(KMessageBox::Yes==KMessageBox::questionYesNo(knGlobals.topWidget,
      i18n("Do you really want to unsubscribe from %1?").arg(g_rpManager->currentGroup()->groupname())))
    g_rpManager->unsubscribeGroup(g_rpManager->currentGroup());
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


void KNodeView::slotFolNew()
{
  kdDebug(5003) << "KNodeView::slotFolNew()" << endl;
  f_olManager->newFolder(0);
}


void KNodeView::slotFolNewChild()
{
  kdDebug(5003) << "KNodeView::slotFolNew()" << endl;
  if(f_olManager->currentFolder())
    f_olManager->newFolder(f_olManager->currentFolder());
}


void KNodeView::slotFolDelete()
{
  kdDebug(5003) << "KNodeView::slotFolDelete()" << endl;

  if(!f_olManager->currentFolder())
    return;

  if(f_olManager->currentFolder()->id()<=3)
      KMessageBox::sorry(knGlobals.topWidget, i18n("You cannot delete a standard-folder."));

  else if( KMessageBox::Yes==KMessageBox::questionYesNo(knGlobals.topWidget,
      i18n("Do you really want to delete this folder and all it's children ?")) ) {

    if(!f_olManager->deleteFolder(f_olManager->currentFolder()))
      KMessageBox::sorry(knGlobals.topWidget,
      i18n("This Folder cannot be deleted because some of\n it's articles are currently in use.") );
    else
      slotCollectionSelected(0);
  }
}


void KNodeView::slotFolRename()
{
  kdDebug(5003) << "KNodeView::slotFolRename()" << endl;
  if(f_olManager->currentFolder()) {
    if(f_olManager->currentFolder()->id()<=3)
      KMessageBox::sorry(knGlobals.topWidget, i18n("You cannot rename a standard-folder."));
    else
      f_olManager->renameFolder(f_olManager->currentFolder());
  }
}


void KNodeView::slotFolMakeTopLevel()
{
  kdDebug(5003) << "KNodeView::slotFolMakeTopLevel()" << endl;
  if(f_olManager->currentFolder())
    f_olManager->moveFolder(f_olManager->currentFolder(), 0);
}


void KNodeView::slotFolCompact()
{
  kdDebug(5003) << "KNodeView::slotFolCompact()" << endl;
  if(f_olManager->currentFolder())
    f_olManager->compactFolder(f_olManager->currentFolder());
}


void KNodeView::slotFolEmpty()
{
  kdDebug(5003) << "KNodeView::slotFolEmpty()" << endl;
  if(f_olManager->currentFolder()) {
    if(f_olManager->currentFolder()->lockedArticles()>0) {
      KMessageBox::sorry(knGlobals.topWidget,
      i18n("This folder cannot be emptied at the moment\nbecause some of it's articles are currently in use.") );
      return;
    }
    if( KMessageBox::Yes == KMessageBox::questionYesNo(
        knGlobals.topWidget, i18n("Do you really want to delete all articles in %1?").arg(f_olManager->currentFolder()->name())) )
      f_olManager->emptyFolder(f_olManager->currentFolder());
  }
}


void KNodeView::slotFolMBoxImport()
{
  kdDebug(5003) << "KNodeView::slotFolMBoxImport()" << endl;
  if(f_olManager->currentFolder()) {
     f_olManager->currentFolder()->importMBoxFile();
  }
}


void KNodeView::slotFolMBoxExport()
{
  kdDebug(5003) << "KNodeView::slotFolMBoxExport()" << endl;
  if(f_olManager->currentFolder()) {
    f_olManager->currentFolder()->exportMBoxFile();
  }
}


void KNodeView::slotArtSortHeaders(int i)
{
  kdDebug(5003) << "KNodeView::slotArtSortHeaders(int i)" << endl;
  h_drView->slotSortList(i);
}


void KNodeView::slotArtSortHeadersKeyb()
{
  kdDebug(5003) << "KNodeView::slotArtSortHeadersKeyb()" << endl;

  int newCol = KNHelper::selectDialog(this, i18n("Select Sort Column"), a_ctArtSortHeaders->items(), a_ctArtSortHeaders->currentItem());
  if (newCol != -1)
    h_drView->slotSortList(newCol);
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

  // find the root of the current thread and make it current,
  // otherwise the current thread will not collapse
  if (a_rtView->article() && a_rtView->article()->listItem()) {
    QListViewItem *item = a_rtView->article()->listItem();
    while (item->parent())
      item = item->parent();
    h_drView->setCurrentItem(item);
  }

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
  if(a_rtView->article() && a_rtView->article()->listItem()->isExpandable()) {
    bool o=!(a_rtView->article()->listItem()->isOpen());
    a_rtView->article()->listItem()->setOpen(o);
  }
}


void KNodeView::slotArtToggleShowThreads()
{
  kdDebug(5003) << "KNodeView::slotArtToggleShowThreads()" << endl;
  if(g_rpManager->currentGroup()) {
    a_rtManager->toggleShowThreads();
  }
}


void KNodeView::slotArtSetArtRead()
{
  kdDebug(5003) << "KNodeView::slotArtSetArtRead()" << endl;
  if(!g_rpManager->currentGroup())
    return;

  KNRemoteArticle::List l;
  getSelectedArticles(l);
  a_rtManager->setRead(l, true);
}


void KNodeView::slotArtSetArtUnread()
{
  kdDebug(5003) << "KNodeView::slotArtSetArtUnread()" << endl;
  if(!g_rpManager->currentGroup())
    return;

  KNRemoteArticle::List l;
  getSelectedArticles(l);
  a_rtManager->setRead(l, false);
}


void KNodeView::slotArtSetThreadRead()
{
  kdDebug(5003) << "slotArtSetThreadRead()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->setRead(l, true);
}


void KNodeView::slotArtSetThreadUnread()
{
  kdDebug(5003) << "KNodeView::slotArtSetThreadUnread()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->setRead(l, false);
}


void KNodeView::slotScoreEdit()
{
  kdDebug(5003) << "KNodeView::slotScoreEdit()" << endl;
  s_coreManager->configure();
}


void KNodeView::slotReScore()
{
  kdDebug(5003) << "KNodeView::slotReScore()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  g_rpManager->currentGroup()->scoreArticles(false);
  a_rtManager->showHdrs(true);
}


void KNodeView::slotScoreLower()
{
  kdDebug(5003) << "KNodeView::slotScoreLower() start" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  if (a_rtView->article() && a_rtView->article()->type()==KNMimeBase::ATremote) {
    KNRemoteArticle *ra = static_cast<KNRemoteArticle*>(a_rtView->article());
    s_coreManager->addRule(KNScorableArticle(ra), g_rpManager->currentGroup()->groupname(), -10);
  }
}


void KNodeView::slotScoreRaise()
{
  kdDebug(5003) << "KNodeView::slotScoreRaise() start" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  if (a_rtView->article() && a_rtView->article()->type()==KNMimeBase::ATremote) {
    KNRemoteArticle *ra = static_cast<KNRemoteArticle*>(a_rtView->article());
    s_coreManager->addRule(KNScorableArticle(ra), g_rpManager->currentGroup()->groupname(), +10);
  }
}


void KNodeView::slotArtToggleIgnored()
{
  kdDebug(5003) << "KNodeView::slotArtToggleIgnored()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->toggleIgnored(l);
  a_rtManager->rescoreArticles(l);
}


void KNodeView::slotArtToggleWatched()
{
  kdDebug(5003) << "KNodeView::slotArtToggleWatched()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->toggleWatched(l);
  a_rtManager->rescoreArticles(l);
}


void KNodeView::slotArtOpenNewWindow()
{
  kdDebug(5003) << "KNodeView::slotArtOpenNewWindow()" << endl;

  if(a_rtView->article()) {
    if (!KNArticleWindow::raiseWindowForArticle(a_rtView->article())) {
      KNArticleWindow *win=new KNArticleWindow(a_rtView->article());
      win->show();
    }
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
  if (!f_olManager->currentFolder())
    return;

  KNLocalArticle::List lst;
  getSelectedArticles(lst);

  if(!lst.isEmpty())
    a_rtManager->deleteArticles(lst);

  if(h_drView->currentItem())
    h_drView->setActive(h_drView->currentItem(),true);
}


void KNodeView::slotArtSendNow()
{
  kdDebug(5003) << "KNodeView::slotArtSendNow()" << endl;
  if (!f_olManager->currentFolder())
    return;

  KNLocalArticle::List lst;
  getSelectedArticles(lst);

  if(!lst.isEmpty())
    a_rtFactory->sendArticles(&lst, true);
}


void KNodeView::slotArtEdit()
{
  kdDebug(5003) << "KNodeVew::slotArtEdit()" << endl;
  if (!f_olManager->currentFolder())
    return;

  if (a_rtView->article() && a_rtView->article()->type()==KNMimeBase::ATlocal)
    a_rtFactory->edit(static_cast<KNLocalArticle*>(a_rtView->article()));
}


void KNodeView::slotNetCancel()
{
  kdDebug(5003) << "KNodeView::slotNetCancel()" << endl;
  n_etAccess->cancelAllJobs();
}


void KNodeView::slotFetchArticleWithID()
{
  kdDebug(5003) << "KNodeView::slotFetchArticleWithID()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KDialogBase *dlg =  new KDialogBase(this, 0, true, i18n("Fetch Article with ID"), KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok);
  QHBox *page = dlg->makeHBoxMainWidget();

  QLabel *label = new QLabel(i18n("&Message-ID:"),page);
  QLineEdit *edit = new QLineEdit(page);
  label->setBuddy(edit);
  edit->setFocus();
  KNHelper::restoreWindowSize("fetchArticleWithID", dlg, QSize(325,66));

  if (dlg->exec()) {
    QString id = edit->text().simplifyWhiteSpace();
    if (id.find(QRegExp("*@*",false,true))!=-1) {
      if (id.find(QRegExp("<*>",false,true))==-1)   // add "<>" when necessary
        id = QString("<%1>").arg(id);

      if(!KNArticleWindow::raiseWindowForArticle(id.latin1())) { //article not yet opened
        KNRemoteArticle *a=new KNRemoteArticle(g_rpManager->currentGroup());
        a->messageID()->from7BitString(id.latin1());
        KNArticleWindow *awin=new KNArticleWindow(a);
        awin->show();
      }
    }
  }

  KNHelper::saveWindowSize("fetchArticleWithID",dlg->size());
  delete dlg;
}

//-------------------------------- </Actions> ----------------------------------


#include "knodeview.moc"
