/*
    KNode, the KDE newsreader
    Copyright (c) 2003 Zack Rusin <zack@kde.org>
    Copyright (c) 2004-2006 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/
#include "knmainwidget.h"

#include <QLayout>
//Added by qt3to4:
#include <Q3Accel>
#include <QEvent>
#include <QDropEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QMenu>
#include <QSplitter>
#include <QtDBus>
#include <ktoolbar.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <kinputdialog.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kedittoolbar.h>
#include <kstandardaction.h>
#include <kdebug.h>
#include <kmenubar.h>
#include <kiconloader.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <k3listviewsearchline.h>
#include <khbox.h>
#include <kselectaction.h>
#include <kstandardshortcut.h>
#include <ktoggleaction.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

#include "broadcaststatus.h"
#include "krsqueezedtextlabel.h"
#include "recentaddresses.h"
using KPIM::BroadcastStatus;
using KRecentAddress::RecentAddresses;

//GUI
#include "knmainwidget.h"
#include "knarticlewindow.h"
#include "kncollectionview.h"
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"

//Core
#include "articlewidget.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "knarticlemanager.h"
#include "knarticlefactory.h"
#include "kngroupmanager.h"
#include "knnntpaccount.h"
#include "knaccountmanager.h"
#include "knfiltermanager.h"
#include "knfoldermanager.h"
#include "knfolder.h"
#include "kncleanup.h"
#include "utilities.h"
#include "knscoring.h"
#include <kpgp.h>
#include "knmemorymanager.h"
#include "scheduler.h"
#include "settings.h"

#include "knodeadaptor.h"

using namespace KNode;

KNMainWidget::KNMainWidget( KXMLGUIClient* client, QWidget* parent ) :
  KVBox( parent ),
  b_lockui( false ),
  m_GUIClient( client )
{
  (void) new KnodeAdaptor( this );
  QDBusConnection::sessionBus().registerObject("/KNode", this);       
  knGlobals.top=this;
  knGlobals.guiClient=client;
  knGlobals.topWidget=this;

  //------------------------------- <CONFIG> ----------------------------------
  c_fgManager = knGlobals.configManager();
  //------------------------------- </CONFIG> ----------------------------------

  //-------------------------------- <GUI> ------------------------------------
  // this will enable keyboard-only actions like that don't appear in any menu
  actionCollection()->setDefaultShortcutContext( Qt::WindowShortcut );
  actionCollection()->setAssociatedWidget( this );

  Q3Accel *accel = new Q3Accel( this );
  initStatusBar();
  setSpacing( 0 );
  setMargin( 0 );
  setLineWidth( 0 );

  // splitters
  mPrimarySplitter = new QSplitter( Qt::Horizontal, this );
  mSecondSplitter = new QSplitter( Qt::Vertical, mPrimarySplitter );

  //article view
  mArticleViewer = new ArticleWidget( mPrimarySplitter, knGlobals.guiClient, actionCollection() );
  knGlobals.artWidget = mArticleViewer;

  //collection view
  c_olView = new KNCollectionView( mSecondSplitter );

  connect(c_olView, SIGNAL(selectionChanged(Q3ListViewItem*)),
          SLOT(slotCollectionSelected(Q3ListViewItem*)));
  connect(c_olView, SIGNAL(contextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)),
          SLOT(slotCollectionRMB(K3ListView*, Q3ListViewItem*, const QPoint&)));
  connect(c_olView, SIGNAL(folderDrop(QDropEvent*, KNCollectionViewItem*)),
          SLOT(slotCollectionViewDrop(QDropEvent*, KNCollectionViewItem*)));
  connect(c_olView, SIGNAL(itemRenamed(Q3ListViewItem*)),
          SLOT(slotCollectionRenamed(Q3ListViewItem*)));

  accel->connectItem( accel->insertItem(Qt::Key_Up), mArticleViewer, SLOT(scrollUp()) );
  accel->connectItem( accel->insertItem(Qt::Key_Down), mArticleViewer, SLOT(scrollDown()) );
  accel->connectItem( accel->insertItem(Qt::Key_PageUp), mArticleViewer, SLOT(scrollPrior()) );
  accel->connectItem( accel->insertItem(Qt::Key_PageDown), mArticleViewer, SLOT(scrollNext()) );

  //header view
  QWidget *dummy = new QWidget( mSecondSplitter );
  QVBoxLayout *vlay = new QVBoxLayout(dummy);
  vlay->setSpacing( 0 );
  vlay->setMargin( 0 );
  h_drView = new KNHeaderView( dummy );

  q_uicksearch = new KToolBar( dummy );
  QLabel *lbl = new QLabel(i18n("&Search:"),dummy);
  lbl->setObjectName("kde toolbar widget");
  q_uicksearch->addWidget( lbl );
  s_earchLineEdit = new K3ListViewSearchLine( q_uicksearch, h_drView );
  q_uicksearch->addWidget( s_earchLineEdit );
  lbl->setBuddy(s_earchLineEdit);

  vlay->addWidget(q_uicksearch);
  vlay->addWidget(h_drView);

  connect(h_drView, SIGNAL(itemSelected(Q3ListViewItem*)),
          SLOT(slotArticleSelected(Q3ListViewItem*)));
  connect(h_drView, SIGNAL(selectionChanged()),
          SLOT(slotArticleSelectionChanged()));
  connect(h_drView, SIGNAL(contextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)),
          SLOT(slotArticleRMB(K3ListView*, Q3ListViewItem*, const QPoint&)));
  connect(h_drView, SIGNAL(doubleClick(Q3ListViewItem *)),
          SLOT(slotOpenArticle(Q3ListViewItem *)));
  connect(h_drView, SIGNAL(sortingChanged(int)),
          SLOT(slotHdrViewSortingChanged(int)));

  //actions
  initActions();

  // splitter setup
  mPrimarySplitter->addWidget( c_olView );
  mPrimarySplitter->addWidget( mSecondSplitter );
  mSecondSplitter->addWidget( dummy );
  mSecondSplitter->addWidget( mArticleViewer );

  //-------------------------------- </GUI> ------------------------------------

  //-------------------------------- <CORE> ------------------------------------

  //Network
  connect( knGlobals.scheduler(), SIGNAL(netActive(bool)), this, SLOT(slotNetworkActive(bool)) );

  //Filter Manager
  f_ilManager = knGlobals.filterManager();
  f_ilManager->setMenuAction(a_ctArtFilter, a_ctArtFilterKeyb);

  //Article Manager
  a_rtManager = knGlobals.articleManager();
  a_rtManager->setView(h_drView);

  //Group Manager
  g_rpManager = knGlobals.groupManager();

  //Folder Manager
  f_olManager = knGlobals.folderManager();

  //Account Manager
  a_ccManager = knGlobals.accountManager();

  //Article Factory
  a_rtFactory=new KNArticleFactory();
  knGlobals.artFactory=a_rtFactory;

  // Score Manager
  s_coreManager = knGlobals.scoringManager();
  //connect(s_coreManager, SIGNAL(changedRules()), SLOT(slotReScore()));
  connect(s_coreManager, SIGNAL(finishedEditing()), SLOT(slotReScore()));

  // Memory Manager
  m_emManager = knGlobals.memoryManager();

  // create a global pgp instance
  p_gp = new Kpgp::Module();
  knGlobals.pgp = p_gp;

  QDBusConnection::sessionBus().registerObject( "/", this, QDBusConnection::ExportScriptableSlots );
  //-------------------------------- </CORE> -----------------------------------

  //apply saved options
  readOptions();

  //apply configuration
  configChanged();

  // set the keyboard focus indicator on the first item in the Collection View
  if( c_olView->firstChild() ) {
    Q3ListViewItem *i = c_olView->firstChild();
    bool open = i->isOpen();
    c_olView->setActive( i );
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
  //delete a_ccel;

  h_drView->clear(); //avoid some random crashes in KNHdrViewItem::~KNHdrViewItem()

  delete knGlobals.scheduler();
  kDebug(5003) << "KNMainWidget::~KNMainWidget() : Scheduler deleted" << endl;

  delete a_rtManager;
  kDebug(5003) << "KNMainWidget::~KNMainWidget() : Article Manager deleted" << endl;

  delete a_rtFactory;
  kDebug(5003) << "KNMainWidget::~KNMainWidget() : Article Factory deleted" << endl;

  delete g_rpManager;
  kDebug(5003) << "KNMainWidget::~KNMainWidget() : Group Manager deleted" << endl;

  delete f_olManager;
  kDebug(5003) << "KNMainWidget::~KNMainWidget() : Folder Manager deleted" << endl;

  delete f_ilManager;
  kDebug(5003) << "KNMainWidget::~KNMainWidget() : Filter Manager deleted" << endl;

  delete a_ccManager;
  kDebug(5003) << "KNMainWidget::~KNMainWidget() : Account Manager deleted" << endl;

  delete c_fgManager;
  kDebug(5003) << "KNMainWidget::~KNMainWidget() : Config deleted" << endl;

  delete m_emManager;
  kDebug(5003) << "KNMainWidget::~KNMainWidget() : Memory Manager deleted" << endl;

  delete p_gp;
  kDebug(5003) << "KNMainWidget::~KNMainWidget() : PGP deleted" << endl;
}

void KNMainWidget::initStatusBar()
{
  //statusbar
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KStatusBar *sb =  mainWin ? mainWin->statusBar() : 0;
  s_tatusFilter = new KRSqueezedTextLabel( QString(), sb );
  s_tatusFilter->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  s_tatusGroup = new KRSqueezedTextLabel( QString(), sb );
  s_tatusGroup->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
}

//================================== GUI =================================

void KNMainWidget::setStatusMsg(const QString& text, int id)
{
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KStatusBar *bar =  mainWin ? mainWin->statusBar() : 0;
  if ( !bar )
    return;
  bar->clearMessage();
  if (text.isEmpty() && (id==SB_MAIN)) {
    BroadcastStatus::instance()->setStatusMsg(i18n(" Ready"));
  } else {
    switch(id) {
      case SB_MAIN:
        BroadcastStatus::instance()->setStatusMsg(text); break;
      case SB_GROUP:
        s_tatusGroup->setText(text); break;
      case SB_FILTER:
        s_tatusFilter->setText(text); break;
    }
  }
}


void KNMainWidget::setStatusHelpMsg(const QString& text)
{
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KStatusBar *bar =  mainWin ? mainWin->statusBar() : 0;
  if ( bar )
    bar->showMessage(text, 2000);
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
  if(b) KApplication::setOverrideCursor(Qt::WaitCursor);
  else  KApplication::restoreOverrideCursor();
}


void KNMainWidget::blockUI(bool b)
{
  b_lockui = b;
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KMenuBar *mbar =  mainWin ? mainWin->menuBar() : 0;
  if ( mbar )
    mbar->setEnabled(!b);
#warning Port me!
  /*a_ccel->setEnabled(!b);
  KAccel *naccel = mainWin ? mainWin->accel() : 0;
  if ( naccel )
  naccel->setEnabled(!b);*/
  if (b)
    installEventFilter(this);
  else
    removeEventFilter(this);
  setCursorBusy(b);
}


void KNMainWidget::disableAccels(bool b)
{
#warning Port me!
  /*a_ccel->setEnabled(!b);
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KAccel *naccel = mainWin ? mainWin->accel() : 0;
  if ( naccel )
    naccel->setEnabled(!b);*/
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
#warning Port me!
  /*a_ccel->setEnabled(false);
  KAccel *naccel = mainWin ? mainWin->accel() : 0;
  if ( naccel )
    naccel->setEnabled(false);*/
  installEventFilter(this);

  kapp->processEvents();

  b_lockui = false;
  if ( mbar )
    mbar->setEnabled(true);
#warning Port me!
//  a_ccel->setEnabled(true);
//   if ( naccel )
//     naccel->setEnabled(true);
  removeEventFilter(this);
}


QSize KNMainWidget::sizeHint() const
{
  return QSize(759,478);    // default optimized for 800x600
}


void KNMainWidget::openURL(const QString &url)
{
  openURL(KUrl(url));
}

void KNMainWidget::openURL(const KUrl &url)
{
  kDebug(5003) << k_funcinfo << url << endl;
  QString host = url.host();
  unsigned short int port = url.port();
  KNNntpAccount *acc=0;

  if (url.url().left(7) == "news://") {

    // lets see if we already have an account for this host...
    KNAccountManager::List list = a_ccManager->accounts();
    for ( KNAccountManager::List::Iterator it = list.begin(); it != list.end(); ++it ) {
      if ( (*it)->server().toLower() == host.toLower() && ( port==0 || (*it)->port() == port ) ) {
        acc = *it;
        break;
      }
    }

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
      acc = a_ccManager->currentAccount();
      if ( acc == 0 )
        acc = a_ccManager->first();
    } else {
      kDebug(5003) << "KNMainWidget::openURL() URL is not a valid news URL" << endl;
    }
  }

  if (acc) {
    QString decodedUrl = KUrl::fromPercentEncoding( url.url().toLatin1() );
    bool isMID=decodedUrl.contains('@' );

    if (!isMID) {
      QString groupname=url.path( KUrl::RemoveTrailingSlash );
      while(groupname.startsWith("/"))
        groupname.remove(0,1);
      Q3ListViewItem *item=0;
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
        c_olView->setActive( item );
      }
    } else {
      QString groupname = decodedUrl.mid( url.protocol().length()+1 );
      KNGroup *g = g_rpManager->currentGroup();
      if (g == 0)
        g = g_rpManager->firstGroupOfAccount(acc);

      if (g) {
        if ( !ArticleWindow::raiseWindowForArticle( groupname.toLatin1() ) ) { //article not yet opened
          KNRemoteArticle *a=new KNRemoteArticle(g);
          QString messageID = '<' + groupname + '>';
          a->messageID()->from7BitString(messageID.toLatin1());
          ArticleWindow *awin = new ArticleWindow( a );
          awin->show();
        }
      } else {
        // TODO: fetch without group
        kDebug(5003) << "KNMainWidget::openURL() account has no groups" << endl;
      }
    }
  }
}


// update fonts and colors
void KNMainWidget::configChanged()
{
  h_drView->readConfig();
  c_olView->readConfig();
  a_rtManager->updateListViewItems();
}


void KNMainWidget::initActions()
{
  //a_ccel=new KAccel(this);

  //navigation
  a_ctNavNextArt            = new KAction( KIcon("next"), i18n("&Next Article"), actionCollection(), "go_nextArticle" );
  a_ctNavNextArt->setToolTip(i18n("Go to next article"));
  a_ctNavNextArt->setShortcut(KShortcut("N;Right"));
  connect(a_ctNavNextArt, SIGNAL(triggered(bool)), h_drView, SLOT(nextArticle()));
  a_ctNavPrevArt            = new KAction( KIcon("previous"), i18n("&Previous Article"), actionCollection(), "go_prevArticle" );
  a_ctNavPrevArt->setShortcut(KShortcut("P;Left"));
  a_ctNavPrevArt->setToolTip(i18n("Go to previous article"));
  connect(a_ctNavPrevArt, SIGNAL(triggered(bool)), h_drView, SLOT(prevArticle()));
  a_ctNavNextUnreadArt = new KAction(KIcon("1rightarrow"), i18n("Next Unread &Article"), actionCollection(), "go_nextUnreadArticle");
  connect(a_ctNavNextUnreadArt, SIGNAL(triggered(bool)), SLOT(slotNavNextUnreadArt()));
  a_ctNavNextUnreadArt->setShortcut(QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_Space));
  a_ctNavNextUnreadThread = new KAction(KIcon("2rightarrow"), i18n("Next Unread &Thread"), actionCollection(), "go_nextUnreadThread");
  connect(a_ctNavNextUnreadThread, SIGNAL(triggered(bool)), SLOT(slotNavNextUnreadThread()));
  a_ctNavNextUnreadThread->setShortcut(QKeySequence(Qt::SHIFT+Qt::Key_Space));
  a_ctNavNextGroup = new KAction(KIcon("down"), i18n("Ne&xt Group"), actionCollection(), "go_nextGroup");
  connect(a_ctNavNextGroup, SIGNAL(triggered(bool)), c_olView, SLOT(nextGroup()));
  a_ctNavNextGroup->setShortcut(QKeySequence(Qt::Key_Plus));
  a_ctNavPrevGroup = new KAction(KIcon("up"), i18n("Pre&vious Group"), actionCollection(), "go_prevGroup");
  connect(a_ctNavPrevGroup, SIGNAL(triggered(bool)), c_olView, SLOT(prevGroup()));
  a_ctNavPrevGroup->setShortcut(QKeySequence(Qt::Key_Minus));
  a_ctNavReadThrough = new KAction(i18n("Read &Through Articles"), actionCollection(), "go_readThrough");
  connect(a_ctNavReadThrough, SIGNAL(triggered(bool) ), SLOT(slotNavReadThrough()));
  a_ctNavReadThrough->setShortcut(QKeySequence(Qt::Key_Space));

  KAction *action = new KAction( i18n("Focus on Next Folder"), actionCollection(), "inc_current_folder" );
  connect(action, SIGNAL(triggered(bool) ), c_olView, SLOT(incCurrentFolder()));
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Right));
  action = new KAction( i18n("Focus on Previous Folder"), actionCollection(), "dec_current_folder" );
  connect(action, SIGNAL(triggered(bool) ), c_olView, SLOT(decCurrentFolder()));
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Left));
  action = new KAction( i18n("Select Folder with Focus"), actionCollection(), "select_current_folder" );
  connect(action, SIGNAL(triggered(bool) ), c_olView, SLOT(selectCurrentFolder()));
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Space));
  action = new KAction( i18n("Focus on Next Article"), actionCollection(), "inc_current_article" );
  connect(action, SIGNAL(triggered(bool) ), h_drView, SLOT(incCurrentArticle()));
  action->setShortcut(QKeySequence(Qt::ALT+Qt::Key_Right));
  action = new KAction( i18n("Focus on Previous Article"), actionCollection(), "dec_current_article" );
  connect(action, SIGNAL(triggered(bool) ), h_drView, SLOT(decCurrentArticle()));
  action->setShortcut(QKeySequence(Qt::ALT+Qt::Key_Left));
  action = new KAction( i18n("Select Article with Focus"), actionCollection(), "select_current_article" );
  connect(action, SIGNAL(triggered(bool) ), h_drView, SLOT(selectCurrentArticle()));
  action->setShortcut(QKeySequence(Qt::ALT+Qt::Key_Space));

  //collection-view - accounts
  a_ctAccProperties = new KAction(KIcon("configure"), i18n("Account &Properties"), actionCollection(), "account_properties");
  connect(a_ctAccProperties, SIGNAL(triggered(bool)), SLOT(slotAccProperties()));
  a_ctAccRename = new KAction(KIcon("text"), i18n("&Rename Account"), actionCollection(), "account_rename");
  connect(a_ctAccRename, SIGNAL(triggered(bool)), SLOT(slotAccRename()));
  a_ctAccSubscribe = new KAction(KIcon("news_subscribe"), i18n("&Subscribe to Newsgroups..."), actionCollection(), "account_subscribe");
  connect(a_ctAccSubscribe, SIGNAL(triggered(bool)), SLOT(slotAccSubscribe()));
  a_ctAccExpireAll = new KAction(i18n("&Expire All Groups"), actionCollection(), "account_expire_all");
  connect(a_ctAccExpireAll, SIGNAL(triggered(bool) ), SLOT(slotAccExpireAll()));
  a_ctAccGetNewHdrs = new KAction(KIcon("mail_get"), i18n("&Get New Articles in All Groups"), actionCollection(), "account_dnlHeaders");
  connect(a_ctAccGetNewHdrs, SIGNAL(triggered(bool)), SLOT(slotAccGetNewHdrs()));
  a_ctAccGetNewHdrsAll = new KAction(KIcon("mail_get_all"), i18n("&Get New Articles in All Accounts"), actionCollection(), "account_dnlAllHeaders");
  connect(a_ctAccGetNewHdrsAll, SIGNAL(triggered(bool)), SLOT(slotAccGetNewHdrsAll()));
  a_ctAccDelete = new KAction(KIcon("editdelete"), i18n("&Delete Account"), actionCollection(), "account_delete");
  connect(a_ctAccDelete, SIGNAL(triggered(bool)), SLOT(slotAccDelete()));
  a_ctAccPostNewArticle = new KAction(KIcon("mail_new"), i18n("&Post to Newsgroup..."), actionCollection(), "article_postNew");
  connect(a_ctAccPostNewArticle, SIGNAL(triggered(bool)), SLOT(slotAccPostNewArticle()));
  a_ctAccPostNewArticle->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_N));

  //collection-view - groups
  a_ctGrpProperties = new KAction(KIcon("configure"), i18n("Group &Properties"), actionCollection(), "group_properties");
  connect(a_ctGrpProperties, SIGNAL(triggered(bool)), SLOT(slotGrpProperties()));
  a_ctGrpRename = new KAction(KIcon("text"), i18n("Rename &Group"), actionCollection(), "group_rename");
  connect(a_ctGrpRename, SIGNAL(triggered(bool)), SLOT(slotGrpRename()));
  a_ctGrpGetNewHdrs = new KAction(KIcon("mail_get"), i18n("&Get New Articles"), actionCollection(), "group_dnlHeaders");
  connect(a_ctGrpGetNewHdrs, SIGNAL(triggered(bool)), SLOT(slotGrpGetNewHdrs()));
  a_ctGrpExpire = new KAction(KIcon("wizard"), i18n("E&xpire Group"), actionCollection(), "group_expire");
  connect(a_ctGrpExpire, SIGNAL(triggered(bool)), SLOT(slotGrpExpire()));
  a_ctGrpReorganize = new KAction(i18n("Re&organize Group"), actionCollection(), "group_reorg");
  connect(a_ctGrpReorganize, SIGNAL(triggered(bool) ), SLOT(slotGrpReorganize()));
  a_ctGrpUnsubscribe = new KAction(KIcon("news_unsubscribe"), i18n("&Unsubscribe From Group"), actionCollection(), "group_unsubscribe");
  connect(a_ctGrpUnsubscribe, SIGNAL(triggered(bool)), SLOT(slotGrpUnsubscribe()));
  a_ctGrpSetAllRead = new KAction(KIcon("goto"), i18n("Mark All as &Read"), actionCollection(), "group_allRead");
  connect(a_ctGrpSetAllRead, SIGNAL(triggered(bool)), SLOT(slotGrpSetAllRead()));
  a_ctGrpSetAllUnread = new KAction(i18n("Mark All as U&nread"), actionCollection(), "group_allUnread");
  connect(a_ctGrpSetAllUnread, SIGNAL(triggered(bool) ), SLOT(slotGrpSetAllUnread()));
  a_ctGrpSetUnread = new KAction(i18n("Mark Last as Unr&ead..."), actionCollection(), "group_unread");
  connect(a_ctGrpSetUnread, SIGNAL(triggered(bool) ), SLOT(slotGrpSetUnread()));

  action = new KAction(KIcon("configure"),  i18n("&Configure KNode..."), actionCollection(), "knode_configure_knode" );
  connect(action, SIGNAL(triggered(bool) ), SLOT(slotSettings()));

  //collection-view - folder
  a_ctFolNew = new KAction(KIcon("folder_new"), i18n("&New Folder"), actionCollection(), "folder_new");
  connect(a_ctFolNew, SIGNAL(triggered(bool)), SLOT(slotFolNew()));
  a_ctFolNewChild = new KAction(KIcon("folder_new"), i18n("New &Subfolder"), actionCollection(), "folder_newChild");
  connect(a_ctFolNewChild, SIGNAL(triggered(bool)), SLOT(slotFolNewChild()));
  a_ctFolDelete = new KAction(KIcon("editdelete"), i18n("&Delete Folder"), actionCollection(), "folder_delete");
  connect(a_ctFolDelete, SIGNAL(triggered(bool)), SLOT(slotFolDelete()));
  a_ctFolRename = new KAction(KIcon("text"), i18n("&Rename Folder"), actionCollection(), "folder_rename");
  connect(a_ctFolRename, SIGNAL(triggered(bool)), SLOT(slotFolRename()));
  a_ctFolCompact = new KAction(KIcon("wizard"), i18n("C&ompact Folder"), actionCollection(), "folder_compact");
  connect(a_ctFolCompact, SIGNAL(triggered(bool)), SLOT(slotFolCompact()));
  a_ctFolCompactAll = new KAction(i18n("Co&mpact All Folders"), actionCollection(), "folder_compact_all");
  connect(a_ctFolCompactAll, SIGNAL(triggered(bool) ), SLOT(slotFolCompactAll()));
  a_ctFolEmpty = new KAction(i18n("&Empty Folder"), actionCollection(), "folder_empty");
  connect(a_ctFolEmpty, SIGNAL(triggered(bool) ), SLOT(slotFolEmpty()));
  a_ctFolMboxImport = new KAction(i18n("&Import MBox Folder..."), actionCollection(), "folder_MboxImport");
  connect(a_ctFolMboxImport, SIGNAL(triggered(bool) ), SLOT(slotFolMBoxImport()));
  a_ctFolMboxExport = new KAction(i18n("E&xport as MBox Folder..."), actionCollection(), "folder_MboxExport");
  connect(a_ctFolMboxExport, SIGNAL(triggered(bool) ), SLOT(slotFolMBoxExport()));

  //header-view - list-handling
  a_ctArtSortHeaders        = new KSelectAction(i18n("S&ort"), KShortcut(), actionCollection(), "view_Sort");
  QStringList items;
  items += i18n("By &Subject");
  items += i18n("By S&ender");
  items += i18n("By S&core");
  items += i18n("By &Lines");
  items += i18n("By &Date");
  a_ctArtSortHeaders->setItems(items);
  a_ctArtSortHeaders->setShortcutConfigurable(false);
  connect(a_ctArtSortHeaders, SIGNAL(activated(int)), this, SLOT(slotArtSortHeaders(int)));
  a_ctArtSortHeadersKeyb = new KAction(i18n("Sort"), actionCollection(), "view_Sort_Keyb");
  connect(a_ctArtSortHeadersKeyb, SIGNAL(triggered(bool)), SLOT(slotArtSortHeadersKeyb()));
  a_ctArtSortHeadersKeyb->setShortcut(QKeySequence(Qt::Key_F7));
  a_ctArtFilter             = new KNFilterSelectAction(i18n("&Filter"), "filter",
                              actionCollection(), "view_Filter");
  a_ctArtFilter->setShortcutConfigurable(false);
  a_ctArtFilterKeyb = new KAction(i18n("Filter"), actionCollection(), "view_Filter_Keyb");
  a_ctArtFilterKeyb->setShortcut(QKeySequence(Qt::Key_F6));
  a_ctArtSearch = new KAction(KIcon("mail_find"), i18n("&Search Articles..."), actionCollection(), "article_search");
  connect(a_ctArtSearch, SIGNAL(triggered(bool)), SLOT(slotArtSearch()));
  a_ctArtSearch->setShortcut(QKeySequence(Qt::Key_F4));
  a_ctArtRefreshList = new KAction(KIcon("reload"), i18n("&Refresh List"), actionCollection(), "view_Refresh");
  connect(a_ctArtRefreshList, SIGNAL(triggered(bool)), SLOT(slotArtRefreshList()));
  a_ctArtRefreshList->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Reload));
  a_ctArtCollapseAll = new KAction(i18n("&Collapse All Threads"), actionCollection(), "view_CollapseAll");
  connect(a_ctArtCollapseAll, SIGNAL(triggered(bool) ), SLOT(slotArtCollapseAll()));
  a_ctArtExpandAll = new KAction(i18n("E&xpand All Threads"), actionCollection(), "view_ExpandAll");
  connect(a_ctArtExpandAll, SIGNAL(triggered(bool) ), SLOT(slotArtExpandAll()));
  a_ctArtToggleThread = new KAction(i18n("&Toggle Subthread"), actionCollection(), "thread_toggle");
  connect(a_ctArtToggleThread, SIGNAL(triggered(bool) ), SLOT(slotArtToggleThread()));
  a_ctArtToggleThread->setShortcut(QKeySequence(Qt::Key_T));
  a_ctArtToggleShowThreads = new KToggleAction(i18n("Show T&hreads"), actionCollection(), "view_showThreads");
  connect(a_ctArtToggleShowThreads, SIGNAL(triggered(bool) ), SLOT(slotArtToggleShowThreads()));
  a_ctArtToggleShowThreads->setCheckedState(KGuiItem(i18n("Hide T&hreads")));

  a_ctArtToggleShowThreads->setChecked( knGlobals.settings()->showThreads() );

  //header-view - remote articles
  a_ctArtSetArtRead = new KAction(i18n("Mark as &Read"), actionCollection(), "article_read");
  connect(a_ctArtSetArtRead, SIGNAL(triggered(bool) ), SLOT(slotArtSetArtRead()));
  a_ctArtSetArtRead->setShortcut(QKeySequence(Qt::Key_D));
  a_ctArtSetArtUnread = new KAction(i18n("Mar&k as Unread"), actionCollection(), "article_unread");
  connect(a_ctArtSetArtUnread, SIGNAL(triggered(bool) ), SLOT(slotArtSetArtUnread()));
  a_ctArtSetArtUnread->setShortcut(QKeySequence(Qt::Key_U));
  a_ctArtSetThreadRead = new KAction(i18n("Mark &Thread as Read"), actionCollection(), "thread_read");
  connect(a_ctArtSetThreadRead, SIGNAL(triggered(bool) ), SLOT(slotArtSetThreadRead()));
  a_ctArtSetThreadRead->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_D));
  a_ctArtSetThreadUnread = new KAction(i18n("Mark T&hread as Unread"), actionCollection(), "thread_unread");
  connect(a_ctArtSetThreadUnread, SIGNAL(triggered(bool) ), SLOT(slotArtSetThreadUnread()));
  a_ctArtSetThreadUnread->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_U));
  a_ctArtOpenNewWindow = new KAction(KIcon("window_new"), i18n("Open in Own &Window"), actionCollection(), "article_ownWindow");
  connect(a_ctArtOpenNewWindow, SIGNAL(triggered(bool)), SLOT(slotArtOpenNewWindow()));
  a_ctArtOpenNewWindow->setShortcut(QKeySequence(Qt::Key_O));

  // scoring
  a_ctScoresEdit = new KAction(KIcon("edit"), i18n("&Edit Scoring Rules..."), actionCollection(), "scoreedit");
  connect(a_ctScoresEdit, SIGNAL(triggered(bool)), SLOT(slotScoreEdit()));
  a_ctScoresEdit->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_E));
  a_ctReScore = new KAction(i18n("Recalculate &Scores"), actionCollection(), "rescore");
  connect(a_ctReScore, SIGNAL(triggered(bool) ), SLOT(slotReScore()));
  a_ctScoreLower = new KAction(i18n("&Lower Score for Author..."), actionCollection(), "scorelower");
  connect(a_ctScoreLower, SIGNAL(triggered(bool) ), SLOT(slotScoreLower()));
  a_ctScoreLower->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_L));
  a_ctScoreRaise = new KAction(i18n("&Raise Score for Author..."), actionCollection(), "scoreraise");
  connect(a_ctScoreRaise, SIGNAL(triggered(bool) ), SLOT(slotScoreRaise()));
  a_ctScoreRaise->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_I));
  a_ctArtToggleIgnored = new KAction(KIcon("bottom"), i18n("&Ignore Thread"), actionCollection(), "thread_ignore");
  connect(a_ctArtToggleIgnored, SIGNAL(triggered(bool)), SLOT(slotArtToggleIgnored()));
  a_ctArtToggleIgnored->setShortcut(QKeySequence(Qt::Key_I));
  a_ctArtToggleWatched = new KAction(KIcon("top"), i18n("&Watch Thread"), actionCollection(), "thread_watch");
  connect(a_ctArtToggleWatched, SIGNAL(triggered(bool)), SLOT(slotArtToggleWatched()));
  a_ctArtToggleWatched->setShortcut(QKeySequence(Qt::Key_W));

  //header-view local articles
  a_ctArtSendOutbox = new KAction(KIcon("mail_send"), i18n("Sen&d Pending Messages"), actionCollection(), "net_sendPending");
  connect(a_ctArtSendOutbox, SIGNAL(triggered(bool)), SLOT(slotArtSendOutbox()));
  a_ctArtDelete = new KAction(KIcon("editdelete"), i18n("&Delete Article"), actionCollection(), "article_delete");
  connect(a_ctArtDelete, SIGNAL(triggered(bool)), SLOT(slotArtDelete()));
  a_ctArtDelete->setShortcut(QKeySequence(Qt::Key_Delete));
  a_ctArtSendNow = new KAction(KIcon("mail_send"), i18n("Send &Now"), actionCollection(), "article_sendNow");
  connect(a_ctArtSendNow, SIGNAL(triggered(bool)), SLOT(slotArtSendNow()));
  a_ctArtEdit = new KAction(KIcon("edit"), i18nc("edit article","&Edit Article..."), actionCollection(), "article_edit");
  connect(a_ctArtEdit, SIGNAL(triggered(bool)), SLOT(slotArtEdit()));
  a_ctArtEdit->setShortcut(QKeySequence(Qt::Key_E));

  //network
  a_ctNetCancel = new KAction(KIcon("stop"), i18n("Stop &Network"), actionCollection(), "net_stop");
  connect(a_ctNetCancel, SIGNAL(triggered(bool)), SLOT(slotNetCancel()));
  a_ctNetCancel->setEnabled(false);

  a_ctFetchArticleWithID = new KAction(i18n("&Fetch Article with ID..."), actionCollection(), "fetch_article_with_id");
  connect(a_ctFetchArticleWithID, SIGNAL(triggered(bool) ), SLOT(slotFetchArticleWithID()));
  a_ctFetchArticleWithID->setEnabled(false);

  a_ctToggleQuickSearch = new KToggleAction(i18n("Show Quick Search"), actionCollection(), "settings_show_quickSearch");
  connect(a_ctToggleQuickSearch, SIGNAL(triggered(bool) ), SLOT(slotToggleQuickSearch()));
  a_ctToggleQuickSearch->setCheckedState(KGuiItem(i18n("Hide Quick Search")));
}

bool KNMainWidget::firstStart()
{
  KConfig *conf=knGlobals.config();
  conf->setGroup("GENERAL");
  QString ver = conf->readEntry("Version");
  if(!ver.isEmpty())
    return false;

  KConfig emailConf("emaildefaults");

  emailConf.setGroup("Defaults");
  QString group = emailConf.readEntry("Profile","Default");

  emailConf.setGroup(QString("PROFILE_%1").arg(group));
  KNode::Identity *id=knGlobals.configManager()->identity();
  id->setName(emailConf.readEntry("FullName"));
  id->setEmail(emailConf.readEntry("EmailAddress").toLatin1());
  id->setOrga(emailConf.readEntry("Organization"));
  id->setReplyTo(emailConf.readEntry("ReplyAddr"));
  id->save();

  KNServerInfo *smtp=knGlobals.accountManager()->smtp();
  smtp->setServer(emailConf.readEntry("OutgoingServer").toLatin1());
  smtp->setPort(25);
  conf->setGroup("MAILSERVER");
  smtp->saveConf(conf);

  conf->setGroup("GENERAL");
  conf->writeEntry("Version", KNODE_VERSION);

  return true;
}


void KNMainWidget::readOptions()
{
  KConfig *conf=knGlobals.config();
  conf->setGroup("APPEARANCE");

  if (conf->readEntry("quicksearch", true))
    a_ctToggleQuickSearch->setChecked(true);
  else
    q_uicksearch->hide();
  c_olView->readConfig();
  h_drView->readConfig();
  a_ctArtSortHeaders->setCurrentItem( h_drView->sortColumn() );

  resize(787,478);  // default optimized for 800x600
  //applyMainWindowSettings(KGlobal::config(),"mainWindow_options");
}


void KNMainWidget::saveOptions()
{
  KConfig *conf=knGlobals.config();
  conf->setGroup("APPEARANCE");

  conf->writeEntry("quicksearch", !q_uicksearch->isHidden());
  //saveMainWindowSettings(KGlobal::config(),"mainWindow_options");

  c_olView->writeConfig();
  h_drView->writeConfig();
  mArticleViewer->writeConfig();
}


bool KNMainWidget::requestShutdown()
{
  kDebug(5003) << "KNMainWidget::requestShutdown()" << endl;

  if( a_rtFactory->jobsPending() &&
      KMessageBox::No==KMessageBox::warningYesNo(this, i18n(
"KNode is currently sending articles. If you quit now you might lose these \
    articles.\nDo you want to quit anyway?"), QString(), KStandardGuiItem::quit(), KStandardGuiItem::cancel())
    )
    return false;

  if(!a_rtFactory->closeComposeWindows())
    return false;

  return true;
}


void KNMainWidget::prepareShutdown()
{
  kDebug(5003) << "KNMainWidget::prepareShutdown()" << endl;

  //cleanup article-views
  ArticleWidget::cleanup();

  // expire groups (if necessary)
  KNCleanUp *cup = new KNCleanUp();
  g_rpManager->expireAll(cup);
  cup->start();

  // compact folders
  KNode::Cleanup *conf=c_fgManager->cleanup();
  if (conf->compactToday()) {
    cup->reset();
    f_olManager->compactAll(cup);
    cup->start();
    conf->setLastCompactDate();
  }

  delete cup;

  saveOptions();
  RecentAddresses::self(knGlobals.config())->save( knGlobals.config() );
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


void KNMainWidget::fontChange( const QFont & )
{
  a_rtFactory->configChanged();
  ArticleWidget::configChanged();
  configChanged();
}


void KNMainWidget::paletteChange( const QPalette & )
{
  ArticleWidget::configChanged();
  configChanged();
}


bool KNMainWidget::eventFilter(QObject *o, QEvent *e)
{
  if (((e->type() == QEvent::KeyPress) ||
       (e->type() == QEvent::KeyRelease) ||
       (e->type() == QEvent::Shortcut) ||
       (e->type() == QEvent::ShortcutOverride)) &&
       b_lockui)
    return true;
  return QWidget::eventFilter(o, e);
}


void KNMainWidget::getSelectedArticles(KNArticle::List &l)
{
  if(!g_rpManager->currentGroup() && !f_olManager->currentFolder())
    return;

  for(Q3ListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected() || (static_cast<KNHdrViewItem*>(i)->isActive()))
      l.append( static_cast<KNArticle*> ((static_cast<KNHdrViewItem*>(i))->art) );
}


void KNMainWidget::getSelectedArticles(KNRemoteArticle::List &l)
{
  if(!g_rpManager->currentGroup()) return;

  for(Q3ListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected() || (static_cast<KNHdrViewItem*>(i)->isActive()))
      l.append( static_cast<KNRemoteArticle*> ((static_cast<KNHdrViewItem*>(i))->art) );
}


void KNMainWidget::getSelectedThreads(KNRemoteArticle::List &l)
{
  KNRemoteArticle *art;
  for(Q3ListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected() || (static_cast<KNHdrViewItem*>(i)->isActive())) {
      art=static_cast<KNRemoteArticle*> ((static_cast<KNHdrViewItem*>(i))->art);
      // ignore the article if it is already in the list
      // (multiple aritcles are selected in one thread)
      if ( !l.contains(art)  )
        art->thread(l);
    }
}


void KNMainWidget::getSelectedArticles( KNLocalArticle::List &l )
{
  if(!f_olManager->currentFolder()) return;

  for(Q3ListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected() || (static_cast<KNHdrViewItem*>(i)->isActive()))
      l.append( static_cast<KNLocalArticle*> ((static_cast<KNHdrViewItem*>(i))->art) );
}


void KNMainWidget::closeCurrentThread()
{
  Q3ListViewItem *item = h_drView->currentItem();
  if (item) {
    while (item->parent())
      item = item->parent();
    h_drView->setCurrentItem(item);
    item->setOpen(false);
    h_drView->ensureItemVisible(item);
  }
}

void KNMainWidget::slotArticleSelected(Q3ListViewItem *i)
{
  kDebug(5003) << "KNMainWidget::slotArticleSelected(QListViewItem *i)" << endl;
  if(b_lockui)
    return;
  KNArticle *selectedArticle=0;

  if(i)
    selectedArticle=(static_cast<KNHdrViewItem*>(i))->art;

  mArticleViewer->setArticle( selectedArticle );

  //actions
  bool enabled;

  enabled=( selectedArticle && selectedArticle->type()==KNArticle::ATremote );
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

  enabled=( selectedArticle && selectedArticle->type()==KNArticle::ATlocal );
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


void KNMainWidget::slotCollectionSelected(Q3ListViewItem *i)
{
  kDebug(5003) << "KNMainWidget::slotCollectionSelected(QListViewItem *i)" << endl;
  if(b_lockui)
    return;
  KNCollection *c=0;
  KNNntpAccount *selectedAccount=0;
  KNGroup *selectedGroup=0;
  KNFolder *selectedFolder=0;

  s_earchLineEdit->clear();
  h_drView->clear();
  slotArticleSelected(0);

  // mark all articles in current group as not new/read
  if ( knGlobals.settings()->leaveGroupMarkAsRead() )
    a_rtManager->setAllRead( true );
  a_rtManager->setAllNotNew();

  if(i) {
    c=(static_cast<KNCollectionViewItem*>(i))->coll;
    switch(c->type()) {
      case KNCollection::CTnntpAccount :
        selectedAccount=static_cast<KNNntpAccount*>(c);
        if(!i->isOpen())
          i->setOpen(true);
      break;
      case KNCollection::CTgroup :
        if ( !h_drView->hasFocus() && !mArticleViewer->hasFocus() )
          h_drView->setFocus();
        selectedGroup=static_cast<KNGroup*>(c);
        selectedAccount=selectedGroup->account();
      break;

      case KNCollection::CTfolder :
        if ( !h_drView->hasFocus() && !mArticleViewer->hasFocus() )
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
    a_ctAccGetNewHdrsAll->setEnabled(enabled);
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


void KNMainWidget::slotCollectionRenamed(Q3ListViewItem *i)
{
  kDebug(5003) << "KNMainWidget::slotCollectionRenamed(QListViewItem *i)" << endl;

  if (i) {
    (static_cast<KNCollectionViewItem*>(i))->coll->setName(i->text(0));
    updateCaption();
    a_rtManager->updateStatusString();
    if ((static_cast<KNCollectionViewItem*>(i))->coll->type()==KNCollection::CTnntpAccount)
      a_ccManager->accountRenamed(static_cast<KNNntpAccount*>((static_cast<KNCollectionViewItem*>(i))->coll));
    disableAccels(false);
  }
}


void KNMainWidget::slotCollectionViewDrop(QDropEvent* e, KNCollectionViewItem* after)
{
  kDebug(5003) << "KNMainWidget::slotCollectionViewDrop() : type = " << e->format(0) << endl;

  KNCollectionViewItem *cvi=static_cast<KNCollectionViewItem*>(after);
  if (cvi && cvi->coll->type() != KNCollection::CTfolder)   // safety measure...
    return;
  KNFolder *dest=cvi ? static_cast<KNFolder*>(cvi->coll) : 0;

  if (e->provides("x-knode-drag/folder") && f_olManager->currentFolder()) {
    f_olManager->moveFolder(f_olManager->currentFolder(), dest);
  }
  else if(dest && e->provides("x-knode-drag/article")) {
    if(f_olManager->currentFolder()) {
      if (e->dropAction() == Qt::MoveAction) {
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


void KNMainWidget::slotArticleRMB(K3ListView*, Q3ListViewItem *i, const QPoint &p)
{
  if(b_lockui)
    return;

  if(i) {
    QMenu *popup;
    if( (static_cast<KNHdrViewItem*>(i))->art->type()==KNArticle::ATremote) {
     popup = static_cast<QMenu *>(factory()->container("remote_popup", m_GUIClient));
    } else {
     popup = static_cast<QMenu *>(factory()->container("local_popup", m_GUIClient));
    }

    if ( popup )
      popup->popup(p);
  }
}


void KNMainWidget::slotCollectionRMB(K3ListView*, Q3ListViewItem *i, const QPoint &p)
{
  if(b_lockui)
    return;

  if(i) {
    if( (static_cast<KNCollectionViewItem*>(i))->coll->type()==KNCollection::CTgroup) {
      QMenu *popup = static_cast<QMenu *>(factory()->container("group_popup", m_GUIClient));
      if ( popup )
        popup->popup(p);
    } else if ((static_cast<KNCollectionViewItem*>(i))->coll->type()==KNCollection::CTfolder) {
      if (static_cast<KNFolder*>(static_cast<KNCollectionViewItem*>(i)->coll)->isRootFolder()) {
        QMenu *popup = static_cast<QMenu *>(factory()->container("root_folder_popup", m_GUIClient));
        if ( popup )
          popup->popup(p);
      } else {
        QMenu *popup  = static_cast<QMenu *>(factory()->container("folder_popup", m_GUIClient));
        if ( popup )
          popup->popup(p);
      }
    } else {
      QMenu *popup = static_cast<QMenu *>(factory()->container("account_popup", m_GUIClient));
      if ( popup )
        popup->popup( p );
    }
  }
}


void KNMainWidget::slotOpenArticle(Q3ListViewItem *item)
{
  if(b_lockui)
    return;

  if (item) {
    KNArticle *art=(static_cast<KNHdrViewItem*>(item))->art;

    if ((art->type()==KNArticle::ATlocal) && ((f_olManager->currentFolder()==f_olManager->outbox())||
                                               (f_olManager->currentFolder()==f_olManager->drafts()))) {
      a_rtFactory->edit( static_cast<KNLocalArticle*>(art) );
    } else {
      if ( !ArticleWindow::raiseWindowForArticle( art ) ) {
        ArticleWindow *w = new ArticleWindow( art );
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


//------------------------------ <Actions> --------------------------------


void KNMainWidget::slotNavNextUnreadArt()
{
  if ( !h_drView->nextUnreadArticle() )
    c_olView->nextGroup();
}


void KNMainWidget::slotNavNextUnreadThread()
{
  if ( !h_drView->nextUnreadThread() )
    c_olView->nextGroup();
}


void KNMainWidget::slotNavReadThrough()
{
  kDebug(5003) << "KNMainWidget::slotNavReadThrough()" << endl;
  if ( !mArticleViewer->atBottom() )
    mArticleViewer->scrollNext();
  else if(g_rpManager->currentGroup() != 0)
    slotNavNextUnreadArt();
}


void KNMainWidget::slotAccProperties()
{
  kDebug(5003) << "KNMainWidget::slotAccProperties()" << endl;
  if(a_ccManager->currentAccount())
    a_ccManager->editProperties(a_ccManager->currentAccount());
  updateCaption();
  a_rtManager->updateStatusString();
}


void KNMainWidget::slotAccRename()
{
  kDebug(5003) << "KNMainWidget::slotAccRename()" << endl;
  if(a_ccManager->currentAccount()) {
    disableAccels(true);   // hack: global accels break the inplace renaming
    c_olView->rename(a_ccManager->currentAccount()->listItem(), 0);
  }
}


void KNMainWidget::slotAccSubscribe()
{
  kDebug(5003) << "KNMainWidget::slotAccSubscribe()" << endl;
  if(a_ccManager->currentAccount())
    g_rpManager->showGroupDialog(a_ccManager->currentAccount());
}


void KNMainWidget::slotAccExpireAll()
{
  kDebug(5003) << "KNMainWidget::slotAccExpireAll()" << endl;
  if(a_ccManager->currentAccount())
    g_rpManager->expireAll(a_ccManager->currentAccount());
}


void KNMainWidget::slotAccGetNewHdrs()
{
  kDebug(5003) << "KNMainWidget::slotAccGetNewHdrs()" << endl;
  if(a_ccManager->currentAccount())
    g_rpManager->checkAll(a_ccManager->currentAccount());
}



void KNMainWidget::slotAccDelete()
{
  kDebug(5003) << "KNMainWidget::slotAccDelete()" << endl;
  if(a_ccManager->currentAccount()) {
    if (a_ccManager->removeAccount(a_ccManager->currentAccount()))
      slotCollectionSelected(0);
  }
}

void KNMainWidget::slotAccGetNewHdrsAll()
{
  KNAccountManager::List list = a_ccManager->accounts();
  for ( KNAccountManager::List::Iterator it = list.begin(); it != list.end(); ++it )
    g_rpManager->checkAll( *it );
}

void KNMainWidget::slotAccPostNewArticle()
{
  kDebug(5003) << "KNMainWidget::slotAccPostNewArticle()" << endl;
  if(g_rpManager->currentGroup())
    a_rtFactory->createPosting(g_rpManager->currentGroup());
  else if(a_ccManager->currentAccount())
    a_rtFactory->createPosting(a_ccManager->currentAccount());
}


void KNMainWidget::slotGrpProperties()
{
  kDebug(5003) << "slotGrpProperties()" << endl;
  if(g_rpManager->currentGroup())
    g_rpManager->showGroupProperties(g_rpManager->currentGroup());
  updateCaption();
  a_rtManager->updateStatusString();
}


void KNMainWidget::slotGrpRename()
{
  kDebug(5003) << "slotGrpRename()" << endl;
  if(g_rpManager->currentGroup()) {
    disableAccels(true);   // hack: global accels break the inplace renaming
    c_olView->rename(g_rpManager->currentGroup()->listItem(), 0);
  }
}


void KNMainWidget::slotGrpGetNewHdrs()
{
  kDebug(5003) << "KNMainWidget::slotGrpGetNewHdrs()" << endl;
  if(g_rpManager->currentGroup())
    g_rpManager->checkGroupForNewHeaders(g_rpManager->currentGroup());
}


void KNMainWidget::slotGrpExpire()
{
  kDebug(5003) << "KNMainWidget::slotGrpExpire()" << endl;
  if(g_rpManager->currentGroup())
    g_rpManager->expireGroupNow(g_rpManager->currentGroup());
}


void KNMainWidget::slotGrpReorganize()
{
  kDebug(5003) << "KNMainWidget::slotGrpReorganize()" << endl;
  g_rpManager->reorganizeGroup(g_rpManager->currentGroup());
}


void KNMainWidget::slotGrpUnsubscribe()
{
  kDebug(5003) << "KNMainWidget::slotGrpUnsubscribe()" << endl;
  if(g_rpManager->currentGroup()) {
    if(KMessageBox::Yes==KMessageBox::questionYesNo(knGlobals.topWidget,
       i18n("Do you really want to unsubscribe from %1?", g_rpManager->currentGroup()->groupname()), QString(), KGuiItem(i18n("Unsubscribe")), KStandardGuiItem::cancel()))
      if (g_rpManager->unsubscribeGroup(g_rpManager->currentGroup()))
        slotCollectionSelected(0);
  }
}


void KNMainWidget::slotGrpSetAllRead()
{
  kDebug(5003) << "KNMainWidget::slotGrpSetAllRead()" << endl;

  a_rtManager->setAllRead(true);
  if ( knGlobals.settings()->markAllReadGoNext() )
    c_olView->nextGroup();
}


void KNMainWidget::slotGrpSetAllUnread()
{
  kDebug(5003) << "KNMainWidget::slotGrpSetAllUnread()" << endl;
  a_rtManager->setAllRead(false);
}

void KNMainWidget::slotGrpSetUnread()
{
  kDebug(5003) << "KNMainWidget::slotGrpSetUnread()" << endl;
  int groupLength = g_rpManager->currentGroup()->length();

  bool ok = false;
  int res = KInputDialog::getInteger(
                i18n( "Mark Last as Unread" ),
                i18n( "Enter how many articles should be marked unread:" ), groupLength, 1, groupLength, 1, &ok, this );
  if ( ok )
    a_rtManager->setAllRead( false, res );
}

void KNMainWidget::slotFolNew()
{
  kDebug(5003) << "KNMainWidget::slotFolNew()" << endl;
  KNFolder *f = f_olManager->newFolder(0);

  if (f) {
    f_olManager->setCurrentFolder(f);
    c_olView->ensureItemVisible(f->listItem());
    c_olView->setActive( f->listItem() );
    slotFolRename();
  }
}


void KNMainWidget::slotFolNewChild()
{
  kDebug(5003) << "KNMainWidget::slotFolNew()" << endl;
  if(f_olManager->currentFolder()) {
    KNFolder *f = f_olManager->newFolder(f_olManager->currentFolder());

    if (f) {
      f_olManager->setCurrentFolder(f);
      c_olView->ensureItemVisible(f->listItem());
      c_olView->setActive( f->listItem() );
      slotFolRename();
    }
  }
}


void KNMainWidget::slotFolDelete()
{
  kDebug(5003) << "KNMainWidget::slotFolDelete()" << endl;

  if(!f_olManager->currentFolder() || f_olManager->currentFolder()->isRootFolder())
    return;

  if(f_olManager->currentFolder()->isStandardFolder())
    KMessageBox::sorry(knGlobals.topWidget, i18n("You cannot delete a standard folder."));

  else if( KMessageBox::Continue==KMessageBox::warningContinueCancel(knGlobals.topWidget,
      i18n("Do you really want to delete this folder and all its children?"),"",KGuiItem(i18n("&Delete"),"editdelete")) ) {

    if(!f_olManager->deleteFolder(f_olManager->currentFolder()))
      KMessageBox::sorry(knGlobals.topWidget,
      i18n("This folder cannot be deleted because some of\n its articles are currently in use.") );
    else
      slotCollectionSelected(0);
  }
}


void KNMainWidget::slotFolRename()
{
  kDebug(5003) << "KNMainWidget::slotFolRename()" << endl;

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
  kDebug(5003) << "KNMainWidget::slotFolCompact()" << endl;
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder())
    f_olManager->compactFolder(f_olManager->currentFolder());
}


void KNMainWidget::slotFolCompactAll()
{
  kDebug(5003) << "KNMainWidget::slotFolCompactAll()" << endl;
  f_olManager->compactAll();
}


void KNMainWidget::slotFolEmpty()
{
  kDebug(5003) << "KNMainWidget::slotFolEmpty()" << endl;
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
    if(f_olManager->currentFolder()->lockedArticles()>0) {
      KMessageBox::sorry(this,
      i18n("This folder cannot be emptied at the moment\nbecause some of its articles are currently in use.") );
      return;
    }
    if( KMessageBox::Continue == KMessageBox::warningContinueCancel(
        this, i18n("Do you really want to delete all articles in %1?", f_olManager->currentFolder()->name()),"",KGuiItem(i18n("&Delete"),"editdelete")) )
      f_olManager->emptyFolder(f_olManager->currentFolder());
  }
}


void KNMainWidget::slotFolMBoxImport()
{
  kDebug(5003) << "KNMainWidget::slotFolMBoxImport()" << endl;
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
     f_olManager->importFromMBox(f_olManager->currentFolder());
  }
}


void KNMainWidget::slotFolMBoxExport()
{
  kDebug(5003) << "KNMainWidget::slotFolMBoxExport()" << endl;
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
    f_olManager->exportToMBox(f_olManager->currentFolder());
  }
}


void KNMainWidget::slotArtSortHeaders(int i)
{
  kDebug(5003) << "KNMainWidget::slotArtSortHeaders(int i)" << endl;
  h_drView->setSorting( i );
}


void KNMainWidget::slotArtSortHeadersKeyb()
{
  kDebug(5003) << "KNMainWidget::slotArtSortHeadersKeyb()" << endl;

  int newCol = KNHelper::selectDialog(this, i18n("Select Sort Column"), a_ctArtSortHeaders->items(), a_ctArtSortHeaders->currentItem());
  if (newCol != -1)
    h_drView->setSorting( newCol );
}


void KNMainWidget::slotArtSearch()
{
  kDebug(5003) << "KNMainWidget::slotArtSearch()" << endl;
  a_rtManager->search();
}


void KNMainWidget::slotArtRefreshList()
{
  kDebug(5003) << "KNMainWidget::slotArtRefreshList()" << endl;
  a_rtManager->showHdrs(true);
}


void KNMainWidget::slotArtCollapseAll()
{
  kDebug(5003) << "KNMainWidget::slotArtCollapseAll()" << endl;

  closeCurrentThread();
  a_rtManager->setAllThreadsOpen(false);
  if (h_drView->currentItem())
    h_drView->ensureItemVisible(h_drView->currentItem());
}


void KNMainWidget::slotArtExpandAll()
{
  kDebug(5003) << "KNMainWidget::slotArtExpandAll()" << endl;

  a_rtManager->setAllThreadsOpen(true);
  if (h_drView->currentItem())
    h_drView->ensureItemVisible(h_drView->currentItem());
}


void KNMainWidget::slotArtToggleThread()
{
  kDebug(5003) << "KNMainWidget::slotArtToggleThread()" << endl;
  if( mArticleViewer->article() && mArticleViewer->article()->listItem()->isExpandable() ) {
    bool o = !(mArticleViewer->article()->listItem()->isOpen());
    mArticleViewer->article()->listItem()->setOpen( o );
  }
}


void KNMainWidget::slotArtToggleShowThreads()
{
  kDebug(5003) << "KNMainWidget::slotArtToggleShowThreads()" << endl;
  if(g_rpManager->currentGroup()) {
    knGlobals.settings()->setShowThreads( !knGlobals.settings()->showThreads() );
    a_rtManager->showHdrs(true);
  }
}


void KNMainWidget::slotArtSetArtRead()
{
  kDebug(5003) << "KNMainWidget::slotArtSetArtRead()" << endl;
  if(!g_rpManager->currentGroup())
    return;

  KNRemoteArticle::List l;
  getSelectedArticles(l);
  a_rtManager->setRead(l, true);
}


void KNMainWidget::slotArtSetArtUnread()
{
  kDebug(5003) << "KNMainWidget::slotArtSetArtUnread()" << endl;
  if(!g_rpManager->currentGroup())
    return;

  KNRemoteArticle::List l;
  getSelectedArticles(l);
  a_rtManager->setRead(l, false);
}


void KNMainWidget::slotArtSetThreadRead()
{
  kDebug(5003) << "slotArtSetThreadRead()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->setRead(l, true);

  if (h_drView->currentItem()) {
    if ( knGlobals.settings()->markThreadReadCloseThread() )
      closeCurrentThread();
    if ( knGlobals.settings()->markThreadReadGoNext() )
      slotNavNextUnreadThread();
  }
}


void KNMainWidget::slotArtSetThreadUnread()
{
  kDebug(5003) << "KNMainWidget::slotArtSetThreadUnread()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->setRead(l, false);
}


void KNMainWidget::slotScoreEdit()
{
  kDebug(5003) << "KNMainWidget::slotScoreEdit()" << endl;
  s_coreManager->configure();
}


void KNMainWidget::slotReScore()
{
  kDebug(5003) << "KNMainWidget::slotReScore()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  g_rpManager->currentGroup()->scoreArticles(false);
  a_rtManager->showHdrs(true);
}


void KNMainWidget::slotScoreLower()
{
  kDebug(5003) << "KNMainWidget::slotScoreLower() start" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  if ( mArticleViewer->article() && mArticleViewer->article()->type() == KNArticle::ATremote ) {
    KNRemoteArticle *ra = static_cast<KNRemoteArticle*>( mArticleViewer->article() );
    s_coreManager->addRule(KNScorableArticle(ra), g_rpManager->currentGroup()->groupname(), -10);
  }
}


void KNMainWidget::slotScoreRaise()
{
  kDebug(5003) << "KNMainWidget::slotScoreRaise() start" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  if ( mArticleViewer->article() && mArticleViewer->article()->type() == KNArticle::ATremote ) {
    KNRemoteArticle *ra = static_cast<KNRemoteArticle*>( mArticleViewer->article() );
    s_coreManager->addRule(KNScorableArticle(ra), g_rpManager->currentGroup()->groupname(), +10);
  }
}


void KNMainWidget::slotArtToggleIgnored()
{
  kDebug(5003) << "KNMainWidget::slotArtToggleIgnored()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  bool revert = !a_rtManager->toggleIgnored(l);
  a_rtManager->rescoreArticles(l);

  if (h_drView->currentItem() && !revert) {
    if ( knGlobals.settings()->ignoreThreadCloseThread() )
      closeCurrentThread();
    if ( knGlobals.settings()->ignoreThreadGoNext() )
      slotNavNextUnreadThread();
  }
}


void KNMainWidget::slotArtToggleWatched()
{
  kDebug(5003) << "KNMainWidget::slotArtToggleWatched()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->toggleWatched(l);
  a_rtManager->rescoreArticles(l);
}


void KNMainWidget::slotArtOpenNewWindow()
{
  kDebug(5003) << "KNMainWidget::slotArtOpenNewWindow()" << endl;

  if( mArticleViewer->article() ) {
    if ( !ArticleWindow::raiseWindowForArticle( mArticleViewer->article() ) ) {
      ArticleWindow *win = new ArticleWindow( mArticleViewer->article() );
      win->show();
    }
  }
}


void KNMainWidget::slotArtSendOutbox()
{
  kDebug(5003) << "KNMainWidget::slotArtSendOutbox()" << endl;
  a_rtFactory->sendOutbox();
}


void KNMainWidget::slotArtDelete()
{
  kDebug(5003) << "KNMainWidget::slotArtDelete()" << endl;
  if (!f_olManager->currentFolder())
    return;

  KNLocalArticle::List lst;
  getSelectedArticles(lst);

  if(!lst.isEmpty())
    a_rtManager->deleteArticles(lst);

  if(h_drView->currentItem())
    h_drView->setActive( h_drView->currentItem() );
}


void KNMainWidget::slotArtSendNow()
{
  kDebug(5003) << "KNMainWidget::slotArtSendNow()" << endl;
  if (!f_olManager->currentFolder())
    return;

  KNLocalArticle::List lst;
  getSelectedArticles(lst);

  if(!lst.isEmpty())
    a_rtFactory->sendArticles( lst, true );
}


void KNMainWidget::slotArtEdit()
{
  kDebug(5003) << "KNodeVew::slotArtEdit()" << endl;
  if (!f_olManager->currentFolder())
    return;

  if ( mArticleViewer->article() && mArticleViewer->article()->type() == KNArticle::ATlocal )
    a_rtFactory->edit( static_cast<KNLocalArticle*>( mArticleViewer->article() ) );
}


void KNMainWidget::slotNetCancel()
{
  kDebug(5003) << "KNMainWidget::slotNetCancel()" << endl;
  knGlobals.scheduler()->cancelJobs();
}


void KNMainWidget::slotFetchArticleWithID()
{
  kDebug(5003) << "KNMainWidget::slotFetchArticleWithID()" << endl;
  if( !g_rpManager->currentGroup() )
    return;

  FetchArticleIdDlg *dlg = new FetchArticleIdDlg( this );
  dlg->setObjectName( "messageid" );

  if (dlg->exec()) {
    QString id = dlg->messageId().simplified();
    if ( id.indexOf( QRegExp("*@*", Qt::CaseInsensitive, QRegExp::Wildcard) ) != -1 ) {
      if ( id.indexOf( QRegExp("<*>", Qt::CaseInsensitive, QRegExp::Wildcard) ) == -1 )   // add "<>" when necessary
        id = QString("<%1>").arg(id);

      if ( !ArticleWindow::raiseWindowForArticle( id.toLatin1() ) ) { //article not yet opened
        KNRemoteArticle *a=new KNRemoteArticle(g_rpManager->currentGroup());
        a->messageID()->from7BitString(id.toLatin1());
        ArticleWindow *awin = new ArticleWindow( a );
        awin->show();
      }
    }
  }

  KNHelper::saveWindowSize("fetchArticleWithID",dlg->size());
  delete dlg;
}


void KNMainWidget::slotToggleQuickSearch()
{
  if (q_uicksearch->isHidden())
    q_uicksearch->show();
  else
    q_uicksearch->hide();
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
  kDebug(5003)<<"m_guiclient is "<< m_GUIClient
           <<", the factory is " << m_GUIClient->factory() <<endl;
  return m_GUIClient->factory();
}

//--------------------------------


KNode::FetchArticleIdDlg::FetchArticleIdDlg( QWidget *parent ) :
    KDialog( parent )
{
  setCaption( i18n("Fetch Article with ID") );
  setButtons( KDialog::Ok | KDialog::Cancel );
  setModal( true );
  KHBox *page = new KHBox( this );
  setMainWidget( page );

  QLabel *label = new QLabel(i18n("&Message-ID:"),page);
  edit = new KLineEdit(page);
  label->setBuddy(edit);
  edit->setFocus();
  enableButtonOk( false );
  setButtonText( KDialog::Ok, i18n("&Fetch") );
  connect( edit, SIGNAL(textChanged( const QString & )), this, SLOT(slotTextChanged(const QString & )));
  KNHelper::restoreWindowSize("fetchArticleWithID", this, QSize(325,66));
}

QString KNode::FetchArticleIdDlg::messageId() const
{
    return edit->text();
}

void KNode::FetchArticleIdDlg::slotTextChanged(const QString &_text )
{
    enableButtonOk( !_text.isEmpty() );
}


////////////////////////////////////////////////////////////////////////
//////////////////////// DCOP implementation
// Move to the next article
void KNMainWidget::nextArticle()
{
  h_drView->nextArticle();
}

// Move to the previous article
void KNMainWidget::previousArticle()
{
  h_drView->prevArticle();
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
  c_olView->nextGroup();
}

// Move to the previous group
void KNMainWidget::previousGroup()
{
  c_olView->prevGroup();
}

void KNMainWidget::fetchHeaders()
{
  // Simply call the slot
  slotAccGetNewHdrs();
}

void KNMainWidget::expireArticles()
{
  slotAccExpireAll();
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

bool KNMainWidget::handleCommandLine()
{
  bool doneSomething = false;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->count()>0) {
    KUrl url=args->url(0);    // we take only one URL
    openURL(url);
    doneSomething = true;
  }
  args->clear();
  return doneSomething;
}

//////////////////////// end DCOP implementation
////////////////////////////////////////////////////////////////////////

#include "knmainwidget.moc"
