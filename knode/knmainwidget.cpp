/*
    KNode, the KDE newsreader
    Copyright (c) 2003 Zack Rusin <zack@kde.org>
    Copyright (c) 2004-2006 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "knmainwidget.h"

#include "headerview.h"

#include <Q3Accel>
#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QMenu>
#include <QSplitter>
#include <ktoolbar.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kedittoolbar.h>
#include <kstandardaction.h>
#include <kdebug.h>
#include <kmenubar.h>
#include <kiconloader.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <k3listviewsearchline.h>
#include <khbox.h>
#include <kselectaction.h>
#include <kstandardshortcut.h>
#include <ktoggleaction.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <ksqueezedtextlabel.h>

#include "libkdepim/misc/uistatesaver.h"
#include "libkdepim/misc/broadcaststatus.h"
#include "libkdepim/addressline/recentaddresses.h"
using KPIM::BroadcastStatus;
using KPIM::RecentAddresses;

#include <mailtransport/transportmanager.h>
using MailTransport::TransportManager;

//GUI
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
#include "knmemorymanager.h"
#include "scheduler.h"
#include "settings.h"

#include "knodeadaptor.h"

using namespace KNode;

KNMainWidget::KNMainWidget( KXMLGUIClient* client, QWidget* parent ) :
  KVBox( parent ),
  c_olView( 0 ),
  b_lockui( false ),
  m_GUIClient( client )
{
  (void) new KnodeAdaptor( this );
  QDBusConnection::sessionBus().registerObject("/KNode", this);
  knGlobals.top=this;
  knGlobals.topWidget=this;

  //------------------------------- <CONFIG> ----------------------------------
  c_fgManager = knGlobals.configManager();
  //------------------------------- </CONFIG> ----------------------------------

  //-------------------------------- <GUI> ------------------------------------
  // this will enable keyboard-only actions like that don't appear in any menu
  //actionCollection()->setDefaultShortcutContext( Qt::WindowShortcut );

  Q3Accel *accel = new Q3Accel( this );
  initStatusBar();
  setSpacing( 0 );
  setMargin( 0 );
  setLineWidth( 0 );

  // splitters
  mPrimarySplitter = new QSplitter( Qt::Horizontal, this );
  mPrimarySplitter->setObjectName( "mPrimarySplitter" );
  mSecondSplitter = new QSplitter( Qt::Vertical, mPrimarySplitter );
  mSecondSplitter->setObjectName( "mSecondSplitter" );

  //article view
  mArticleViewer = new ArticleWidget( mPrimarySplitter, client, actionCollection(), true/*main viewer*/ );

  //collection view
  c_olView = new KNCollectionView( mSecondSplitter );

  connect( c_olView, SIGNAL(itemSelectionChanged()),
           this, SLOT(slotCollectionSelected()) );
  connect( c_olView, SIGNAL(contextMenu(QTreeWidgetItem*,QPoint)),
           this, SLOT(slotCollectionRMB(QTreeWidgetItem*,QPoint)) );
  connect( c_olView, SIGNAL(renamed(QTreeWidgetItem*)),
           this, SLOT(slotCollectionRenamed(QTreeWidgetItem*)) );

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
  connect(h_drView, SIGNAL(contextMenu(K3ListView*,Q3ListViewItem*,QPoint)),
          SLOT(slotArticleRMB(K3ListView*,Q3ListViewItem*,QPoint)));
  connect(h_drView, SIGNAL(doubleClick(Q3ListViewItem*)),
          SLOT(slotOpenArticle(Q3ListViewItem*)));
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
  a_rtFactory = KNGlobals::self()->articleFactory();

  // Score Manager
  s_coreManager = knGlobals.scoringManager();
  //connect(s_coreManager, SIGNAL(changedRules()), SLOT(slotReScore()));
  connect(s_coreManager, SIGNAL(finishedEditing()), SLOT(slotReScore()));

  QDBusConnection::sessionBus().registerObject( "/", this, QDBusConnection::ExportScriptableSlots );
  //-------------------------------- </CORE> -----------------------------------

  //apply saved options
  readOptions();

  //apply configuration
  configChanged();

  // set the keyboard focus indicator on the first item in the Collection View
  if( c_olView->firstItem() ) {
    QTreeWidgetItem *i = c_olView->firstItem();
    bool open = i->isExpanded();
    c_olView->setActive( i );
    i->setExpanded( open );
  }

  c_olView->setFocus();

  setStatusMsg();

  if( firstStart() ) {  // open the config dialog on the first start
    show();              // the settings dialog must appear in front of the main window!
    slotSettings();
  }

  actionCollection()->addAssociatedWidget( this );
  foreach (QAction* action, actionCollection()->actions())
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

KNMainWidget::~KNMainWidget()
{
  // Avoid that removals of items from c_olView call this object back and lead to a crash.
  disconnect( c_olView, SIGNAL(itemSelectionChanged()),
              this, SLOT(slotCollectionSelected()) );

  knGlobals.reset(); // cleanup

  h_drView->clear(); //avoid some random crashes in KNHdrViewItem::~KNHdrViewItem()
}

void KNMainWidget::initStatusBar()
{
  //statusbar
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(topLevelWidget());
  KStatusBar *sb =  mainWin ? mainWin->statusBar() : 0;
  s_tatusFilter = new KSqueezedTextLabel( QString(), sb );
  s_tatusFilter->setTextElideMode( Qt::ElideRight );
  s_tatusFilter->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  s_tatusGroup = new KSqueezedTextLabel( QString(), sb );
  s_tatusGroup->setTextElideMode( Qt::ElideRight );
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

void KNMainWidget::disableAccels(bool b)
{
#ifdef __GNUC__
#warning Port me!
#endif
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
#ifdef __GNUC__
#warning Port me!
#endif
  /*a_ccel->setEnabled(false);
  KAccel *naccel = mainWin ? mainWin->accel() : 0;
  if ( naccel )
    naccel->setEnabled(false);*/
  installEventFilter(this);

  qApp->processEvents();

  b_lockui = false;
  if ( mbar )
    mbar->setEnabled(true);
#ifdef __GNUC__
#warning Port me!
#endif
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
  kDebug(5003) << url;
  QString host = url.host();
  short int port = url.port();
  KNNntpAccount::Ptr acc;

  if (url.url().left(7) == "news://") {

    // lets see if we already have an account for this host...
    KNNntpAccount::List list = a_ccManager->accounts();
    for ( KNNntpAccount::List::Iterator it = list.begin(); it != list.end(); ++it ) {
      if ( (*it)->server().toLower() == host.toLower() && ( port==-1 || (*it)->port() == port ) ) {
        acc = *it;
        break;
      }
    }

    if(!acc) {
      acc = KNNntpAccount::Ptr( new KNNntpAccount() );
      acc->setName(host);
      acc->setServer(host);

      if(port!=-1)
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
      kDebug(5003) <<"KNMainWidget::openURL() URL is not a valid news URL";
    }
  }

  if (acc) {
    QString decodedUrl = KUrl::fromPercentEncoding( url.url().toLatin1() );
    bool isMID=decodedUrl.contains('@' );

    if (!isMID) {
      QString groupname=url.path( KUrl::RemoveTrailingSlash );
      while(groupname.startsWith('/'))
        groupname.remove(0,1);
      QTreeWidgetItem *item=0;
      if ( groupname.isEmpty() ) {
        item=acc->listItem();
      } else {
        KNGroup::Ptr grp = g_rpManager->group( groupname, acc );

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
        c_olView->setActive( item );
      }
    } else {
      QString groupname = decodedUrl.mid( url.protocol().length()+1 );
      KNGroup::Ptr g = g_rpManager->currentGroup();
      if (g == 0)
        g = g_rpManager->firstGroupOfAccount(acc);

      if (g) {
        if ( !ArticleWindow::raiseWindowForArticle( groupname.toLatin1() ) ) { //article not yet opened
          KNRemoteArticle::Ptr a( new KNRemoteArticle(g) );
          QString messageID = '<' + groupname + '>';
          a->messageID()->from7BitString(messageID.toLatin1());
          ArticleWindow *awin = new ArticleWindow( a );
          awin->show();
        }
      } else {
        // TODO: fetch without group
        kDebug(5003) <<"KNMainWidget::openURL() account has no groups";
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
  a_ctNavNextArt = actionCollection()->addAction("go_nextArticle" );
  a_ctNavNextArt->setText(i18n("&Next Article"));
  a_ctNavNextArt->setToolTip(i18n("Go to next article"));
  a_ctNavNextArt->setShortcuts(KShortcut("N; Right"));
  connect(a_ctNavNextArt, SIGNAL(triggered(bool)), h_drView, SLOT(nextArticle()));

  a_ctNavPrevArt = actionCollection()->addAction("go_prevArticle" );
  a_ctNavPrevArt->setText(i18n("&Previous Article"));
  a_ctNavPrevArt->setShortcuts(KShortcut("P; Left"));
  a_ctNavPrevArt->setToolTip(i18n("Go to previous article"));
  connect(a_ctNavPrevArt, SIGNAL(triggered(bool)), h_drView, SLOT(prevArticle()));

  a_ctNavNextUnreadArt = actionCollection()->addAction("go_nextUnreadArticle");
  a_ctNavNextUnreadArt->setIcon(KIcon("go-next"));
  a_ctNavNextUnreadArt->setText(i18n("Next Unread &Article"));
  connect(a_ctNavNextUnreadArt, SIGNAL(triggered(bool)), SLOT(slotNavNextUnreadArt()));
  a_ctNavNextUnreadArt->setShortcut(QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_Space));

  a_ctNavNextUnreadThread = actionCollection()->addAction("go_nextUnreadThread");
  a_ctNavNextUnreadThread->setIcon(KIcon("go-last"));
  a_ctNavNextUnreadThread->setText(i18n("Next Unread &Thread"));
  connect(a_ctNavNextUnreadThread, SIGNAL(triggered(bool)), SLOT(slotNavNextUnreadThread()));
  a_ctNavNextUnreadThread->setShortcut(QKeySequence(Qt::SHIFT+Qt::Key_Space));

  a_ctNavNextGroup = actionCollection()->addAction("go_nextGroup");
  a_ctNavNextGroup->setIcon(KIcon("go-down"));
  a_ctNavNextGroup->setText(i18n("Ne&xt Group"));
  connect(a_ctNavNextGroup, SIGNAL(triggered(bool)), c_olView, SLOT(nextGroup()));
  a_ctNavNextGroup->setShortcut(QKeySequence(Qt::Key_Plus));

  a_ctNavPrevGroup = actionCollection()->addAction("go_prevGroup");
  a_ctNavPrevGroup->setIcon(KIcon("go-up"));
  a_ctNavPrevGroup->setText(i18n("Pre&vious Group"));
  connect(a_ctNavPrevGroup, SIGNAL(triggered(bool)), c_olView, SLOT(prevGroup()));
  a_ctNavPrevGroup->setShortcut(QKeySequence(Qt::Key_Minus));

  a_ctNavReadThrough = actionCollection()->addAction("go_readThrough");
  a_ctNavReadThrough->setText(i18n("Read &Through Articles"));
  connect(a_ctNavReadThrough, SIGNAL(triggered(bool)), SLOT(slotNavReadThrough()));
  a_ctNavReadThrough->setShortcut(QKeySequence(Qt::Key_Space));

  KAction *action = actionCollection()->addAction("inc_current_folder");
  action->setText(i18n("Focus on Next Folder"));
  connect(action, SIGNAL(triggered(bool)), c_olView, SLOT(incCurrentFolder()));
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Right));

  action = actionCollection()->addAction("dec_current_folder");
  action->setText(i18n("Focus on Previous Folder"));
  connect(action, SIGNAL(triggered(bool)), c_olView, SLOT(decCurrentFolder()));
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Left));

  action = actionCollection()->addAction("select_current_folder");
  action->setText(i18n("Select Folder with Focus"));
  connect(action, SIGNAL(triggered(bool)), c_olView, SLOT(selectCurrentFolder()));
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Space));

  action = actionCollection()->addAction("inc_current_article");
  action->setText(i18n("Focus on Next Article"));
  connect(action, SIGNAL(triggered(bool)), h_drView, SLOT(incCurrentArticle()));
  action->setShortcut(QKeySequence(Qt::ALT+Qt::Key_Right));

  action = actionCollection()->addAction("dec_current_article");
  action->setText(i18n("Focus on Previous Article"));
  connect(action, SIGNAL(triggered(bool)), h_drView, SLOT(decCurrentArticle()));
  action->setShortcut(QKeySequence(Qt::ALT+Qt::Key_Left));

  action = actionCollection()->addAction("select_current_article");
  action->setText(i18n("Select Article with Focus"));
  connect(action, SIGNAL(triggered(bool)), h_drView, SLOT(selectCurrentArticle()));
  action->setShortcut(QKeySequence(Qt::ALT+Qt::Key_Space));

  //collection-view - accounts
  a_ctAccProperties = actionCollection()->addAction("account_properties");
  a_ctAccProperties->setIcon(KIcon("document-properties"));
  a_ctAccProperties->setText(i18n("Account &Properties"));
  connect(a_ctAccProperties, SIGNAL(triggered(bool)), SLOT(slotAccProperties()));

  a_ctAccRename = actionCollection()->addAction("account_rename");
  a_ctAccRename->setIcon(KIcon("edit-rename"));
  a_ctAccRename->setText(i18n("&Rename Account"));
  connect(a_ctAccRename, SIGNAL(triggered(bool)), SLOT(slotAccRename()));

  a_ctAccSubscribe = actionCollection()->addAction("account_subscribe");
  a_ctAccSubscribe->setIcon(KIcon("news-subscribe"));
  a_ctAccSubscribe->setText(i18n("&Subscribe to Newsgroups..."));
  connect(a_ctAccSubscribe, SIGNAL(triggered(bool)), SLOT(slotAccSubscribe()));

  a_ctAccExpireAll = actionCollection()->addAction("account_expire_all");
  a_ctAccExpireAll->setText(i18n("&Expire All Groups"));
  connect(a_ctAccExpireAll, SIGNAL(triggered(bool)), SLOT(slotAccExpireAll()));

  a_ctAccGetNewHdrs = actionCollection()->addAction("account_dnlHeaders");
  a_ctAccGetNewHdrs->setIcon(KIcon("mail-receive"));
  a_ctAccGetNewHdrs->setText(i18n("&Get New Articles in All Groups"));
  connect(a_ctAccGetNewHdrs, SIGNAL(triggered(bool)), SLOT(slotAccGetNewHdrs()));

  a_ctAccGetNewHdrsAll = actionCollection()->addAction("account_dnlAllHeaders");
  a_ctAccGetNewHdrsAll->setIcon(KIcon("mail-receive-all"));
  a_ctAccGetNewHdrsAll->setText(i18n("&Get New Articles in All Accounts"));
  connect(a_ctAccGetNewHdrsAll, SIGNAL(triggered(bool)), SLOT(slotAccGetNewHdrsAll()));

  a_ctAccDelete = actionCollection()->addAction("account_delete");
  a_ctAccDelete->setIcon(KIcon("edit-delete"));
  a_ctAccDelete->setText(i18n("&Delete Account"));
  connect(a_ctAccDelete, SIGNAL(triggered(bool)), SLOT(slotAccDelete()));

  a_ctAccPostNewArticle = actionCollection()->addAction("article_postNew");
  a_ctAccPostNewArticle->setIcon(KIcon("mail-message-new"));
  a_ctAccPostNewArticle->setText(i18n("&Post to Newsgroup..."));
  connect(a_ctAccPostNewArticle, SIGNAL(triggered(bool)), SLOT(slotAccPostNewArticle()));
  a_ctAccPostNewArticle->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_N));

  //collection-view - groups
  a_ctGrpProperties = actionCollection()->addAction("group_properties");
  a_ctGrpProperties->setIcon(KIcon("document-properties"));
  a_ctGrpProperties->setText(i18n("Group &Properties"));
  connect(a_ctGrpProperties, SIGNAL(triggered(bool)), SLOT(slotGrpProperties()));

  a_ctGrpRename = actionCollection()->addAction("group_rename");
  a_ctGrpRename->setIcon(KIcon("edit-rename"));
  a_ctGrpRename->setText(i18n("Rename &Group"));
  connect(a_ctGrpRename, SIGNAL(triggered(bool)), SLOT(slotGrpRename()));

  a_ctGrpGetNewHdrs = actionCollection()->addAction("group_dnlHeaders");
  a_ctGrpGetNewHdrs->setIcon(KIcon("mail-receive"));
  a_ctGrpGetNewHdrs->setText(i18n("&Get New Articles"));
  connect(a_ctGrpGetNewHdrs, SIGNAL(triggered(bool)), SLOT(slotGrpGetNewHdrs()));

  a_ctGrpExpire = actionCollection()->addAction("group_expire");
  a_ctGrpExpire->setText(i18n("E&xpire Group"));
  connect(a_ctGrpExpire, SIGNAL(triggered(bool)), SLOT(slotGrpExpire()));

  a_ctGrpReorganize = actionCollection()->addAction("group_reorg");
  a_ctGrpReorganize->setText(i18n("Re&organize Group"));
  connect(a_ctGrpReorganize, SIGNAL(triggered(bool)), SLOT(slotGrpReorganize()));

  a_ctGrpUnsubscribe = actionCollection()->addAction("group_unsubscribe");
  a_ctGrpUnsubscribe->setIcon(KIcon("news-unsubscribe"));
  a_ctGrpUnsubscribe->setText(i18n("&Unsubscribe From Group"));
  connect(a_ctGrpUnsubscribe, SIGNAL(triggered(bool)), SLOT(slotGrpUnsubscribe()));

  a_ctGrpSetAllRead = actionCollection()->addAction("group_allRead");
  a_ctGrpSetAllRead->setIcon(KIcon("mail-mark-read"));
  a_ctGrpSetAllRead->setText(i18n("Mark All as &Read"));
  connect(a_ctGrpSetAllRead, SIGNAL(triggered(bool)), SLOT(slotGrpSetAllRead()));

  a_ctGrpSetAllUnread = actionCollection()->addAction("group_allUnread");
  a_ctGrpSetAllUnread->setText(i18n("Mark All as U&nread"));
  connect(a_ctGrpSetAllUnread, SIGNAL(triggered(bool)), SLOT(slotGrpSetAllUnread()));

  a_ctGrpSetUnread = actionCollection()->addAction("group_unread");
  a_ctGrpSetUnread->setText(i18n("Mark Last as Unr&ead..."));
  connect(a_ctGrpSetUnread, SIGNAL(triggered(bool)), SLOT(slotGrpSetUnread()));

  action = actionCollection()->addAction("knode_configure_knode");
  action->setIcon(KIcon("configure"));
  action->setText(i18n("&Configure KNode..."));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotSettings()));

  //collection-view - folder
  a_ctFolNew = actionCollection()->addAction("folder_new");
  a_ctFolNew->setIcon(KIcon("folder-new"));
  a_ctFolNew->setText(i18n("&New Folder"));
  connect(a_ctFolNew, SIGNAL(triggered(bool)), SLOT(slotFolNew()));

  a_ctFolNewChild = actionCollection()->addAction("folder_newChild");
  a_ctFolNewChild->setIcon(KIcon("folder-new"));
  a_ctFolNewChild->setText(i18n("New &Subfolder"));
  connect(a_ctFolNewChild, SIGNAL(triggered(bool)), SLOT(slotFolNewChild()));

  a_ctFolDelete = actionCollection()->addAction("folder_delete");
  a_ctFolDelete->setIcon(KIcon("edit-delete"));
  a_ctFolDelete->setText(i18n("&Delete Folder"));
  connect(a_ctFolDelete, SIGNAL(triggered(bool)), SLOT(slotFolDelete()));

  a_ctFolRename = actionCollection()->addAction("folder_rename");
  a_ctFolRename->setIcon(KIcon("edit-rename"));
  a_ctFolRename->setText(i18n("&Rename Folder"));
  connect(a_ctFolRename, SIGNAL(triggered(bool)), SLOT(slotFolRename()));

  a_ctFolCompact = actionCollection()->addAction("folder_compact");
  a_ctFolCompact->setText(i18n("C&ompact Folder"));
  connect(a_ctFolCompact, SIGNAL(triggered(bool)), SLOT(slotFolCompact()));

  a_ctFolCompactAll = actionCollection()->addAction("folder_compact_all");
  a_ctFolCompactAll->setText(i18n("Co&mpact All Folders"));
  connect(a_ctFolCompactAll, SIGNAL(triggered(bool)), SLOT(slotFolCompactAll()));

  a_ctFolEmpty = actionCollection()->addAction("folder_empty");
  a_ctFolEmpty->setText(i18n("&Empty Folder"));
  connect(a_ctFolEmpty, SIGNAL(triggered(bool)), SLOT(slotFolEmpty()));

  a_ctFolMboxImport = actionCollection()->addAction("folder_MboxImport");
  a_ctFolMboxImport->setText(i18n("&Import MBox Folder..."));
  connect(a_ctFolMboxImport, SIGNAL(triggered(bool)), SLOT(slotFolMBoxImport()));

  a_ctFolMboxExport = actionCollection()->addAction("folder_MboxExport");
  a_ctFolMboxExport->setText(i18n("E&xport as MBox Folder..."));
  connect(a_ctFolMboxExport, SIGNAL(triggered(bool)), SLOT(slotFolMBoxExport()));

  //header-view - list-handling
  a_ctArtSortHeaders = actionCollection()->add<KSelectAction>("view_Sort");
  a_ctArtSortHeaders->setText(i18n("S&ort"));
  QStringList items;
  items += i18n("By &Subject");
  items += i18n("By S&ender");
  items += i18n("By S&core");
  items += i18n("By &Lines");
  items += i18n("By &Date");
  a_ctArtSortHeaders->setItems(items);
  a_ctArtSortHeaders->setShortcutConfigurable(false);
  connect(a_ctArtSortHeaders, SIGNAL(activated(int)), this, SLOT(slotArtSortHeaders(int)));

  a_ctArtSortHeadersKeyb = actionCollection()->addAction("view_Sort_Keyb");
  a_ctArtSortHeadersKeyb->setText(i18n("Sort"));
  connect(a_ctArtSortHeadersKeyb, SIGNAL(triggered(bool)), SLOT(slotArtSortHeadersKeyb()));
  a_ctArtSortHeadersKeyb->setShortcut(QKeySequence(Qt::Key_F7));

  a_ctArtFilter = new KNFilterSelectAction(i18n("&Filter"), "view-filter",
                                           actionCollection(), "view_Filter");
  a_ctArtFilter->setShortcutConfigurable(false);

  a_ctArtFilterKeyb = actionCollection()->addAction("view_Filter_Keyb");
  a_ctArtFilterKeyb->setText(i18n("Filter"));
  a_ctArtFilterKeyb->setShortcut(QKeySequence(Qt::Key_F6));

  a_ctArtSearch = actionCollection()->addAction("article_search");
  a_ctArtSearch->setIcon(KIcon("edit-find-mail"));
  a_ctArtSearch->setText(i18n("&Search Articles..."));
  connect(a_ctArtSearch, SIGNAL(triggered(bool)), SLOT(slotArtSearch()));
  a_ctArtSearch->setShortcut(QKeySequence(Qt::Key_F4));

  a_ctArtRefreshList = actionCollection()->addAction("view_Refresh");
  a_ctArtRefreshList->setIcon(KIcon("view-refresh"));
  a_ctArtRefreshList->setText(i18n("&Refresh List"));
  connect(a_ctArtRefreshList, SIGNAL(triggered(bool)), SLOT(slotArtRefreshList()));
  a_ctArtRefreshList->setShortcuts(KStandardShortcut::shortcut(KStandardShortcut::Reload));

  a_ctArtCollapseAll = actionCollection()->addAction("view_CollapseAll");
  a_ctArtCollapseAll->setText(i18n("&Collapse All Threads"));
  connect(a_ctArtCollapseAll, SIGNAL(triggered(bool)), SLOT(slotArtCollapseAll()));

  a_ctArtExpandAll = actionCollection()->addAction("view_ExpandAll");
  a_ctArtExpandAll->setText(i18n("E&xpand All Threads"));
  connect(a_ctArtExpandAll, SIGNAL(triggered(bool)), SLOT(slotArtExpandAll()));

  a_ctArtToggleThread = actionCollection()->addAction("thread_toggle");
  a_ctArtToggleThread->setText(i18n("&Toggle Subthread"));
  connect(a_ctArtToggleThread, SIGNAL(triggered(bool)), SLOT(slotArtToggleThread()));
  a_ctArtToggleThread->setShortcut(QKeySequence(Qt::Key_T));

  a_ctArtToggleShowThreads = actionCollection()->add<KToggleAction>("view_showThreads");
  a_ctArtToggleShowThreads->setText(i18n("Show T&hreads"));
  connect(a_ctArtToggleShowThreads, SIGNAL(triggered(bool)), SLOT(slotArtToggleShowThreads()));

  a_ctArtToggleShowThreads->setChecked( knGlobals.settings()->showThreads() );

  //header-view - remote articles
  a_ctArtSetArtRead = actionCollection()->addAction("article_read");
  a_ctArtSetArtRead->setIcon(KIcon("mail-mark-read"));
  a_ctArtSetArtRead->setText(i18n("Mark as &Read"));
  connect(a_ctArtSetArtRead, SIGNAL(triggered(bool)), SLOT(slotArtSetArtRead()));
  a_ctArtSetArtRead->setShortcut(QKeySequence(Qt::Key_D));

  a_ctArtSetArtUnread = actionCollection()->addAction("article_unread");
  a_ctArtSetArtUnread->setIcon(KIcon("mail-mark-unread"));
  a_ctArtSetArtUnread->setText(i18n("Mar&k as Unread"));
  connect(a_ctArtSetArtUnread, SIGNAL(triggered(bool)), SLOT(slotArtSetArtUnread()));
  a_ctArtSetArtUnread->setShortcut(QKeySequence(Qt::Key_U));

  a_ctArtSetThreadRead = actionCollection()->addAction("thread_read");
  a_ctArtSetThreadRead->setText(i18n("Mark &Thread as Read"));
  connect(a_ctArtSetThreadRead, SIGNAL(triggered(bool)), SLOT(slotArtSetThreadRead()));
  a_ctArtSetThreadRead->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_D));

  a_ctArtSetThreadUnread = actionCollection()->addAction("thread_unread");
  a_ctArtSetThreadUnread->setText(i18n("Mark T&hread as Unread"));
  connect(a_ctArtSetThreadUnread, SIGNAL(triggered(bool)), SLOT(slotArtSetThreadUnread()));
  a_ctArtSetThreadUnread->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_U));

  a_ctArtOpenNewWindow = actionCollection()->addAction("article_ownWindow");
  a_ctArtOpenNewWindow->setIcon(KIcon("window-new"));
  a_ctArtOpenNewWindow->setText(i18n("Open in Own &Window"));
  connect(a_ctArtOpenNewWindow, SIGNAL(triggered(bool)), SLOT(slotArtOpenNewWindow()));
  a_ctArtOpenNewWindow->setShortcut(QKeySequence(Qt::Key_O));

  // scoring
  a_ctScoresEdit = actionCollection()->addAction("scoreedit");
  a_ctScoresEdit->setIcon(KIcon("document-properties"));
  a_ctScoresEdit->setText(i18n("&Edit Scoring Rules..."));
  connect(a_ctScoresEdit, SIGNAL(triggered(bool)), SLOT(slotScoreEdit()));
  a_ctScoresEdit->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_E));

  a_ctReScore = actionCollection()->addAction("rescore");
  a_ctReScore->setText(i18n("Recalculate &Scores"));
  connect(a_ctReScore, SIGNAL(triggered(bool)), SLOT(slotReScore()));

  a_ctScoreLower = actionCollection()->addAction("scorelower");
  a_ctScoreLower->setText(i18n("&Lower Score for Author..."));
  connect(a_ctScoreLower, SIGNAL(triggered(bool)), SLOT(slotScoreLower()));
  a_ctScoreLower->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_L));

  a_ctScoreRaise = actionCollection()->addAction("scoreraise");
  a_ctScoreRaise->setText(i18n("&Raise Score for Author..."));
  connect(a_ctScoreRaise, SIGNAL(triggered(bool)), SLOT(slotScoreRaise()));
  a_ctScoreRaise->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_I));

  a_ctArtToggleIgnored = actionCollection()->addAction("thread_ignore");
  a_ctArtToggleIgnored->setIcon(KIcon("go-bottom"));
  a_ctArtToggleIgnored->setText(i18n("&Ignore Thread"));
  connect(a_ctArtToggleIgnored, SIGNAL(triggered(bool)), SLOT(slotArtToggleIgnored()));
  a_ctArtToggleIgnored->setShortcut(QKeySequence(Qt::Key_I));

  a_ctArtToggleWatched = actionCollection()->addAction("thread_watch");
  a_ctArtToggleWatched->setIcon(KIcon("go-top"));
  a_ctArtToggleWatched->setText(i18n("&Watch Thread"));
  connect(a_ctArtToggleWatched, SIGNAL(triggered(bool)), SLOT(slotArtToggleWatched()));
  a_ctArtToggleWatched->setShortcut(QKeySequence(Qt::Key_W));

  //header-view local articles
  a_ctArtSendOutbox = actionCollection()->addAction("net_sendPending");
  a_ctArtSendOutbox->setIcon(KIcon("mail-send"));
  a_ctArtSendOutbox->setText(i18n("Sen&d Pending Messages"));
  connect(a_ctArtSendOutbox, SIGNAL(triggered(bool)), SLOT(slotArtSendOutbox()));

  a_ctArtDelete = actionCollection()->addAction("article_delete");
  a_ctArtDelete->setIcon(KIcon("edit-delete"));
  a_ctArtDelete->setText(i18n("&Delete Article"));
  connect(a_ctArtDelete, SIGNAL(triggered(bool)), SLOT(slotArtDelete()));
  a_ctArtDelete->setShortcut(QKeySequence(Qt::Key_Delete));

  a_ctArtSendNow = actionCollection()->addAction("article_sendNow");
  a_ctArtSendNow->setIcon(KIcon("mail-send"));
  a_ctArtSendNow->setText(i18n("Send &Now"));
  connect(a_ctArtSendNow, SIGNAL(triggered(bool)), SLOT(slotArtSendNow()));

  a_ctArtEdit = actionCollection()->addAction("article_edit");
  a_ctArtEdit->setIcon(KIcon("document-properties"));
  a_ctArtEdit->setText(i18nc("edit article","&Edit Article..."));
  connect(a_ctArtEdit, SIGNAL(triggered(bool)), SLOT(slotArtEdit()));
  a_ctArtEdit->setShortcut(QKeySequence(Qt::Key_E));

  //network
  a_ctNetCancel = actionCollection()->addAction("net_stop");
  a_ctNetCancel->setIcon(KIcon("process-stop"));
  a_ctNetCancel->setText(i18n("Stop &Network"));
  connect(a_ctNetCancel, SIGNAL(triggered(bool)), SLOT(slotNetCancel()));
  a_ctNetCancel->setEnabled(false);

  a_ctFetchArticleWithID = actionCollection()->addAction("fetch_article_with_id");
  a_ctFetchArticleWithID->setText(i18n("&Fetch Article with ID..."));
  connect(a_ctFetchArticleWithID, SIGNAL(triggered(bool)), SLOT(slotFetchArticleWithID()));
  a_ctFetchArticleWithID->setEnabled(false);

  a_ctToggleQuickSearch = actionCollection()->add<KToggleAction>("settings_show_quickSearch");
  a_ctToggleQuickSearch->setText(i18n("Show Quick Search"));
  connect(a_ctToggleQuickSearch, SIGNAL(triggered(bool)), SLOT(slotToggleQuickSearch()));
}

bool KNMainWidget::firstStart()
{
  KConfigGroup conf(knGlobals.config(), "GENERAL");
  QString ver = conf.readEntry("Version");
  if(!ver.isEmpty())
    return false;

  if ( TransportManager::self()->isEmpty() )
    TransportManager::self()->createDefaultTransport();

  conf.writeEntry("Version", KNODE_VERSION);

  return true;
}


void KNMainWidget::readOptions()
{
  KConfigGroup conf(knGlobals.config(), "APPEARANCE");

  if (conf.readEntry("quicksearch", true))
    a_ctToggleQuickSearch->setChecked(true);
  else
    q_uicksearch->hide();
  c_olView->readConfig();
  h_drView->readConfig();
  a_ctArtSortHeaders->setCurrentItem( h_drView->sortColumn() );

  resize(787,478);  // default optimized for 800x600
  //applyMainWindowSettings(KGlobal::config(),"mainWindow_options");

  KPIM::UiStateSaver::restoreState( this, KConfigGroup( knGlobals.config(), "UI State" ) );
}


void KNMainWidget::saveOptions()
{
  KConfigGroup conf(knGlobals.config(), "APPEARANCE");

  conf.writeEntry("quicksearch", !q_uicksearch->isHidden());
  //saveMainWindowSettings(KGlobal::config(),"mainWindow_options");

  c_olView->writeConfig();
  h_drView->writeConfig();
  mArticleViewer->writeConfig();

  KConfigGroup cfg( knGlobals.config(), "UI State" );
  KPIM::UiStateSaver::saveState( this, cfg );
}


bool KNMainWidget::requestShutdown()
{
  kDebug(5003) <<"KNMainWidget::requestShutdown()";

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
  kDebug(5003) <<"KNMainWidget::prepareShutdown()";

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
      l.append( boost::static_pointer_cast<KNArticle>( static_cast<KNHdrViewItem*>( i )->art ) );
}


void KNMainWidget::getSelectedArticles(KNRemoteArticle::List &l)
{
  if(!g_rpManager->currentGroup()) return;

  for(Q3ListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected() || (static_cast<KNHdrViewItem*>(i)->isActive()))
      l.append( boost::static_pointer_cast<KNRemoteArticle>( static_cast<KNHdrViewItem*>(i)->art ) );
}


void KNMainWidget::getSelectedThreads(KNRemoteArticle::List &l)
{
  KNRemoteArticle::Ptr art;
  for(Q3ListViewItem *i=h_drView->firstChild(); i; i=i->itemBelow())
    if(i->isSelected() || (static_cast<KNHdrViewItem*>(i)->isActive())) {
      art = boost::static_pointer_cast<KNRemoteArticle>( static_cast<KNHdrViewItem*>( i )->art );
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
      l.append( boost::static_pointer_cast<KNLocalArticle>( static_cast<KNHdrViewItem*>(i)->art ) );
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
  kDebug(5003) <<"KNMainWidget::slotArticleSelected(QListViewItem *i)";
  if(b_lockui)
    return;
  KNArticle::Ptr selectedArticle;

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


void KNMainWidget::slotCollectionSelected()
{
  kDebug(5003) <<"KNMainWidget::slotCollectionSelected(QListViewItem *i)";
  if(b_lockui)
    return;
  KNCollection::Ptr c;
  KNNntpAccount::Ptr selectedAccount;
  KNGroup::Ptr selectedGroup;
  KNFolder::Ptr selectedFolder;

  s_earchLineEdit->clear();
  h_drView->clear();
  slotArticleSelected(0);

  // mark all articles in current group as not new/read
  if ( knGlobals.settings()->leaveGroupMarkAsRead() )
    a_rtManager->setAllRead( true );
  a_rtManager->setAllNotNew();

  QTreeWidgetItem *i = c_olView->selectedItems().value( 0 ); // Single item selection
  if(i) {
    c = static_cast<KNCollectionViewItem*>( i )->collection();
    switch(c->type()) {
      case KNCollection::CTnntpAccount :
        selectedAccount = boost::static_pointer_cast<KNNntpAccount>( c );
        if( !i->isExpanded() ) {
          i->setExpanded( true );
        }
      break;
      case KNCollection::CTgroup :
        if ( !h_drView->hasFocus() && !mArticleViewer->hasFocus() )
          h_drView->setFocus();
        selectedGroup = boost::static_pointer_cast<KNGroup>( c );
        selectedAccount=selectedGroup->account();
      break;

      case KNCollection::CTfolder :
        if ( !h_drView->hasFocus() && !mArticleViewer->hasFocus() )
          h_drView->setFocus();
        selectedFolder = boost::static_pointer_cast<KNFolder>( c );
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
    //Laurent fix me
    //a_ctAccPostNewArticle->setEnabled(enabled);
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


void KNMainWidget::slotCollectionRenamed(QTreeWidgetItem *i)
{
  kDebug(5003) <<"KNMainWidget::slotCollectionRenamed(QListViewItem *i)";

  if (i) {
    static_cast<KNCollectionViewItem*>( i )->collection()->setName( i->text( 0 ) );
    updateCaption();
    a_rtManager->updateStatusString();
    if ( static_cast<KNCollectionViewItem*>( i )->collection()->type() == KNCollection::CTnntpAccount ) {
      a_ccManager->accountRenamed( boost::static_pointer_cast<KNNntpAccount>( static_cast<KNCollectionViewItem*>( i )->collection() ) );
    }
    disableAccels(false);
  }
}


void KNMainWidget::slotArticleRMB(K3ListView*, Q3ListViewItem *i, const QPoint &p)
{
  if(b_lockui)
    return;

  if(i) {
    QMenu *popup;
    if( (static_cast<KNHdrViewItem*>(i))->art->type()==KNArticle::ATremote) {
     popup = popupMenu( "remote_popup" );
    } else {
     popup = popupMenu( "local_popup" );
    }

    if ( popup )
      popup->popup(p);
  }
}


void KNMainWidget::slotCollectionRMB( QTreeWidgetItem *i, const QPoint &pos )
{
  if(b_lockui)
    return;

  if(i) {
    QMenu *popup = 0;
    if( static_cast<KNCollectionViewItem*>( i )->collection()->type() == KNCollection::CTgroup ) {
      popup = popupMenu( "group_popup" );
    } else if ( static_cast<KNCollectionViewItem*>( i )->collection()->type() == KNCollection::CTfolder ) {
      if ( boost::static_pointer_cast<KNFolder>( static_cast<KNCollectionViewItem*>( i )->collection() )->isRootFolder() ) {
        popup = popupMenu( "root_folder_popup" );
      } else {
        popup = popupMenu( "folder_popup" );
      }
    } else {
      popup = popupMenu( "account_popup" );
    }
    if ( popup ) {
      popup->popup( pos );
    }
  }
}


void KNMainWidget::slotOpenArticle(Q3ListViewItem *item)
{
  if(b_lockui)
    return;

  if (item) {
    KNArticle::Ptr art = (static_cast<KNHdrViewItem*>(item))->art;

    if ((art->type()==KNArticle::ATlocal) && ((f_olManager->currentFolder()==f_olManager->outbox())||
                                               (f_olManager->currentFolder()==f_olManager->drafts()))) {
      a_rtFactory->edit( boost::static_pointer_cast<KNLocalArticle>( art ) );
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
  kDebug(5003) <<"KNMainWidget::slotNavReadThrough()";
  if ( !mArticleViewer->atBottom() )
    mArticleViewer->scrollNext();
  else if(g_rpManager->currentGroup() != 0)
    slotNavNextUnreadArt();
}


void KNMainWidget::slotAccProperties()
{
  kDebug(5003) <<"KNMainWidget::slotAccProperties()";
  if(a_ccManager->currentAccount())
    a_ccManager->editProperties(a_ccManager->currentAccount());
  updateCaption();
  a_rtManager->updateStatusString();
}


void KNMainWidget::slotAccRename()
{
  kDebug(5003) <<"KNMainWidget::slotAccRename()";
  if(a_ccManager->currentAccount()) {
//     disableAccels(true);   // hack: global accels break the inplace renaming
    c_olView->editItem( a_ccManager->currentAccount()->listItem(), c_olView->labelColumnIndex() );
  }
}


void KNMainWidget::slotAccSubscribe()
{
  kDebug(5003) <<"KNMainWidget::slotAccSubscribe()";
  if(a_ccManager->currentAccount())
    g_rpManager->showGroupDialog(a_ccManager->currentAccount());
}


void KNMainWidget::slotAccExpireAll()
{
  kDebug(5003) <<"KNMainWidget::slotAccExpireAll()";
  if(a_ccManager->currentAccount())
    g_rpManager->expireAll(a_ccManager->currentAccount());
}


void KNMainWidget::slotAccGetNewHdrs()
{
  kDebug(5003) <<"KNMainWidget::slotAccGetNewHdrs()";
  if(a_ccManager->currentAccount())
    g_rpManager->checkAll(a_ccManager->currentAccount());
}



void KNMainWidget::slotAccDelete()
{
  kDebug(5003) <<"KNMainWidget::slotAccDelete()";
  if(a_ccManager->currentAccount()) {
    if (a_ccManager->removeAccount(a_ccManager->currentAccount()))
      slotCollectionSelected();
  }
}

void KNMainWidget::slotAccGetNewHdrsAll()
{
  KNNntpAccount::List list = a_ccManager->accounts();
  for ( KNNntpAccount::List::Iterator it = list.begin(); it != list.end(); ++it )
    g_rpManager->checkAll( *it );
}

void KNMainWidget::slotAccPostNewArticle()
{
  kDebug(5003) <<"KNMainWidget::slotAccPostNewArticle()";
  if(g_rpManager->currentGroup())
    a_rtFactory->createPosting(g_rpManager->currentGroup());
  else if(a_ccManager->currentAccount())
    a_rtFactory->createPosting(a_ccManager->currentAccount());
}


void KNMainWidget::slotGrpProperties()
{
  kDebug(5003) <<"slotGrpProperties()";
  if(g_rpManager->currentGroup())
    g_rpManager->showGroupProperties(g_rpManager->currentGroup());
  updateCaption();
  a_rtManager->updateStatusString();
}


void KNMainWidget::slotGrpRename()
{
  kDebug(5003) <<"slotGrpRename()";
  if(g_rpManager->currentGroup()) {
//     disableAccels(true);   // hack: global accels break the inplace renaming
    c_olView->editItem( g_rpManager->currentGroup()->listItem(),  c_olView->labelColumnIndex() );
  }
}


void KNMainWidget::slotGrpGetNewHdrs()
{
  kDebug(5003) <<"KNMainWidget::slotGrpGetNewHdrs()";
  if(g_rpManager->currentGroup())
    g_rpManager->checkGroupForNewHeaders(g_rpManager->currentGroup());
}


void KNMainWidget::slotGrpExpire()
{
  kDebug(5003) <<"KNMainWidget::slotGrpExpire()";
  if(g_rpManager->currentGroup())
    g_rpManager->expireGroupNow(g_rpManager->currentGroup());
}


void KNMainWidget::slotGrpReorganize()
{
  kDebug(5003) <<"KNMainWidget::slotGrpReorganize()";
  g_rpManager->reorganizeGroup(g_rpManager->currentGroup());
}


void KNMainWidget::slotGrpUnsubscribe()
{
  kDebug(5003) <<"KNMainWidget::slotGrpUnsubscribe()";
  if(g_rpManager->currentGroup()) {
    if(KMessageBox::Yes==KMessageBox::questionYesNo(knGlobals.topWidget,
       i18n("Do you really want to unsubscribe from %1?", g_rpManager->currentGroup()->groupname()), QString(), KGuiItem(i18n("Unsubscribe")), KStandardGuiItem::cancel()))
      if (g_rpManager->unsubscribeGroup(g_rpManager->currentGroup()))
        slotCollectionSelected();
  }
}


void KNMainWidget::slotGrpSetAllRead()
{
  kDebug(5003) <<"KNMainWidget::slotGrpSetAllRead()";

  a_rtManager->setAllRead(true);
  if ( knGlobals.settings()->markAllReadGoNext() )
    c_olView->nextGroup();
}


void KNMainWidget::slotGrpSetAllUnread()
{
  kDebug(5003) <<"KNMainWidget::slotGrpSetAllUnread()";
  a_rtManager->setAllRead(false);
}

void KNMainWidget::slotGrpSetUnread()
{
  kDebug(5003) <<"KNMainWidget::slotGrpSetUnread()";
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
  kDebug(5003) <<"KNMainWidget::slotFolNew()";
  KNFolder::Ptr f = f_olManager->newFolder( KNFolder::Ptr() );

  if (f) {
    f_olManager->setCurrentFolder(f);
    c_olView->setActive( f->listItem() );
    slotFolRename();
  }
}


void KNMainWidget::slotFolNewChild()
{
  kDebug(5003) <<"KNMainWidget::slotFolNew()";
  if(f_olManager->currentFolder()) {
    KNFolder::Ptr f = f_olManager->newFolder( f_olManager->currentFolder() );

    if (f) {
      f_olManager->setCurrentFolder(f);
      c_olView->setActive( f->listItem() );
      slotFolRename();
    }
  }
}


void KNMainWidget::slotFolDelete()
{
  kDebug(5003) <<"KNMainWidget::slotFolDelete()";

  if(!f_olManager->currentFolder() || f_olManager->currentFolder()->isRootFolder())
    return;

  if(f_olManager->currentFolder()->isStandardFolder())
    KMessageBox::sorry(knGlobals.topWidget, i18n("You cannot delete a standard folder."));

  else if( KMessageBox::Continue==KMessageBox::warningContinueCancel(knGlobals.topWidget,
      i18n("Do you really want to delete this folder and all its children?"),"",KGuiItem(i18n("&Delete"),"edit-delete")) ) {

    if(!f_olManager->deleteFolder(f_olManager->currentFolder()))
      KMessageBox::sorry(knGlobals.topWidget,
      i18n("This folder cannot be deleted because some of\n its articles are currently in use.") );
    else
      slotCollectionSelected();
  }
}


void KNMainWidget::slotFolRename()
{
  kDebug(5003) <<"KNMainWidget::slotFolRename()";

  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
    if(f_olManager->currentFolder()->isStandardFolder())
      KMessageBox::sorry(knGlobals.topWidget, i18n("You cannot rename a standard folder."));
    else {
//       disableAccels(true);   // hack: global accels break the inplace renaming
      c_olView->editItem( f_olManager->currentFolder()->listItem(), c_olView->labelColumnIndex() );
    }
  }
}


void KNMainWidget::slotFolCompact()
{
  kDebug(5003) <<"KNMainWidget::slotFolCompact()";
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder())
    f_olManager->compactFolder(f_olManager->currentFolder());
}


void KNMainWidget::slotFolCompactAll()
{
  kDebug(5003) <<"KNMainWidget::slotFolCompactAll()";
  f_olManager->compactAll();
}


void KNMainWidget::slotFolEmpty()
{
  kDebug(5003) <<"KNMainWidget::slotFolEmpty()";
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
    if(f_olManager->currentFolder()->lockedArticles()>0) {
      KMessageBox::sorry(this,
      i18n("This folder cannot be emptied at the moment\nbecause some of its articles are currently in use.") );
      return;
    }
    if( KMessageBox::Continue == KMessageBox::warningContinueCancel(
        this, i18n("Do you really want to delete all articles in %1?", f_olManager->currentFolder()->name()),"",KGuiItem(i18n("&Delete"),"edit-delete")) )
      f_olManager->emptyFolder(f_olManager->currentFolder());
  }
}


void KNMainWidget::slotFolMBoxImport()
{
  kDebug(5003) <<"KNMainWidget::slotFolMBoxImport()";
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
     f_olManager->importFromMBox(f_olManager->currentFolder());
  }
}


void KNMainWidget::slotFolMBoxExport()
{
  kDebug(5003) <<"KNMainWidget::slotFolMBoxExport()";
  if(f_olManager->currentFolder() && !f_olManager->currentFolder()->isRootFolder()) {
    f_olManager->exportToMBox(f_olManager->currentFolder());
  }
}


void KNMainWidget::slotArtSortHeaders(int i)
{
  kDebug(5003) <<"KNMainWidget::slotArtSortHeaders(int i)";
  h_drView->setSorting( i );
}


void KNMainWidget::slotArtSortHeadersKeyb()
{
  kDebug(5003) <<"KNMainWidget::slotArtSortHeadersKeyb()";

  int newCol = KNHelper::selectDialog(this, i18n("Select Sort Column"), a_ctArtSortHeaders->items(), a_ctArtSortHeaders->currentItem());
  if (newCol != -1)
    h_drView->setSorting( newCol );
}


void KNMainWidget::slotArtSearch()
{
  kDebug(5003) <<"KNMainWidget::slotArtSearch()";
  a_rtManager->search();
}


void KNMainWidget::slotArtRefreshList()
{
  kDebug(5003) <<"KNMainWidget::slotArtRefreshList()";
  a_rtManager->showHdrs(true);
}


void KNMainWidget::slotArtCollapseAll()
{
  kDebug(5003) <<"KNMainWidget::slotArtCollapseAll()";

  closeCurrentThread();
  a_rtManager->setAllThreadsOpen(false);
  if (h_drView->currentItem())
    h_drView->ensureItemVisible(h_drView->currentItem());
}


void KNMainWidget::slotArtExpandAll()
{
  kDebug(5003) <<"KNMainWidget::slotArtExpandAll()";

  a_rtManager->setAllThreadsOpen(true);
  if (h_drView->currentItem())
    h_drView->ensureItemVisible(h_drView->currentItem());
}


void KNMainWidget::slotArtToggleThread()
{
  kDebug(5003) <<"KNMainWidget::slotArtToggleThread()";
  if( mArticleViewer->article() && mArticleViewer->article()->listItem()->isExpandable() ) {
    bool o = !(mArticleViewer->article()->listItem()->isOpen());
    mArticleViewer->article()->listItem()->setOpen( o );
  }
}


void KNMainWidget::slotArtToggleShowThreads()
{
  kDebug(5003) <<"KNMainWidget::slotArtToggleShowThreads()";
  if(g_rpManager->currentGroup()) {
    knGlobals.settings()->setShowThreads( !knGlobals.settings()->showThreads() );
    a_rtManager->showHdrs(true);
  }
}


void KNMainWidget::slotArtSetArtRead()
{
  kDebug(5003) <<"KNMainWidget::slotArtSetArtRead()";
  if(!g_rpManager->currentGroup())
    return;

  KNRemoteArticle::List l;
  getSelectedArticles(l);
  a_rtManager->setRead(l, true);
}


void KNMainWidget::slotArtSetArtUnread()
{
  kDebug(5003) <<"KNMainWidget::slotArtSetArtUnread()";
  if(!g_rpManager->currentGroup())
    return;

  KNRemoteArticle::List l;
  getSelectedArticles(l);
  a_rtManager->setRead(l, false);
}


void KNMainWidget::slotArtSetThreadRead()
{
  kDebug(5003) <<"slotArtSetThreadRead()";
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
  kDebug(5003) <<"KNMainWidget::slotArtSetThreadUnread()";
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->setRead(l, false);
}


void KNMainWidget::slotScoreEdit()
{
  kDebug(5003) <<"KNMainWidget::slotScoreEdit()";
  s_coreManager->configure();
}


void KNMainWidget::slotReScore()
{
  kDebug(5003) <<"KNMainWidget::slotReScore()";
  if( !g_rpManager->currentGroup() )
    return;

  g_rpManager->currentGroup()->scoreArticles(false);
  a_rtManager->showHdrs(true);
}


void KNMainWidget::slotScoreLower()
{
  kDebug(5003) <<"KNMainWidget::slotScoreLower() start";
  if( !g_rpManager->currentGroup() )
    return;

  if ( mArticleViewer->article() && mArticleViewer->article()->type() == KNArticle::ATremote ) {
    KNRemoteArticle::Ptr ra = boost::static_pointer_cast<KNRemoteArticle>( mArticleViewer->article() );
    s_coreManager->addRule(KNScorableArticle(ra), g_rpManager->currentGroup()->groupname(), -10);
  }
}


void KNMainWidget::slotScoreRaise()
{
  kDebug(5003) <<"KNMainWidget::slotScoreRaise() start";
  if( !g_rpManager->currentGroup() )
    return;

  if ( mArticleViewer->article() && mArticleViewer->article()->type() == KNArticle::ATremote ) {
    KNRemoteArticle::Ptr ra = boost::static_pointer_cast<KNRemoteArticle>( mArticleViewer->article() );
    s_coreManager->addRule(KNScorableArticle(ra), g_rpManager->currentGroup()->groupname(), +10);
  }
}


void KNMainWidget::slotArtToggleIgnored()
{
  kDebug(5003) <<"KNMainWidget::slotArtToggleIgnored()";
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
  kDebug(5003) <<"KNMainWidget::slotArtToggleWatched()";
  if( !g_rpManager->currentGroup() )
    return;

  KNRemoteArticle::List l;
  getSelectedThreads(l);
  a_rtManager->toggleWatched(l);
  a_rtManager->rescoreArticles(l);
}


void KNMainWidget::slotArtOpenNewWindow()
{
  kDebug(5003) <<"KNMainWidget::slotArtOpenNewWindow()";

  if( mArticleViewer->article() ) {
    if ( !ArticleWindow::raiseWindowForArticle( mArticleViewer->article() ) ) {
      ArticleWindow *win = new ArticleWindow( mArticleViewer->article() );
      win->show();
    }
  }
}


void KNMainWidget::slotArtSendOutbox()
{
  kDebug(5003) <<"KNMainWidget::slotArtSendOutbox()";
  a_rtFactory->sendOutbox();
}


void KNMainWidget::slotArtDelete()
{
  kDebug(5003) <<"KNMainWidget::slotArtDelete()";
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
  kDebug(5003) <<"KNMainWidget::slotArtSendNow()";
  if (!f_olManager->currentFolder())
    return;

  KNLocalArticle::List lst;
  getSelectedArticles(lst);

  if(!lst.isEmpty())
    a_rtFactory->sendArticles( lst, true );
}


void KNMainWidget::slotArtEdit()
{
  kDebug(5003) <<"KNodeVew::slotArtEdit()";
  if (!f_olManager->currentFolder())
    return;

  if ( mArticleViewer->article() && mArticleViewer->article()->type() == KNArticle::ATlocal )
    a_rtFactory->edit( boost::static_pointer_cast<KNLocalArticle>( mArticleViewer->article() ) );
}


void KNMainWidget::slotNetCancel()
{
  kDebug(5003) <<"KNMainWidget::slotNetCancel()";
  knGlobals.scheduler()->cancelJobs();
}


void KNMainWidget::slotFetchArticleWithID()
{
  kDebug(5003) <<"KNMainWidget::slotFetchArticleWithID()";
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
        KNRemoteArticle::Ptr a( new KNRemoteArticle( g_rpManager->currentGroup() ) );
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

QMenu * KNMainWidget::popupMenu( const QString &name ) const
{
  Q_ASSERT( m_GUIClient );
  Q_ASSERT( m_GUIClient->factory() );
  return static_cast<QMenu*>( m_GUIClient->factory()->container( name, m_GUIClient ) );
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
  connect( edit, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
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
//////////////////////// D-Bus implementation
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

//////////////////////// end D-Bus implementation
////////////////////////////////////////////////////////////////////////

