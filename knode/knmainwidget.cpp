/*
    knmainwidget.cpp

    KNode, the KDE newsreader
    Copyright (c) 2003 Zack Rusin <zack@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/
#include "knmainwidget.h"

#include <qhbox.h>
#include <qinputdialog.h>

#include <kaccel.h>
#include <kxmlguiclient.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <kmenubar.h>
#include <kiconloader.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kapplication.h>
#include "recentaddresses.h"
using KRecentAddress::RecentAddresses;
//GUI
#include "knmainwidget.h"
#include "knarticlewidget.h"
#include "knarticlewindow.h"
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"

//Core
#include "knglobals.h"
#include "knconfigmanager.h"
#include "knarticlemanager.h"
#include "knarticlefactory.h"
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
#include <kpgp.h>
#include "knmemorymanager.h"

KNGlobals knGlobals;

KNMainWidget::KNMainWidget( KXMLGUIClient* client, bool detachable, QWidget* parent,
                            const char* name  )
  : DCOPObject("KNodeIface"), KDockArea( parent, name ),
    b_lockui( false ), m_GUIClient( client ),
    b_popupInitialized( false )
{
  knGlobals.top=this;
  knGlobals.topWidget=this;
  if ( detachable )
    kapp->setMainWidget(this);  // this makes the external viewer windows close on shutdown...

  //------------------------------- <CONFIG> ----------------------------------
  c_fgManager = new KNConfigManager();
  knGlobals.cfgManager=c_fgManager;
  //------------------------------- </CONFIG> ----------------------------------

  //-------------------------------- <GUI> ------------------------------------
  initStatusBar();

  //setup splitter behavior
  manager()->setSplitterHighResolution(true);
  manager()->setSplitterOpaqueResize(true);
  manager()->setSplitterKeepSize(true);

  //article view
  a_rtDock = createDockWidget("article_viewer", SmallIcon("contents"), 0,
                              kapp->makeStdCaption(i18n("Article Viewer")), i18n("Article Viewer"));
  if (!detachable) {
    a_rtDock->setEnableDocking(KDockWidget::DockFullSite);
  }
  KDockWidgetHeader *header = new KDockWidgetHeader(a_rtDock, "artDockHeader");
  a_rtDock->setHeader(header);
  a_rtView=new KNArticleWidget( actionCollection(), a_rtDock ,"artView");
  header->setDragPanel(new KNDockWidgetHeaderDrag(a_rtView, header, a_rtDock));
  knGlobals.artWidget=a_rtView;
  a_rtDock->setWidget(a_rtView);
  //setView(a_rtDock);
  setMainDockWidget(a_rtDock);

  connect(a_rtDock, SIGNAL(iMBeingClosed()), SLOT(slotArticleDockHidden()));
  connect(a_rtDock, SIGNAL(hasUndocked()), SLOT(slotArticleDockHidden()));
  connect(a_rtView, SIGNAL(focusChangeRequest(QWidget *)), SLOT(slotDockWidgetFocusChangeRequest(QWidget *)));
  // KMail emulation...
  connect(a_rtView, SIGNAL(keyLeftPressed()), SLOT(slotNavPrevArt()));
  connect(a_rtView, SIGNAL(keyRightPressed()), SLOT(slotNavNextArt()));

  //collection view
  c_olDock = createDockWidget("group_view", UserIcon("group"), 0,
                              kapp->makeStdCaption(i18n("Group View")), i18n("Group View"));
  if (!detachable) {
    c_olDock->setEnableDocking(KDockWidget::DockFullSite);
  }
  header = new KDockWidgetHeader(c_olDock, "colDockHeader");
  c_olDock->setHeader(header);
  c_olView=new KNListView( this, "collectionView");
  header->setDragPanel(new KNDockWidgetHeaderDrag(c_olView, header, c_olDock));
  c_olDock->setWidget(c_olView);
  c_olDock->manualDock(a_rtDock, KDockWidget::DockLeft, 3000);

  c_olView->setAcceptDrops(true);
  c_olView->setDragEnabled(true);
  c_olView->addAcceptableDropMimetype("x-knode-drag/article", false);
  c_olView->addAcceptableDropMimetype("x-knode-drag/folder", true);
  c_olView->setSelectionModeExt(KListView::Single);
  c_olView->setFrameStyle(QFrame::Panel | QFrame::Plain);
  c_olView->setLineWidth(1);
  c_olView->setTreeStepSize(12);
  c_olView->setRootIsDecorated(true);
  c_olView->setShowSortIndicator(true);
  c_olView->addColumn(i18n("Name"),162);
  c_olView->addColumn(i18n("Total"),36);
  c_olView->addColumn(i18n("Unread"),48);
  c_olView->setColumnAlignment(1,AlignCenter);
  c_olView->setColumnAlignment(2,AlignCenter);
  c_olView->setAlternateBackground(QColor());

  connect(c_olDock, SIGNAL(iMBeingClosed()), SLOT(slotGroupDockHidden()));
  connect(c_olDock, SIGNAL(hasUndocked()), SLOT(slotGroupDockHidden()));
  connect(c_olView, SIGNAL(focusChangeRequest(QWidget *)), SLOT(slotDockWidgetFocusChangeRequest(QWidget *)));
  connect(c_olView, SIGNAL(itemSelected(QListViewItem*)),
          SLOT(slotCollectionSelected(QListViewItem*)));
  connect(c_olView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
          SLOT(slotCollectionRMB(QListViewItem*, const QPoint&, int)));
  connect(c_olView, SIGNAL(dropped(QDropEvent*, QListViewItem*)),
          SLOT(slotCollectionViewDrop(QDropEvent*, QListViewItem*)));
  connect(c_olView, SIGNAL(itemRenamed(QListViewItem*)),
          SLOT(slotCollectionRenamed(QListViewItem*)));
  connect(c_olView, SIGNAL(reparented()), SLOT(slotReparented()));
  // KMail emulation...
  connect(c_olView, SIGNAL(keyLeftPressed()), SLOT(slotNavPrevArt()));
  connect(c_olView, SIGNAL(keyRightPressed()), SLOT(slotNavNextArt()));
  connect(c_olView, SIGNAL(keyUpPressed()), a_rtView, SLOT(slotKeyUp()));
  connect(c_olView, SIGNAL(keyDownPressed()), a_rtView, SLOT(slotKeyDown()));
  connect(c_olView, SIGNAL(keyPriorPressed()), a_rtView, SLOT(slotKeyPrior()));
  connect(c_olView, SIGNAL(keyNextPressed()), a_rtView, SLOT(slotKeyNext()));

  //header view
  h_drDock = createDockWidget("header_view", SmallIcon("text_block"), 0,
                              kapp->makeStdCaption(i18n("Header View")), i18n("Header View"));
  if (!detachable) {
    h_drDock->setEnableDocking(KDockWidget::DockFullSite);
  }
  header = new KDockWidgetHeader(h_drDock, "headerDockHeader");
  h_drDock->setHeader(header);
  h_drView=new KNListView(h_drDock, "hdrView");
  header->setDragPanel(new KNDockWidgetHeaderDrag(h_drView, header, h_drDock));
  h_drDock->setWidget(h_drView);
  h_drDock->manualDock(a_rtDock, KDockWidget::DockTop, 5000);

  h_drView->setAcceptDrops(false);
  h_drView->setDragEnabled(true);
  h_drView->setSelectionModeExt(KListView::Extended);
  h_drView->setFrameStyle(QFrame::Panel | QFrame::Plain);
  h_drView->setLineWidth(1);
  h_drView->setShowSortIndicator(true);
  h_drView->setRootIsDecorated(true);
  h_drView->addColumn(i18n("Subject"),207);
  h_drView->addColumn(i18n("From"),115);
  h_drView->addColumn(i18n("Score"),42);
  h_drView->addColumn(i18n("Lines"),42);
  h_drView->addColumn(i18n("Date"),102);

  connect(h_drDock, SIGNAL(iMBeingClosed()), SLOT(slotHeaderDockHidden()));
  connect(h_drDock, SIGNAL(hasUndocked()), SLOT(slotHeaderDockHidden()));
  connect(h_drView, SIGNAL(focusChangeRequest(QWidget *)),
          SLOT(slotDockWidgetFocusChangeRequest(QWidget *)));
  connect(h_drView, SIGNAL(itemSelected(QListViewItem*)),
          SLOT(slotArticleSelected(QListViewItem*)));
  connect(h_drView, SIGNAL(selectionChanged()),
          SLOT(slotArticleSelectionChanged()));
  connect(h_drView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
          SLOT(slotArticleRMB(QListViewItem*, const QPoint&, int)));
  connect(h_drView, SIGNAL(middleMBClick(QListViewItem *)),
          SLOT(slotArticleMMB(QListViewItem *)));
  connect(h_drView, SIGNAL(sortingChanged(int)),
          SLOT(slotHdrViewSortingChanged(int)));
  // KMail emulation...
  connect(h_drView, SIGNAL(keyLeftPressed()), SLOT(slotNavPrevArt()));
  connect(h_drView, SIGNAL(keyRightPressed()), SLOT(slotNavNextArt()));
  connect(h_drView, SIGNAL(keyUpPressed()), a_rtView, SLOT(slotKeyUp()));
  connect(h_drView, SIGNAL(keyDownPressed()), a_rtView, SLOT(slotKeyDown()));
  connect(h_drView, SIGNAL(keyPriorPressed()), a_rtView, SLOT(slotKeyPrior()));
  connect(h_drView, SIGNAL(keyNextPressed()), a_rtView, SLOT(slotKeyNext()));

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
  //connect(s_coreManager, SIGNAL(changedRules()), SLOT(slotReScore()));
  connect(s_coreManager, SIGNAL(finishedEditing()), SLOT(slotReScore()));

  // Memory Manager
  m_emManager = new KNMemoryManager();
  knGlobals.memManager = m_emManager;

  // create a global pgp instance
  p_gp = new Kpgp::Module();
  knGlobals.pgp = p_gp;

  //-------------------------------- </CORE> -----------------------------------

  //apply saved options
  readOptions();

  //apply configuration
  configChanged();

  // set the keyboard focus indicator on the first item in the Collection View
  if( c_olView->firstChild() ) {
    QListViewItem *i = c_olView->firstChild();
    bool open = i->isOpen();
    c_olView->setActive( i,true );
    i->setOpen( open );
  }

  c_olView->setFocus();

  setStatusMsg();

  if( firstStart() ) {  // open the config dialog on the first start
    show();              // the settings dialog must appear in front of the main window!
    slotSettings();
  }
}

KNMainWidget::~KNMainWidget()
{
  delete a_ccel;

  h_drView->clear(); //avoid some random crashes in KNHdrViewItem::~KNHdrViewItem()

  delete n_etAccess;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : Net deleted" << endl;

  delete a_rtManager;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : Article Manager deleted" << endl;

  delete a_rtFactory;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : Article Factory deleted" << endl;

  delete g_rpManager;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : Group Manager deleted" << endl;

  delete f_olManager;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : Folder Manager deleted" << endl;

  delete f_ilManager;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : Filter Manager deleted" << endl;

  delete a_ccManager;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : Account Manager deleted" << endl;

  delete c_fgManager;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : Config deleted" << endl;

  delete s_coreManager;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : Score Manager deleted" << endl;

  delete m_emManager;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : Memory Manager deleted" << endl;

  delete p_gp;
  kdDebug(5003) << "KNMainWidget::~KNMainWidget() : PGP deleted" << endl;

  delete c_olDock;
  delete h_drDock;
  delete a_rtDock;
}

void KNMainWidget::initStatusBar()
{
  //statusbar
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KStatusBar *sb =  mainWin ? mainWin->statusBar() : 0;
  p_rogBar = new KNProgress( sb->sizeHint().height()-4, 1000, 0, sb );
  knGlobals.progressBar = p_rogBar;
  sb->addWidget( p_rogBar );
  sb->insertItem( QString::null, SB_MAIN,2 );
  sb->setItemAlignment( SB_MAIN,AlignLeft | AlignVCenter );
  sb->insertItem( QString::null, SB_FILTER, 2 );
  sb->setItemAlignment( SB_FILTER,AlignLeft | AlignVCenter );
  s_tatusGroup = new KStatusBarLabel( QString::null, SB_GROUP, sb );
  s_tatusGroup->setAlignment( AlignLeft | AlignVCenter );
  sb->addWidget( s_tatusGroup, 3 );
}

//================================== GUI =================================

void KNMainWidget::setStatusMsg(const QString& text, int id)
{
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KStatusBar *bar =  mainWin ? mainWin->statusBar() : 0;
  if ( !bar )
    return;
  bar->clear();
  if (text.isEmpty() && (id==SB_MAIN))
    if (knGlobals.netAccess->currentMsg().isEmpty())
      bar->changeItem(i18n(" Ready"),SB_MAIN);
    else
      bar->changeItem(knGlobals.netAccess->currentMsg(), SB_MAIN);   // restore the original message
  else if ( id == SB_GROUP ) {
    int statusWidth = s_tatusGroup->width();
    QString mtext = text;
    while (!mtext.isEmpty() && fontMetrics().width( mtext ) >= statusWidth) {
      mtext.truncate( mtext.length() - 1);
    }
    s_tatusGroup->setText( mtext );
  }
  else {
    bar->changeItem(text, id);
  }
}


void KNMainWidget::setStatusHelpMsg(const QString& text)
{
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KStatusBar *bar =  mainWin ? mainWin->statusBar() : 0;
  bar->message(text, 2000);
}


void KNMainWidget::updateCaption()
{
  QString newCaption=i18n("KDE News Reader");
  if (g_rpManager->currentGroup()) {
    newCaption = g_rpManager->currentGroup()->name();
    if (g_rpManager->currentGroup()->status()==KNGroup::moderated)
      newCaption += i18n(" (moderated)");
  } else if (a_ccManager->currentAccount()) {
    newCaption = a_ccManager->currentAccount()->name();
  } else if (f_olManager->currentFolder()) {
    newCaption = f_olManager->currentFolder()->name();
  }
  emit signalCaptionChangeRequest(newCaption);
}


void KNMainWidget::setCursorBusy(bool b)
{
  if(b) KApplication::setOverrideCursor(waitCursor);
  else  KApplication::restoreOverrideCursor();
}


void KNMainWidget::blockUI(bool b)
{
  b_lockui = b;
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KMenuBar *mbar =  mainWin ? mainWin->menuBar() : 0;
  if ( mbar )
    mbar->setEnabled(!b);
  a_ccel->setEnabled(!b);
  KAccel *naccel = mainWin ? mainWin->accel() : 0;
  if ( naccel )
    naccel->setEnabled(!b);
  if (b)
    installEventFilter(this);
  else
    removeEventFilter(this);
  setCursorBusy(b);
}


void KNMainWidget::disableAccels(bool b)
{
  a_ccel->setEnabled(!b);
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KAccel *naccel = mainWin ? mainWin->accel() : 0;
  if ( naccel )
    naccel->setEnabled(!b);
  if (b)
    installEventFilter(this);
  else
    removeEventFilter(this);
}


// processEvents with some blocking
void KNMainWidget::secureProcessEvents()
{
  b_lockui = true;
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KMenuBar *mbar =  mainWin ? mainWin->menuBar() : 0;
  if ( mbar )
    mbar->setEnabled(false);
  a_ccel->setEnabled(false);
  KAccel *naccel = mainWin ? mainWin->accel() : 0;
  if ( naccel )
    naccel->setEnabled(false);
  installEventFilter(this);

  kapp->processEvents();

  b_lockui = false;
  if ( mbar )
    mbar->setEnabled(true);
  a_ccel->setEnabled(true);
  if ( naccel )
    naccel->setEnabled(true);
  removeEventFilter(this);
}


QSize KNMainWidget::sizeHint() const
{
  return QSize(759,478);    // default optimized for 800x600
}


void KNMainWidget::openURL(const KURL &url)
{
  QString host = url.host();
  unsigned short int port = url.port();
  KNNntpAccount *acc=0;

  if (url.url().left(7) == "news://") {

    // lets see if we already have an account for this host...
    for(acc=a_ccManager->first(); acc; acc=a_ccManager->next())
      if( acc->server()==host && (port==0 || acc->port()==port) )
        break;

    if(!acc) {
      acc=new KNNntpAccount();
      acc->setName(host);
      acc->setServer(host);

      if(port!=0)
        acc->setPort(port);

      if(url.hasUser() && url.hasPass()) {
        acc->setNeedsLogon(true);
        acc->setUser(url.user());
        acc->setPass(url.pass());
      }

      if(!a_ccManager->newAccount(acc))
        return;
    }
  } else {
    if (url.url().left(5) == "news:") {
      // TODO: make the default server configurable
      acc=a_ccManager->currentAccount();
      if (acc == 0)
        acc=a_ccManager->first();
    } else {
      kdDebug(5003) << "KNMainWidget::openURL() URL is not a valid news URL" << endl;
    }
  }

  if (acc) {
    bool isMID=(url.url().contains('@')==1);

    if (!isMID) {
      QString groupname=url.path(-1);
      while(groupname.startsWith("/"))
        groupname.remove(0,1);
      QListViewItem *item=0;
      if(groupname.isEmpty())
        item=acc->listItem();
      else {
        KNGroup *grp= g_rpManager->group(groupname, acc);

        if(!grp) {
          KNGroupInfo inf(groupname, "");
          g_rpManager->subscribeGroup(&inf, acc);
          grp=g_rpManager->group(groupname, acc);
          if(grp)
            item=grp->listItem();
        }
        else
          item=grp->listItem();
      }

      if (item) {
        c_olView->ensureItemVisible(item);
        c_olView->setActive(item, true);
      }
    } else {
      QString groupname = url.url().mid( url.protocol().length()+1 );
      KNGroup *g = g_rpManager->currentGroup();
      if (g == 0)
        g = g_rpManager->firstGroupOfAccount(acc);

      if (g) {
        if(!KNArticleWindow::raiseWindowForArticle(groupname.latin1())) { //article not yet opened
          KNRemoteArticle *a=new KNRemoteArticle(g);
          QString messageID = "<"+groupname+">";
          a->messageID()->from7BitString(messageID.latin1());
          KNArticleWindow *awin=new KNArticleWindow(a);
          awin->show();
        }
      } else {
        // TODO: fetch without group
        kdDebug(5003) << "KNMainWidget::openURL() account has no groups" << endl;
      }
    }
  }
}


// update fonts and colors
void KNMainWidget::configChanged()
{
  KNConfig::Appearance *app=c_fgManager->appearance();

  c_olView->setFont(app->groupListFont());
  h_drView->setFont(app->articleListFont());

  QPalette p = palette();
  p.setColor(QColorGroup::Base, app->backgroundColor());
  p.setColor(QColorGroup::Text, app->textColor());
  c_olView->setPalette(p);
  c_olView->setAlternateBackground(app->backgroundColor());
  h_drView->setPalette(p);
  h_drView->setAlternateBackground(app->alternateBackgroundColor());

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


void KNMainWidget::initActions()
{
  a_ccel=new KAccel(this);
  a_rtView->setCharsetKeyboardAction()->plugAccel(a_ccel);

  //navigation
  a_ctNavNextArt            = new KAction(i18n("&Next Article"), "next", Key_N , this,
                              SLOT(slotNavNextArt()), actionCollection(), "go_nextArticle");
  a_ctNavPrevArt            = new KAction(i18n("&Previous Article"), "previous", Key_B , this,
                              SLOT(slotNavPrevArt()), actionCollection(), "go_prevArticle");
  a_ctNavNextUnreadArt      = new KAction(i18n("Next Unread &Article"), "1rightarrow", ALT+Key_Space , this,
                              SLOT(slotNavNextUnreadArt()), actionCollection(), "go_nextUnreadArticle");
  a_ctNavNextUnreadThread   = new KAction(i18n("Next Unread &Thread"),"2rightarrow", CTRL+Key_Space , this,
                              SLOT(slotNavNextUnreadThread()), actionCollection(), "go_nextUnreadThread");
  a_ctNavNextGroup          = new KAction(i18n("Ne&xt Group"), "down", Key_Plus , this,
                              SLOT(slotNavNextGroup()), actionCollection(), "go_nextGroup");
  a_ctNavPrevGroup          = new KAction(i18n("Pre&vious Group"), "up", Key_Minus , this,
                              SLOT(slotNavPrevGroup()), actionCollection(), "go_prevGroup");
  a_ctNavReadThrough        = new KAction(i18n("Read &Through Articles"), Key_Space , this,
                              SLOT(slotNavReadThrough()), actionCollection(), "go_readThrough");
  a_ctNavReadThrough->plugAccel(a_ccel);

  //collection-view - accounts
  a_ctAccProperties         = new KAction(i18n("Account &Properties..."), "configure", 0, this,
                              SLOT(slotAccProperties()), actionCollection(), "account_properties");
  a_ctAccRename             = new KAction(i18n("&Rename Account"), "text", 0, this,
                              SLOT(slotAccRename()), actionCollection(), "account_rename");
  a_ctAccSubscribe          = new KAction(i18n("&Subscribe to Newsgroups..."), "news_subscribe", 0, this,
                              SLOT(slotAccSubscribe()), actionCollection(), "account_subscribe");
  a_ctAccExpireAll          = new KAction(i18n("&Expire All Groups"), 0, this,
                              SLOT(slotAccExpireAll()), actionCollection(), "account_expire_all");
  a_ctAccGetNewHdrs         = new KAction(i18n("&Get New Articles in All Groups"), "mail_get", 0, this,
                              SLOT(slotAccGetNewHdrs()), actionCollection(), "account_dnlHeaders");
  a_ctAccDelete             = new KAction(i18n("&Delete Account"), "editdelete", 0, this,
                              SLOT(slotAccDelete()), actionCollection(), "account_delete");
  a_ctAccPostNewArticle     = new KAction(i18n("&Post to Newsgroup..."), "mail_new", Key_P , this,
                              SLOT(slotAccPostNewArticle()), actionCollection(), "article_postNew");

  //collection-view - groups
  a_ctGrpProperties         = new KAction(i18n("Group &Properties..."), "configure", 0, this,
                              SLOT(slotGrpProperties()), actionCollection(), "group_properties");
  a_ctGrpRename             = new KAction(i18n("Rename &Group"), "text", 0, this,
                              SLOT(slotGrpRename()), actionCollection(), "group_rename");
  a_ctGrpGetNewHdrs         = new KAction(i18n("&Get New Articles"), "mail_get" , 0, this,
                              SLOT(slotGrpGetNewHdrs()), actionCollection(), "group_dnlHeaders");
  a_ctGrpExpire             = new KAction(i18n("E&xpire Group"), "wizard", 0, this,
                              SLOT(slotGrpExpire()), actionCollection(), "group_expire");
  a_ctGrpReorganize         = new KAction(i18n("Re&organize Group"), 0, this,
                              SLOT(slotGrpReorganize()), actionCollection(), "group_reorg");
  a_ctGrpUnsubscribe        = new KAction(i18n("&Unsubscribe From Group"), "news_unsubscribe", 0, this,
                              SLOT(slotGrpUnsubscribe()), actionCollection(), "group_unsubscribe");
  a_ctGrpSetAllRead         = new KAction(i18n("Mark All as &Read"), "goto", 0, this,
                              SLOT(slotGrpSetAllRead()), actionCollection(), "group_allRead");
  a_ctGrpSetAllUnread       = new KAction(i18n("Mark All as U&nread"), 0, this,
                              SLOT(slotGrpSetAllUnread()), actionCollection(), "group_allUnread");
  a_ctGrpSetUnread          = new KAction(i18n("Mark Last as Unr&ead..."), 0, this,
                              SLOT(slotGrpSetUnread()), actionCollection(), "group_unread");



   (void) new KAction( i18n("&Configure KNode..."),
                       "configure", 0, this,
                       SLOT(slotSettings()), actionCollection(),
                       "knode_configure_knode" );
   (void) new KAction( i18n("Configure S&hortcuts..."),
                       "configure_shortcuts", 0, this,
                       SLOT(slotConfKeys()), actionCollection(),
                       "knode_configure_shortcuts" );

  //collection-view - folder
  a_ctFolNew                = new KAction(i18n("&New Folder"), "folder_new", 0, this,
                              SLOT(slotFolNew()), actionCollection(), "folder_new");
  a_ctFolNewChild           = new KAction(i18n("New &Subfolder"), "folder_new", 0, this,
                              SLOT(slotFolNewChild()), actionCollection(), "folder_newChild");
  a_ctFolDelete             = new KAction(i18n("&Delete Folder"), "editdelete", 0, this,
                              SLOT(slotFolDelete()), actionCollection(), "folder_delete");
  a_ctFolRename             = new KAction(i18n("&Rename Folder"), "text", 0, this,
                              SLOT(slotFolRename()), actionCollection(), "folder_rename");
  a_ctFolCompact            = new KAction(i18n("C&ompact Folder"), "wizard", 0, this,
                              SLOT(slotFolCompact()), actionCollection(), "folder_compact");
  a_ctFolCompactAll         = new KAction(i18n("Co&mpact All Folders"), 0, this,
                              SLOT(slotFolCompactAll()), actionCollection(), "folder_compact_all");
  a_ctFolEmpty              = new KAction(i18n("&Empty Folder"), 0, this,
                              SLOT(slotFolEmpty()), actionCollection(), "folder_empty");
  a_ctFolMboxImport         = new KAction(i18n("&Import MBox Folder..."), 0, this,
                              SLOT(slotFolMBoxImport()), actionCollection(), "folder_MboxImport");
  a_ctFolMboxExport         = new KAction(i18n("E&xport as MBox Folder..."), 0, this,
                              SLOT(slotFolMBoxExport()), actionCollection(), "folder_MboxExport");

  //header-view - list-handling
  a_ctArtSortHeaders        = new KSelectAction(i18n("S&ort"), 0, actionCollection(), "view_Sort");
  QStringList items;
  items += i18n("By &Subject");
  items += i18n("By S&ender");
  items += i18n("By S&core");
  items += i18n("By &Lines");
  items += i18n("By &Date");
  a_ctArtSortHeaders->setItems(items);
  a_ctArtSortHeaders->setShortcutConfigurable(false);
  connect(a_ctArtSortHeaders, SIGNAL(activated(int)), this, SLOT(slotArtSortHeaders(int)));
  a_ctArtSortHeadersKeyb   = new KAction(i18n("Sort"), QString::null, Key_F7 , this,
                             SLOT(slotArtSortHeadersKeyb()), actionCollection(), "view_Sort_Keyb");
  a_ctArtSortHeadersKeyb->plugAccel(a_ccel);
  a_ctArtFilter             = new KNFilterSelectAction(i18n("&Filter"), "filter",
                              actionCollection(), "view_Filter");
  a_ctArtFilter->setShortcutConfigurable(false);
  a_ctArtFilterKeyb         = new KAction(i18n("Filter"), Key_F6, actionCollection(), "view_Filter_Keyb");
  a_ctArtFilterKeyb->plugAccel(a_ccel);
  a_ctArtSearch             = new KAction(i18n("&Search Articles..."),"mail_find" , Key_F4 , this,
                              SLOT(slotArtSearch()), actionCollection(), "article_search");
  a_ctArtFind               = new KAction(i18n("F&ind in Article..."),KStdAccel::shortcut(KStdAccel::Find) , this,
                              SLOT(slotArtFind()), actionCollection(), "find_in_article");
  a_ctArtRefreshList        = new KAction(i18n("&Refresh List"),"reload", KStdAccel::shortcut(KStdAccel::Reload), this,
                              SLOT(slotArtRefreshList()), actionCollection(), "view_Refresh");
  a_ctArtCollapseAll        = new KAction(i18n("&Collapse All Threads"), 0 , this,
                              SLOT(slotArtCollapseAll()), actionCollection(), "view_CollapseAll");
  a_ctArtExpandAll          = new KAction(i18n("E&xpand All Threads"), 0 , this,
                              SLOT(slotArtExpandAll()), actionCollection(), "view_ExpandAll");
  a_ctArtToggleThread       = new KAction(i18n("&Toggle Subthread"), Key_T, this,
                              SLOT(slotArtToggleThread()), actionCollection(), "thread_toggle");
  a_ctArtToggleShowThreads  = new KToggleAction(i18n("Show T&hreads"), 0 , this,
                              SLOT(slotArtToggleShowThreads()), actionCollection(), "view_showThreads");
  a_ctArtToggleShowThreads->setChecked(c_fgManager->readNewsGeneral()->showThreads());

  //header-view - remote articles
  a_ctArtSetArtRead         = new KAction(i18n("Mark as &Read"), Key_D , this,
                              SLOT(slotArtSetArtRead()), actionCollection(), "article_read");
  a_ctArtSetArtUnread       = new KAction(i18n("Mar&k as Unread"), Key_U , this,
                              SLOT(slotArtSetArtUnread()), actionCollection(), "article_unread");
  a_ctArtSetThreadRead      = new KAction(i18n("Mark &Thread as Read"), CTRL+Key_D , this,
                              SLOT(slotArtSetThreadRead()), actionCollection(), "thread_read");
  a_ctArtSetThreadUnread    = new KAction(i18n("Mark T&hread as Unread"), CTRL+Key_U , this,
                              SLOT(slotArtSetThreadUnread()), actionCollection(), "thread_unread");
  a_ctArtOpenNewWindow      = new KAction(i18n("Open in Own &Window"), "window_new", Key_O , this,
                              SLOT(slotArtOpenNewWindow()), actionCollection(), "article_ownWindow");

  // scoring
  a_ctScoresEdit            = new KAction(i18n("&Edit Scoring Rules..."), "edit", CTRL+Key_E, this,
                              SLOT(slotScoreEdit()), actionCollection(), "scoreedit");
  a_ctReScore               = new KAction(i18n("Recalculate &Scores"), 0, this,
                              SLOT(slotReScore()),actionCollection(),"rescore");
  a_ctScoreLower            = new KAction(i18n("&Lower Score for Author..."), CTRL+Key_L, this,
                              SLOT(slotScoreLower()), actionCollection(), "scorelower");
  a_ctScoreRaise            = new KAction(i18n("&Raise Score for Author..."), CTRL+Key_I, this,
                              SLOT(slotScoreRaise()),actionCollection(),"scoreraise");
  a_ctArtToggleIgnored      = new KAction(i18n("&Ignore Thread"), "bottom", Key_I , this,
                              SLOT(slotArtToggleIgnored()), actionCollection(), "thread_ignore");
  a_ctArtToggleWatched      = new KAction(i18n("&Watch Thread"), "top", Key_W , this,
                              SLOT(slotArtToggleWatched()), actionCollection(), "thread_watch");

  //header-view local articles
  a_ctArtSendOutbox         = new KAction(i18n("Sen&d Pending Messages"), "mail_send", 0, this,
                              SLOT(slotArtSendOutbox()), actionCollection(), "net_sendPending");
  a_ctArtDelete             = new KAction(i18n("&Delete Article"), "editdelete", Key_Delete, this,
                              SLOT(slotArtDelete()), actionCollection(), "article_delete");
  a_ctArtSendNow            = new KAction(i18n("Send &Now"),"mail_send", 0 , this,
                              SLOT(slotArtSendNow()), actionCollection(), "article_sendNow");
  a_ctArtEdit               = new KAction(i18n("edit article","&Edit Article..."), "edit", Key_E , this,
                              SLOT(slotArtEdit()), actionCollection(), "article_edit");

  //network
  a_ctNetCancel             = new KAction(i18n("Stop &Network"),"stop",0, this,
                              SLOT(slotNetCancel()), actionCollection(), "net_stop");
  a_ctNetCancel->setEnabled(false);

  a_ctFetchArticleWithID    = new KAction(i18n("&Fetch Article with ID..."), 0, this,
                              SLOT(slotFetchArticleWithID()), actionCollection(), "fetch_article_with_id");
  a_ctFetchArticleWithID->setEnabled(false);

  a_ctToggleGroupView        = new KToggleAction(i18n("Show &Group View"), CTRL+Key_G, this,
                               SLOT(slotToggleGroupView()), actionCollection(), "settings_show_groupView");
  a_ctToggleHeaderView       = new KToggleAction(i18n("Show &Header View"), CTRL+Key_H, this,
                               SLOT(slotToggleHeaderView()), actionCollection(), "settings_show_headerView");
  a_ctToggleArticleViewer    = new KToggleAction(i18n("Show &Article Viewer"), CTRL+Key_J, this,
                               SLOT(slotToggleArticleViewer()), actionCollection(), "settings_show_articleViewer");
  a_ctSwitchToGroupView      = new KAction(i18n("Switch to Group View"), Key_G , this,
                               SLOT(slotSwitchToGroupView()), actionCollection(), "switch_to_group_view");
  a_ctSwitchToGroupView->plugAccel(a_ccel);
  a_ctSwitchToHeaderView     = new KAction(i18n("Switch to Header View"), Key_H , this,
                               SLOT(slotSwitchToHeaderView()), actionCollection(), "switch_to_header_view");
  a_ctSwitchToHeaderView->plugAccel(a_ccel);
  a_ctSwitchToArticleViewer  = new KAction(i18n("Switch to Article Viewer"), Key_J , this,
                               SLOT(slotSwitchToArticleViewer()), actionCollection(), "switch_to_article_viewer");
  a_ctSwitchToArticleViewer->plugAccel(a_ccel);
}

void KNMainWidget::initPopups()
{
  a_ccPopup = static_cast<QPopupMenu *>(factory()->container("account_popup", m_GUIClient));
  if (!a_ccPopup) a_ccPopup = new QPopupMenu(this);

  g_roupPopup = static_cast<QPopupMenu *>(factory()->container("group_popup", m_GUIClient));
  if (!g_roupPopup) g_roupPopup = new QPopupMenu(this);

  r_ootFolderPopup = static_cast<QPopupMenu *>(factory()->container("root_folder_popup", m_GUIClient));
  if (!r_ootFolderPopup) r_ootFolderPopup = new QPopupMenu(this);

  f_olderPopup = static_cast<QPopupMenu *>(factory()->container("folder_popup", m_GUIClient));
  if (!f_olderPopup) f_olderPopup = new QPopupMenu(this);

  r_emotePopup = static_cast<QPopupMenu *>(factory()->container("remote_popup", m_GUIClient));
  if (!r_emotePopup) r_emotePopup = new QPopupMenu(this);

  l_ocalPopup = static_cast<QPopupMenu *>(factory()->container("local_popup", m_GUIClient));
  if (!l_ocalPopup) l_ocalPopup = new QPopupMenu(this);

  QPopupMenu *pop = static_cast<QPopupMenu *>(factory()->container("body_popup", m_GUIClient));
  if (!pop) pop = new QPopupMenu(this);
  a_rtView->setBodyPopup(pop);

  b_popupInitialized = true;
}


bool KNMainWidget::firstStart()
{
  KConfig *conf=KNGlobals::config();
  conf->setGroup("GENERAL");
  QString ver = conf->readEntry("Version");
  if(!ver.isEmpty())
    return false;

  KConfig emailConf("emaildefaults");

  emailConf.setGroup("Defaults");
  QString group = emailConf.readEntry("Profile","Default");

  emailConf.setGroup(QString("PROFILE_%1").arg(group));
  KNConfig::Identity *id=knGlobals.cfgManager->identity();
  id->setName(emailConf.readEntry("FullName"));
  id->setEmail(emailConf.readEntry("EmailAddress").latin1());
  id->setOrga(emailConf.readEntry("Organization"));
  id->setReplyTo(emailConf.readEntry("ReplyAddr"));
  id->save();

  KNServerInfo *smtp=knGlobals.accManager->smtp();
  smtp->setServer(emailConf.readEntry("OutgoingServer").latin1());
  smtp->setPort(25);
  conf->setGroup("MAILSERVER");
  smtp->saveConf(conf);

  conf->setGroup("GENERAL");
  conf->writeEntry("Version", KNODE_VERSION);

  return true;
}


void KNMainWidget::readOptions()
{
  KConfig *conf=KNGlobals::config();
  conf->setGroup("APPEARANCE");

  QValueList<int> lst = conf->readIntListEntry("Hdrs_Size3");
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
  bool sortByThreadChangeDate=conf->readBoolEntry("sortByThreadChangeDate", false);
  h_drView->setColAsc(sortCol, sortAsc);
  h_drView->setSorting(sortCol, sortAsc);
  h_drView->setSortByThreadChangeDate(sortByThreadChangeDate);
  if(sortByThreadChangeDate)
    h_drView->setColumnText(4, i18n("Date (thread changed)"));
  else
    h_drView->setColumnText(4, i18n("Date"));
  a_ctArtSortHeaders->setCurrentItem(sortCol);

  sortCol = conf->readNumEntry("account_sortCol", 0);
  sortAsc = conf->readBoolEntry("account_sortAscending", true);
  c_olView->setColAsc(sortCol, sortAsc);
  c_olView->setSorting(sortCol, sortAsc);

  resize(787,478);  // default optimized for 800x600
  //applyMainWindowSettings(KGlobal::config(),"mainWindow_options");

  // restore dock configuration
  manager()->readConfig(KNGlobals::config(),"dock_configuration");
}


void KNMainWidget::saveOptions()
{
  KConfig *conf=KNGlobals::config();
  conf->setGroup("APPEARANCE");

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
  conf->writeEntry("sortByThreadChangeDate", h_drView->sortByThreadChangeDate());
  conf->writeEntry("account_sortCol", c_olView->sortColumn());
  conf->writeEntry("account_sortAscending", c_olView->ascending());

  //saveMainWindowSettings(KGlobal::config(),"mainWindow_options");

  // store dock configuration
  manager()->writeConfig(KNGlobals::config(),"dock_configuration");
}


bool KNMainWidget::requestShutdown()
{
  kdDebug(5003) << "KNMainWidget::requestShutdown()" << endl;

  if( a_rtFactory->jobsPending() &&
      KMessageBox::No==KMessageBox::warningYesNo(this, i18n(
"KNode is currently sending articles. If you quit now you might lose these \
articles.\nDo you want to quit anyway?"))
    )
    return false;

  if(!a_rtFactory->closeComposeWindows())
    return false;

  return true;
}


void KNMainWidget::prepareShutdown()
{
  kdDebug(5003) << "KNMainWidget::prepareShutdown()" << endl;

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

  delete cup;

  saveOptions();
  RecentAddresses::self()->save( KNGlobals::config() );
  c_fgManager->syncConfig();
  a_rtManager->deleteTempFiles();
  g_rpManager->syncGroups();
  f_olManager->syncFolders();
  f_ilManager->prepareShutdown();
  a_ccManager->prepareShutdown();
  s_coreManager->save();
}


bool KNMainWidget::queryClose()
{
  if(b_lockui)
    return false;

  if(!requestShutdown())
    return false;

  prepareShutdown();

  return true;
}


void KNMainWidget::showEvent(QShowEvent *)
{
  slotCheckDockWidgetStatus();
}


void KNMainWidget::fontChange( const QFont & )
{
  a_rtFactory->configChanged();
  KNArticleWidget::configChanged();
  configChanged();
}


void KNMainWidget::paletteChange( const QPalette & )
{
  c_fgManager->appearance()->updateHexcodes();
  KNArticleWidget::configChanged();
  configChanged();
}


bool KNMainWidget::eventFilter(QObject *o, QEvent *e)
{
  if (((e->type() == QEvent::KeyPress) ||
       (e->type() == QEvent::KeyRelease) ||
       (e->type() == QEvent::Accel) ||
       (e->type() == QEvent::AccelOverride)) &&
       b_lockui)
    return true;
  return KDockArea::eventFilter(o, e);
}


void KNMainWidget::getSelectedArticles(KNArticle::List &l)
{
  if(!g_rpManager->currentGroup() && !f_olManager->currentFolder())
    return;

  for(QListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected() || (static_cast<KNHdrViewItem*>(i)->isActive()))
      l.append( static_cast<KNArticle*> ((static_cast<KNHdrViewItem*>(i))->art) );
}


void KNMainWidget::getSelectedArticles(KNRemoteArticle::List &l)
{
  if(!g_rpManager->currentGroup()) return;

  for(QListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected() || (static_cast<KNHdrViewItem*>(i)->isActive()))
      l.append( static_cast<KNRemoteArticle*> ((static_cast<KNHdrViewItem*>(i))->art) );
}


void KNMainWidget::getSelectedThreads(KNRemoteArticle::List &l)
{
  KNRemoteArticle *art;
  for(QListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected() || (static_cast<KNHdrViewItem*>(i)->isActive())) {
      art=static_cast<KNRemoteArticle*> ((static_cast<KNHdrViewItem*>(i))->art);
      // ignore the article if it is already in the list
      // (multiple aritcles are selected in one thread)
      if (l.findRef(art)==-1)
        art->thread(l);
    }
}


void KNMainWidget::getSelectedArticles(QPtrList<KNLocalArticle> &l)
{
  if(!f_olManager->currentFolder()) return;

  for(QListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected() || (static_cast<KNHdrViewItem*>(i)->isActive()))
      l.append( static_cast<KNLocalArticle*> ((static_cast<KNHdrViewItem*>(i))->art) );
}


void KNMainWidget::closeCurrentThread()
{
  QListViewItem *item = h_drView->currentItem();
  if (item) {
    while (item->parent())
      item = item->parent();
    h_drView->setCurrentItem(item);
    item->setOpen(false);
    h_drView->ensureItemVisible(item);
  }
}


void KNMainWidget::slotArticleSelected(QListViewItem *i)
{
  kdDebug(5003) << "KNMainWidget::slotArticleSelected(QListViewItem *i)" << endl;
  if(b_lockui)
    return;
  KNArticle *selectedArticle=0;

  if(i)
    selectedArticle=(static_cast<KNHdrViewItem*>(i))->art;

  a_rtView->setArticle(selectedArticle);

  //actions
  bool enabled;

  enabled=( selectedArticle && selectedArticle->type()==KMime::Base::ATremote );
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

  enabled=( selectedArticle && selectedArticle->type()==KMime::Base::ATlocal );
  a_ctArtDelete->setEnabled(enabled);
  a_ctArtSendNow->setEnabled(enabled && (f_olManager->currentFolder()==f_olManager->outbox()));
  a_ctArtEdit->setEnabled(enabled && ((f_olManager->currentFolder()==f_olManager->outbox())||
                                      (f_olManager->currentFolder()==f_olManager->drafts())));
}


void KNMainWidget::slotArticleSelectionChanged()
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


void KNMainWidget::slotCollectionSelected(QListViewItem *i)
{
  kdDebug(5003) << "KNMainWidget::slotCollectionSelected(QListViewItem *i)" << endl;
  if(b_lockui)
    return;
  KNCollection *c=0;
  KNNntpAccount *selectedAccount=0;
  KNGroup *selectedGroup=0;
  KNFolder *selectedFolder=0;

  h_drView->clear();
  slotArticleSelected(0);

  if(i) {
    c=(static_cast<KNCollectionViewItem*>(i))->coll;
    switch(c->type()) {
      case KNCollection::CTnntpAccount :
        selectedAccount=static_cast<KNNntpAccount*>(c);
        if(!i->isOpen())
          i->setOpen(true);
      break;
      case KNCollection::CTgroup :
        if (!(h_drView->hasFocus())&&!(a_rtView->hasFocus()))
          h_drView->setFocus();
        selectedGroup=static_cast<KNGroup*>(c);
        selectedAccount=selectedGroup->account();
      break;

      case KNCollection::CTfolder :
        if (!(h_drView->hasFocus())&&!(a_rtView->hasFocus()))
          h_drView->setFocus();
        selectedFolder=static_cast<KNFolder*>(c);
      break;

      default: break;
    }
  }

  a_ccManager->setCurrentAccount(selectedAccount);
  g_rpManager->setCurrentGroup(selectedGroup);
  f_olManager->setCurrentFolder(selectedFolder);
  if (!selectedGroup && !selectedFolder)         // called from showHeaders() otherwise
    a_rtManager->updateStatusString();

  updateCaption();

  //actions
  bool enabled;

  enabled=(selectedGroup) || (selectedFolder && !selectedFolder->isRootFolder());
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
    a_ctAccRename->setEnabled(enabled);
    a_ctAccSubscribe->setEnabled(enabled);
    a_ctAccExpireAll->setEnabled(enabled);
    a_ctAccGetNewHdrs->setEnabled(enabled);
    a_ctAccDelete->setEnabled(enabled);
    a_ctAccPostNewArticle->setEnabled(enabled);
  }

  enabled=( selectedGroup!=0 );
  if(a_ctGrpProperties->isEnabled() != enabled) {
    a_ctGrpProperties->setEnabled(enabled);
    a_ctGrpRename->setEnabled(enabled);
    a_ctGrpGetNewHdrs->setEnabled(enabled);
    a_ctGrpExpire->setEnabled(enabled);
    a_ctGrpReorganize->setEnabled(enabled);
    a_ctGrpUnsubscribe->setEnabled(enabled);
    a_ctGrpSetAllRead->setEnabled(enabled);
    a_ctGrpSetAllUnread->setEnabled(enabled);
    a_ctGrpSetUnread->setEnabled(enabled);
    a_ctArtFilter->setEnabled(enabled);
    a_ctArtFilterKeyb->setEnabled(enabled);
    a_ctArtRefreshList->setEnabled(enabled);
    a_ctArtCollapseAll->setEnabled(enabled);
    a_ctArtExpandAll->setEnabled(enabled);
    a_ctArtToggleShowThreads->setEnabled(enabled);
    a_ctReScore->setEnabled(enabled);
  }

  a_ctFolNewChild->setEnabled(selectedFolder!=0);

  enabled=( selectedFolder!=0 && !selectedFolder->isRootFolder() && !selectedFolder->isStandardFolder() );
  if(a_ctFolDelete->isEnabled() != enabled) {
    a_ctFolDelete->setEnabled(enabled);
    a_ctFolRename->setEnabled(enabled);
  }

  enabled=( selectedFolder!=0 &&  !selectedFolder->isRootFolder() );
  if(a_ctFolCompact->isEnabled() != enabled) {
    a_ctFolCompact->setEnabled(enabled);
    a_ctFolEmpty->setEnabled(enabled);
    a_ctFolMboxImport->setEnabled(enabled);
    a_ctFolMboxExport->setEnabled(enabled);
  }
}


void KNMainWidget::slotCollectionRenamed(QListViewItem *i)
{
  kdDebug(5003) << "KNMainWidget::slotCollectionRenamed(QListViewItem *i)" << endl;

  if (i) {
    (static_cast<KNCollectionViewItem*>(i))->coll->setName(i->text(0));
    updateCaption();
    a_rtManager->updateStatusString();
    if ((static_cast<KNCollectionViewItem*>(i))->coll->type()==KNCollection::CTnntpAccount)
      a_ccManager->accountRenamed(static_cast<KNNntpAccount*>((static_cast<KNCollectionViewItem*>(i))->coll));
    disableAccels(false);
  }
}


void KNMainWidget::slotCollectionViewDrop(QDropEvent* e, QListViewItem* after)
{
  kdDebug(5003) << "KNMainWidget::slotCollectionViewDrop() : type = " << e->format(0) << endl;

  KNCollectionViewItem *cvi=static_cast<KNCollectionViewItem*>(after);
  if (cvi && cvi->coll->type() != KNCollection::CTfolder)   // safety measure...
    return;
  KNFolder *dest=cvi ? static_cast<KNFolder*>(cvi->coll) : 0;

  if (e->provides("x-knode-drag/folder") && f_olManager->currentFolder()) {
    f_olManager->moveFolder(f_olManager->currentFolder(), dest);
  }
  else if(dest && e->provides("x-knode-drag/article")) {
    if(f_olManager->currentFolder()) {
      if (e->action() == QDropEvent::Move) {
        KNLocalArticle::List l;
        getSelectedArticles(l);
        a_rtManager->moveIntoFolder(l, dest);
      } else {
        KNArticle::List l;
        getSelectedArticles(l);
        a_rtManager->copyIntoFolder(l, dest);
      }
    }
    else if(g_rpManager->currentGroup()) {
      KNArticle::List l;
      getSelectedArticles(l);
      a_rtManager->copyIntoFolder(l, dest);
    }
  }
}


void KNMainWidget::slotArticleRMB(QListViewItem *i, const QPoint &p, int)
{
  if(b_lockui)
    return;

  if(!b_popupInitialized)
    initPopups();

  if(i) {
    if( (static_cast<KNHdrViewItem*>(i))->art->type()==KMime::Base::ATremote)
      r_emotePopup->popup(p);
    else
      l_ocalPopup->popup(p);
  }
}


void KNMainWidget::slotCollectionRMB(QListViewItem *i, const QPoint &p, int)
{
  if(b_lockui)
    return;

  if ( !b_popupInitialized )
    initPopups();

  if(i) {
    if( (static_cast<KNCollectionViewItem*>(i))->coll->type()==KNCollection::CTgroup)
      g_roupPopup->popup(p);
    else if ((static_cast<KNCollectionViewItem*>(i))->coll->type()==KNCollection::CTfolder) {
      if (static_cast<KNFolder*>(static_cast<KNCollectionViewItem*>(i)->coll)->isRootFolder())
        r_ootFolderPopup->popup(p);
      else
        f_olderPopup->popup(p);
    } else
      a_ccPopup->popup(p);
  }
}


void KNMainWidget::slotArticleMMB(QListViewItem *item)
{
  if(b_lockui)
    return;

  if (item) {
    KNArticle *art=(static_cast<KNHdrViewItem*>(item))->art;

    if ((art->type()==KMime::Base::ATlocal) && ((f_olManager->currentFolder()==f_olManager->outbox())||
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


void KNMainWidget::slotHdrViewSortingChanged(int i)
{
  a_ctArtSortHeaders->setCurrentItem(i);
}


void KNMainWidget::slotNetworkActive(bool b)
{
  a_ctNetCancel->setEnabled(b);
}


void KNMainWidget::slotCheckDockWidgetStatus()
{
  a_ctToggleGroupView->setChecked(c_olDock->isVisible());
  a_ctToggleHeaderView->setChecked(h_drDock->isVisible());
  a_ctToggleArticleViewer->setChecked(a_rtDock->isVisible());
}


void KNMainWidget::slotGroupDockHidden()
{
  a_ctToggleGroupView->setChecked(false);
}


void KNMainWidget::slotHeaderDockHidden()
{
  a_ctToggleHeaderView->setChecked(false);
}


void KNMainWidget::slotArticleDockHidden()
{
  a_ctToggleArticleViewer->setChecked(false);
}


void KNMainWidget::slotDockWidgetFocusChangeRequest(QWidget *w)
{
  if (w == a_rtView) {
    if (c_olView->isVisible()) {
      c_olView->setFocus();
      if (!w->hasFocus())  // fails if the view is visible but floating
        return;
    }
    if (h_drView->isVisible()) {
      h_drView->setFocus();
      return;
    }
  }
  if (w == c_olView) {
    if (h_drView->isVisible()) {
      h_drView->setFocus();
      if (!w->hasFocus())  // fails if the view is visible but floating
        return;
    }
    if (a_rtView->isVisible()) {
      a_rtView->setFocus();
      return;
    }
  }
  if (w == h_drView) {
    if (a_rtView->isVisible()) {
      a_rtView->setFocus();
      if (!w->hasFocus())  // fails if the view is visible but floating
        return;
    }
    if (c_olView->isVisible()) {
      c_olView->setFocus();
      return;
    }
  }
}


//------------------------------ <Actions> --------------------------------


void KNMainWidget::slotNavNextArt()
{
  kdDebug(5003) << "KNMainWidget::slotNavNextArt()" << endl;
  KNHdrViewItem *it= static_cast<KNHdrViewItem*>(h_drView->currentItem());

  if (it) {
    if (it->isActive()) {  // take current article, if not selected
      if (it->isExpandable())
        it->setOpen(true);
      it=static_cast<KNHdrViewItem*>(it->itemBelow());
    }
  } else
    it=static_cast<KNHdrViewItem*>(h_drView->firstChild());

  if(it)
    h_drView->setActive(it, true);
}


void KNMainWidget::slotNavPrevArt()
{
  kdDebug(5003) << "KNMainWidget::slotNavPrevArt()" << endl;
  KNHdrViewItem *it= static_cast<KNHdrViewItem*>(h_drView->currentItem());

  if (it && it->isActive()) {  // take current article, if not selected
    if (it) it=static_cast<KNHdrViewItem*>(it->itemAbove());
    else it=static_cast<KNHdrViewItem*>(h_drView->firstChild());
  }

  if(it)
    h_drView->setActive(it, true);
}


void KNMainWidget::slotNavNextUnreadArt()
{
  kdDebug(5003) << "KNMainWidget::slotNavNextUnreadArt()" << endl;

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
    if(current->isExpandable() && art->hasUnreadFollowUps() && !current->isOpen())
        h_drView->setOpen(current, true);
    next=static_cast<KNHdrViewItem*>(current->itemBelow());
  }

  while(next) {
    art=static_cast<KNRemoteArticle*>(next->art);
    if(!art->isRead()) break;
    else {
      if(next->isExpandable() && art->hasUnreadFollowUps() && !next->isOpen())
        h_drView->setOpen(next, true);
      next=static_cast<KNHdrViewItem*>(next->itemBelow());
    }
  }

  if(next)
    h_drView->setActive(next, true);
  else
    slotNavNextGroup();
}


void KNMainWidget::slotNavNextUnreadThread()
{
  kdDebug(5003) << "KNMainWidget::slotNavNextUnreadThread()" << endl;

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
    if (art->isRead())
      slotNavNextUnreadArt();
    else
      h_drView->setActive(next, true);
  }
  else
    slotNavNextGroup();
}


void KNMainWidget::slotNavNextGroup()
{
  kdDebug(5003) << "KNMainWidget::slotNavNextGroup()" << endl;
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
      secureProcessEvents();
      next=static_cast<KNCollectionViewItem*>(next->firstChild());
    }
    else next=static_cast<KNCollectionViewItem*>(next->itemBelow());
  }

  if (next)
    c_olView->setActive(next, true);
}


void KNMainWidget::slotNavPrevGroup()
{
  kdDebug(5003) << "KNMainWidget::slotNavPrevGroup()" << endl;
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

  if (prev)
    c_olView->setActive(prev, true);
}


void KNMainWidget::slotNavReadThrough()
{
  kdDebug(5003) << "KNMainWidget::slotNavReadThrough()" << endl;
  if (a_rtView->scrollingDownPossible())
    a_rtView->scrollDown();
  else if(g_rpManager->currentGroup() != 0)
    slotNavNextUnreadArt();
}


void KNMainWidget::slotAccProperties()
{
  kdDebug(5003) << "KNMainWidget::slotAccProperties()" << endl;
  if(a_ccManager->currentAccount())
    a_ccManager->editProperties(a_ccManager->currentAccount());
  updateCaption();
  a_rtManager->updateStatusString();
}


void KNMainWidget::slotAccRename()
{
  kdDebug(5003) << "KNMainWidget::slotAccRename()" << endl;
  if(a_ccManager->currentAccount()) {
    disableAccels(true);   // hack: global accels break the inplace renaming
    c_olView->rename(a_ccManager->currentAccount()->listItem(), 0);
  }
}


void KNMainWidget::slotAccSubscribe()
{
  kdDebug(5003) << "KNMainWidget::slotAccSubscribe()" << endl;
  if(a_ccManager->currentAccount())
    g_rpManager->showGroupDialog(a_ccManager->currentAccount());
}


void KNMainWidget::slotAccExpireAll()
{
  kdDebug(5003) << "KNMainWidget::slotAccExpireAll()" << endl;
  if(a_ccManager->currentAccount())
    g_rpManager->expireAll(a_ccManager->currentAccount());
}


void KNMainWidget::slotAccGetNewHdrs()
{
  kdDebug(5003) << "KNMainWidget::slotAccGetNewHdrs()" << endl;
  if(a_ccManager->currentAccount())
    g_rpManager->checkAll(a_ccManager->currentAccount());
}


void KNMainWidget::slotAccDelete()
{
  kdDebug(5003) << "KNMainWidget::slotAccDelete()" << endl;
  if(a_ccManager->currentAccount()) {
    if (a_ccManager->removeAccount(a_ccManager->currentAccount()))
      slotCollectionSelected(0);
  }
}


void KNMainWidget::slotAccPostNewArticle()
{
  kdDebug(5003) << "KNMainWidget::slotAccPostNewArticle()" << endl;
  if(g_rpManager->currentGroup())
    a_rtFactory->createPosting(g_rpManager->currentGroup());
  else if(a_ccManager->currentAccount())
    a_rtFactory->createPosting(a_ccManager->currentAccount());
}


void KNMainWidget::slotGrpProperties()
{
  kdDebug(5003) << "slotGrpProperties()" << endl;
  if(g_rpManager->currentGroup())
    g_rpManager->showGroupProperties(g_rpManager->currentGroup());
  updateCaption();
  a_rtManager->updateStatusString();
}


void KNMainWidget::slotGrpRename()
{
  kdDebug(5003) << "slotGrpRename()" << endl;
  if(g_rpManager->currentGroup()) {
    disableAccels(true);   // hack: global accels break the inplace renaming
    c_olView->rename(g_rpManager->currentGroup()->listItem(), 0);
  }
}


void KNMainWidget::slotGrpGetNewHdrs()
{
  kdDebug(5003) << "KNMainWidget::slotGrpGetNewHdrs()" << endl;
  if(g_rpManager->currentGroup())
    g_rpManager->checkGroupForNewHeaders(g_rpManager->currentGroup());
}


void KNMainWidget::slotGrpExpire()
{
  kdDebug(5003) << "KNMainWidget::slotGrpExpire()" << endl;
  if(g_rpManager->currentGroup())
    g_rpManager->expireGroupNow(g_rpManager->currentGroup());
}


void KNMainWidget::slotGrpReorganize()
{
  kdDebug(5003) << "KNMainWidget::slotGrpReorganize()" << endl;
  g_rpManager->reorganizeGroup(g_rpManager->currentGroup());
}


void KNMainWidget::slotGrpUnsubscribe()
{
  kdDebug(5003) << "KNMainWidget::slotGrpUnsubscribe()" << endl;
  if(g_rpManager->currentGroup()) {
    if(KMessageBox::Yes==KMessageBox::questionYesNo(knGlobals.topWidget,
      i18n("Do you really want to unsubscribe from %1?").arg(g_rpManager->currentGroup()->groupname())))
      if (g_rpManager->unsubscribeGroup(g_rpManager->currentGroup()))
        slotCollectionSelected(0);
  }
}


void KNMainWidget::slotGrpSetAllRead()
{
  kdDebug(5003) << "KNMainWidget::slotGrpSetAllRead()" << endl;

  a_rtManager->setAllRead(true);
  if (c_fgManager->readNewsNavigation()->markAllReadGoNext())
    slotNavNextGroup();
}


void KNMainWidget::slotGrpSetAllUnread()
{
  kdDebug(5003) << "KNMainWidget::slotGrpSetAllUnread()" << endl;
  a_rtManager->setAllRead(false);
}

void KNMainWidget::slotGrpSetUnread()
{
  kdDebug(5003) << "KNMainWidget::slotGrpSetUnread()" << endl;
  int groupLength=g_rpManager->currentGroup()->length();

  bool ok = FALSE;
  int res = QInputDialog::getInteger(
                i18n( "Mark Last as Unread" ),
                i18n( "Enter how many articles should be marked unread:" ), groupLength, 1, groupLength, 1, &ok, this );
  if ( ok )
    a_rtManager->setAllRead(res, false);
}

void KNMainWidget::slotFolNew()
{
  kdDebug(5003) << "KNMainWidget::slotFolNew()" << endl;
  KNFolder *f = f_olManager->newFolder(0);

  if (f) {
    f_olManager->setCurrentFolder(f);
    c_olView->ensureItemVisible(f->listItem());
    c_olView->setActive(f->listItem(), true);
    slotFolRename();
  }
}


void KNMainWidget::slotFolNewChild()
{
  kdDebug(5003) << "KNMainWidget::slotFolNew()" << endl;
  if(f_olManager->currentFolder()) {
    KNFolder *f = f_olManager->newFolder(f_olManager->currentFolder());

    if (f) {
      f_olManager->setCurrentFolder(f);
      c_olView->ensureItemVisible(f->listItem());
      c_olView->setActive(f->listItem(), true);
      slotFolRename();
    }
  }
}


void KNMainWidget::slotFolDelete()
{
  kdDebug(5003) << "KNMainWidget::slotFolDelete()" << endl;

  if(!f_olManager->currentFolder() || f_olManager->currentFolder()->isRootFolder())
    return;

  if(f_olManager->currentFolder()->isStandardFolder())
    KMessageBox::sorry(knGlobals.topWidget, i18n("You cannot delete a standard folder."));

  else if( KMessageBox::Yes==KMessageBox::questionYesNo(knGlobals.topWidget,
      i18n("Do you really want to delete this folder and all its children?")) ) {

    if(!f_olManager->deleteFolder(f_olManager->currentFolder()))
      KMessageBox::sorry(knGlobals.topWidget,
      i18n("This folder cannot be deleted because some of\n its articles are currently in use.") );
    else
      slotCollectionSelected(0);
  }
}


void KNMainWidget::slotFolRename()
{
  kdDebug(5003) << "KNMainWidget::slotFolRename()" << endl;

  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
    if(f_olManager->currentFolder()->isStandardFolder())
      KMessageBox::sorry(knGlobals.topWidget, i18n("You cannot rename a standard folder."));
    else {
      disableAccels(true);   // hack: global accels break the inplace renaming
      c_olView->rename(f_olManager->currentFolder()->listItem(), 0);
    }
  }
}


void KNMainWidget::slotFolCompact()
{
  kdDebug(5003) << "KNMainWidget::slotFolCompact()" << endl;
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder())
    f_olManager->compactFolder(f_olManager->currentFolder());
}


void KNMainWidget::slotFolCompactAll()
{
  kdDebug(5003) << "KNMainWidget::slotFolCompactAll()" << endl;
  f_olManager->compactAll();
}


void KNMainWidget::slotFolEmpty()
{
  kdDebug(5003) << "KNMainWidget::slotFolEmpty()" << endl;
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
    if(f_olManager->currentFolder()->lockedArticles()>0) {
      KMessageBox::sorry(this,
      i18n("This folder cannot be emptied at the moment\nbecause some of its articles are currently in use.") );
      return;
    }
    if( KMessageBox::Yes == KMessageBox::questionYesNo(
        this, i18n("Do you really want to delete all articles in %1?").arg(f_olManager->currentFolder()->name())) )
      f_olManager->emptyFolder(f_olManager->currentFolder());
  }
}


void KNMainWidget::slotFolMBoxImport()
{
  kdDebug(5003) << "KNMainWidget::slotFolMBoxImport()" << endl;
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
     f_olManager->importFromMBox(f_olManager->currentFolder());
  }
}


void KNMainWidget::slotFolMBoxExport()
{
  kdDebug(5003) << "KNMainWidget::slotFolMBoxExport()" << endl;
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
    f_olManager->exportToMBox(f_olManager->currentFolder());
  }
}


void KNMainWidget::slotArtSortHeaders(int i)
{
  kdDebug(5003) << "KNMainWidget::slotArtSortHeaders(int i)" << endl;
  h_drView->slotSortList(i);
}


void KNMainWidget::slotArtSortHeadersKeyb()
{
  kdDebug(5003) << "KNMainWidget::slotArtSortHeadersKeyb()" << endl;

  int newCol = KNHelper::selectDialog(this, i18n("Select Sort Column"), a_ctArtSortHeaders->items(), a_ctArtSortHeaders->currentItem());
  if (newCol != -1)
    h_drView->slotSortList(newCol);
}


void KNMainWidget::slotArtSearch()
{
  kdDebug(5003) << "KNMainWidget::slotArtSearch()" << endl;
  a_rtManager->search();
}


void KNMainWidget::slotArtFind()
{
  kdDebug(5003) << "KNMainWidget::slotArtFind()" << endl;
  a_rtView->find();
}


void KNMainWidget::slotArtRefreshList()
{
  kdDebug(5003) << "KNMainWidget::slotArtRefreshList()" << endl;
  a_rtManager->showHdrs(true);
}


void KNMainWidget::slotArtCollapseAll()
{
  kdDebug(5003) << "KNMainWidget::slotArtCollapseAll()" << endl;

  closeCurrentThread();
  a_rtManager->setAllThreadsOpen(false);
  if (h_drView->currentItem())
    h_drView->ensureItemVisible(h_drView->currentItem());
}


void KNMainWidget::slotArtExpandAll()
{
  kdDebug(5003) << "KNMainWidget::slotArtExpandAll()" << endl;

  a_rtManager->setAllThreadsOpen(true);
  if (h_drView->currentItem())
    h_drView->ensureItemVisible(h_drView->currentItem());
}


void KNMainWidget::slotArtToggleThread()
{
  kdDebug(5003) << "KNMainWidget::slotArtToggleThread()" << endl;
  if(a_rtView->article() && a_rtView->article()->listItem()->isExpandable()) {
    bool o=!(a_rtView->article()->listItem()->isOpen());
    a_rtView->article()->listItem()->setOpen(o);
  }
}


void KNMainWidget::slotArtToggleShowThreads()
{
  kdDebug(5003) << "KNMainWidget::slotArtToggleShowThreads()" << endl;
  if(g_rpManager->currentGroup()) {
    c_fgManager->readNewsGeneral()->setShowThreads(!c_fgManager->readNewsGeneral()->showThreads());
    a_rtManager->showHdrs(true);
  }
}


void KNMainWidget::slotArtSetArtRead()
{
  kdDebug(5003) << "KNMainWidget::slotArtSetArtRead()" << endl;
  if(!g_rpManager->currentGroup())
    return;

  KNRemoteArticle::List l;
  getSelectedArticles(l);
  a_rtManager->setRead(l, true);
}


void KNMainWidget::slotArtSetArtUnread()
{
  kdDebug(5003) << "KNMainWidget::slotArtSetArtUnread()" << endl;
  if(!g_rpManager->currentGroup())
    return;

  KNRemoteArticle::List l;
  getSelectedArticles(l);
  a_rtManager->setRead(l, false);
}


void KNMainWidget::slotArtSetThreadRead()
{
  kdDebug(5003) << "slotArtSetThreadRead()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->setRead(l, true);

  if (h_drView->currentItem()) {
    if (c_fgManager->readNewsNavigation()->markThreadReadCloseThread())
      closeCurrentThread();
    if (c_fgManager->readNewsNavigation()->markThreadReadGoNext())
      slotNavNextUnreadThread();
  }
}


void KNMainWidget::slotArtSetThreadUnread()
{
  kdDebug(5003) << "KNMainWidget::slotArtSetThreadUnread()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->setRead(l, false);
}


void KNMainWidget::slotScoreEdit()
{
  kdDebug(5003) << "KNMainWidget::slotScoreEdit()" << endl;
  s_coreManager->configure();
}


void KNMainWidget::slotReScore()
{
  kdDebug(5003) << "KNMainWidget::slotReScore()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  g_rpManager->currentGroup()->scoreArticles(false);
  a_rtManager->showHdrs(true);
}


void KNMainWidget::slotScoreLower()
{
  kdDebug(5003) << "KNMainWidget::slotScoreLower() start" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  if (a_rtView->article() && a_rtView->article()->type()==KMime::Base::ATremote) {
    KNRemoteArticle *ra = static_cast<KNRemoteArticle*>(a_rtView->article());
    s_coreManager->addRule(KNScorableArticle(ra), g_rpManager->currentGroup()->groupname(), -10);
  }
}


void KNMainWidget::slotScoreRaise()
{
  kdDebug(5003) << "KNMainWidget::slotScoreRaise() start" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  if (a_rtView->article() && a_rtView->article()->type()==KMime::Base::ATremote) {
    KNRemoteArticle *ra = static_cast<KNRemoteArticle*>(a_rtView->article());
    s_coreManager->addRule(KNScorableArticle(ra), g_rpManager->currentGroup()->groupname(), +10);
  }
}


void KNMainWidget::slotArtToggleIgnored()
{
  kdDebug(5003) << "KNMainWidget::slotArtToggleIgnored()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  bool revert = !a_rtManager->toggleIgnored(l);
  a_rtManager->rescoreArticles(l);

  if (h_drView->currentItem() && !revert) {
    if (c_fgManager->readNewsNavigation()->ignoreThreadCloseThread())
      closeCurrentThread();
    if (c_fgManager->readNewsNavigation()->ignoreThreadGoNext())
      slotNavNextUnreadThread();
  }
}


void KNMainWidget::slotArtToggleWatched()
{
  kdDebug(5003) << "KNMainWidget::slotArtToggleWatched()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->toggleWatched(l);
  a_rtManager->rescoreArticles(l);
}


void KNMainWidget::slotArtOpenNewWindow()
{
  kdDebug(5003) << "KNMainWidget::slotArtOpenNewWindow()" << endl;

  if(a_rtView->article()) {
    if (!KNArticleWindow::raiseWindowForArticle(a_rtView->article())) {
      KNArticleWindow *win=new KNArticleWindow(a_rtView->article());
      win->show();
    }
  }
}


void KNMainWidget::slotArtSendOutbox()
{
  kdDebug(5003) << "KNMainWidget::slotArtSendOutbox()" << endl;
  a_rtFactory->sendOutbox();
}


void KNMainWidget::slotArtDelete()
{
  kdDebug(5003) << "KNMainWidget::slotArtDelete()" << endl;
  if (!f_olManager->currentFolder())
    return;

  KNLocalArticle::List lst;
  getSelectedArticles(lst);

  if(!lst.isEmpty())
    a_rtManager->deleteArticles(lst);

  if(h_drView->currentItem())
    h_drView->setActive(h_drView->currentItem(),true);
}


void KNMainWidget::slotArtSendNow()
{
  kdDebug(5003) << "KNMainWidget::slotArtSendNow()" << endl;
  if (!f_olManager->currentFolder())
    return;

  KNLocalArticle::List lst;
  getSelectedArticles(lst);

  if(!lst.isEmpty())
    a_rtFactory->sendArticles(&lst, true);
}


void KNMainWidget::slotArtEdit()
{
  kdDebug(5003) << "KNodeVew::slotArtEdit()" << endl;
  if (!f_olManager->currentFolder())
    return;

  if (a_rtView->article() && a_rtView->article()->type()==KMime::Base::ATlocal)
    a_rtFactory->edit(static_cast<KNLocalArticle*>(a_rtView->article()));
}


void KNMainWidget::slotNetCancel()
{
  kdDebug(5003) << "KNMainWidget::slotNetCancel()" << endl;
  n_etAccess->cancelAllJobs();
}


void KNMainWidget::slotFetchArticleWithID()
{
  kdDebug(5003) << "KNMainWidget::slotFetchArticleWithID()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  FetchArticleIdDlg *dlg = new FetchArticleIdDlg(this, "messageid" );

  if (dlg->exec()) {
    QString id = dlg->messageId().simplifyWhiteSpace();
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

void KNMainWidget::slotToggleGroupView()
{
  c_olDock->changeHideShowState();
  slotCheckDockWidgetStatus();
}


void KNMainWidget::slotToggleHeaderView()
{
  h_drDock->changeHideShowState();
  slotCheckDockWidgetStatus();
}


void KNMainWidget::slotToggleArticleViewer()
{
  a_rtDock->changeHideShowState();
  slotCheckDockWidgetStatus();
}


void KNMainWidget::slotSwitchToGroupView()
{
  if (!c_olView->isVisible())
    slotToggleGroupView();
  c_olView->setFocus();
}


void KNMainWidget::slotSwitchToHeaderView()
{
  if (!h_drView->isVisible())
    slotToggleHeaderView();
  h_drView->setFocus();
}

void KNMainWidget::slotSwitchToArticleViewer()
{
  if (!a_rtView->isVisible())
    slotToggleArticleViewer();
  a_rtView->setFocus();
}


void KNMainWidget::slotConfKeys()
{
  //KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
  KKeyDialog::configure( actionCollection(),
			 true /*allow one-letter shortcuts*/
    );
}

void KNMainWidget::slotSettings()
{
  c_fgManager->configure();
}

KActionCollection* KNMainWidget::actionCollection() const
{
  return m_GUIClient->actionCollection();
}

KXMLGUIFactory* KNMainWidget::factory() const
{
  kdDebug()<<"m_guiclient is "<< m_GUIClient
           <<", the factory is " << m_GUIClient->factory() <<endl;
  return m_GUIClient->factory();
}

//--------------------------------


FetchArticleIdDlg::FetchArticleIdDlg(QWidget *parent, const char */*name*/ )
    :KDialogBase(parent, 0, true, i18n("Fetch Article with ID"), KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok)
{
  QHBox *page = makeHBoxMainWidget();

  QLabel *label = new QLabel(i18n("&Message-ID:"),page);
  edit = new KLineEdit(page);
  label->setBuddy(edit);
  edit->setFocus();
  enableButtonOK( false );
  connect( edit, SIGNAL(textChanged( const QString & )), this, SLOT(slotTextChanged(const QString & )));
  KNHelper::restoreWindowSize("fetchArticleWithID", this, QSize(325,66));
}

QString FetchArticleIdDlg::messageId() const
{
    return edit->text();
}

void FetchArticleIdDlg::slotTextChanged(const QString &_text )
{
    enableButtonOK( !_text.isEmpty() );
}

void KNMainWidget::slotReparented()
{
  if ( c_olView->parent() != c_olDock )
    c_olDock->undock();
}

////////////////////////////////////////////////////////////////////////
//////////////////////// DCOP implementation
// Move to the next article
void KNMainWidget::nextArticle()
{
  slotNavNextArt();
}

// Move to the previous article
void KNMainWidget::previousArticle()
{
  slotNavPrevArt();
}

// Move to the next unread article
void KNMainWidget::nextUnreadArticle()
{
  slotNavNextUnreadArt();
}

// Move to the next unread thread
void KNMainWidget::nextUnreadThread()
{
  slotNavNextUnreadThread();
}

// Move to the next group
void KNMainWidget::nextGroup()
{
  slotNavNextGroup();
}

// Move to the previous group
void KNMainWidget::previousGroup()
{
  slotNavPrevGroup();
}

void KNMainWidget::fetchHeaders()
{
  // Simply call the slot
  slotAccGetNewHdrs();
}

// Open the editor to post a new article in the selected group
void KNMainWidget::postArticle()
{
  slotAccPostNewArticle();
}

// Fetch the new headers in the selected groups
void KNMainWidget::fetchHeadersInCurrentGroup()
{
  slotGrpGetNewHdrs();
}

// Expire the articles in the current group
void KNMainWidget::expireArticlesInCurrentGroup()
{
  slotGrpExpire();
}

// Mark all the articles in the current group as read
void KNMainWidget::markAllAsRead()
{
  slotGrpSetAllRead();
}

// Mark all the articles in the current group as unread
void KNMainWidget::markAllAsUnread()
{
  slotGrpSetAllUnread();
}

// Mark the current article as read
void KNMainWidget::markAsRead()
{
  slotArtSetArtRead();
}

// Mark the current article as unread
void KNMainWidget::markAsUnread()
{
  slotArtSetArtUnread();
}

// Mark the current thread as read
void KNMainWidget::markThreadAsRead()
{
  slotArtSetThreadRead();
}

// Mark the current thread as unread
void KNMainWidget::markThreadAsUnread()
{
  slotArtSetThreadUnread();
}

// Send the pending articles
void KNMainWidget::sendPendingMessages()
{
  slotArtSendOutbox();
}

// Delete the current article
void KNMainWidget::deleteArticle()
{
  slotArtDelete();
}

// Send the current article
void KNMainWidget::sendNow()
{
  slotArtSendNow();
}

// Edit the current article
void KNMainWidget::editArticle()
{
  slotArtEdit();
}

//////////////////////// end DCOP implementation
////////////////////////////////////////////////////////////////////////

#include "knmainwidget.moc"
