/*
    knmainwidget.h

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
#ifndef KNMAINWIDGET_H
#define KNMAINWIDGET_H

#include "knodeiface.h"

#include <kdockwidget.h>
#include <kdialogbase.h>
#include "resource.h"

#include <qglobal.h>
#include <kdepimmacros.h>

class QListViewItem;

class KURL;
class KAccel;
class KAction;
class KToggleAction;
class KSelectAction;
class KStatusBarLabel;
class KLineEdit;
class KXMLGUIClient;

class KNHeaderView;
class KNCollectionView;
class KNCollectionViewItem;
class KNProgress;
class KNConfigManager;
class KNAccountManager;
class KNGroupManager;
class KNFolderManager;
class KNArticleManager;
class KNArticleFactory;
class KNFilterManager;
class KNScoringManager;
class KNMemoryManager;
class KNFilterSelectAction;
class KNNetAccess;
namespace Kpgp {
    class Module;
}
class KNArticleWidget;
class KNArticle;
class KNLocalArticle;
class KNRemoteArticle;
class KActionCollection;

class KDE_EXPORT KNMainWidget : public KDockArea, virtual public KNodeIface
{
  Q_OBJECT
public:
  KNMainWidget( KXMLGUIClient *client, bool detachable, QWidget* parent, const char* name );
  ~KNMainWidget();

  /** exit */
  bool queryClose();
  void prepareShutdown();

  //GUI
  void setStatusMsg(const QString& = QString::null, int id=SB_MAIN);
  void setStatusHelpMsg(const QString& text);
  void updateCaption();
  void setCursorBusy(bool b=true);
  void blockUI(bool b=true);
  void disableAccels(bool b=true);
  /** processEvents with some blocking */
  void secureProcessEvents();

  /** useful default value */
  virtual QSize sizeHint() const;

  /** handle URL given as command-line argument */
  void openURL(const KURL &url);

  /** update fonts and colors */
  void configChanged();

  /** access to GUI-elements */
  KNCollectionView* collectionView()const  { return c_olView; }
  KNHeaderView*       headerView()const      { return h_drView; }
  KNArticleWidget*  articleView()const     { return a_rtView; }
  KStatusBarLabel*  statusBarLabelGroup() const { return s_tatusGroup; }
  KStatusBarLabel*  statusBarLabelFilter() const { return s_tatusFilter; }
  public: //The dcop interface
  // Implementation of KNodeIface
  /* Navigation */
  // Move to the next article
  virtual void nextArticle();
  // Move to the previous article
  virtual void previousArticle();
  // Move to the next unread article
  virtual void nextUnreadArticle();
  // Move to the next unread thread
  virtual void nextUnreadThread();
  // Move to the next group
  virtual void nextGroup();
  // Move to the previous group
  virtual void previousGroup();

  /* Group options */
  // Open the editor to post a new article in the selected group
  virtual void postArticle();
  // Fetch the new headers in the selected groups
  virtual void fetchHeadersInCurrentGroup();
  // Expire the articles in the current group
  virtual void expireArticlesInCurrentGroup();
  // Mark all the articles in the current group as read
  virtual void markAllAsRead();
  // Mark all the articles in the current group as unread
  virtual void markAllAsUnread();

  /* Header view */
  // Mark the current article as read
  virtual void markAsRead();
  // Mark the current article as unread
  virtual void markAsUnread();
  // Mark the current thread as read
  virtual void markThreadAsRead();
  // Mark the current thread as unread
  virtual void markThreadAsUnread();

  /* Articles */

  // Send the pending articles
  virtual void sendPendingMessages();
  // Delete the current article
  virtual void deleteArticle();
  // Send the current article
  virtual void sendNow();
  // Edit the current article
  virtual void editArticle();
  /// Fetch all the new article headers
  virtual void fetchHeaders();
  /// Expire articles in all groups
  virtual void expireArticles();

  /* Kontact integration */
  /// Process command-line options
  virtual bool handleCommandLine();

  //end dcop interface
signals:
  void signalCaptionChangeRequest( const QString& );

protected:

  KActionCollection* actionCollection() const;
  KXMLGUIFactory *factory() const;

  void initActions();
  void initStatusBar();

  /** checks if run for the first time, sets some global defaults (email configuration) */
  bool firstStart();

  void readOptions();
  void saveOptions();

  bool requestShutdown();

  virtual void showEvent(QShowEvent *);

  /** update appearance */
  virtual void fontChange( const QFont & );
  virtual void paletteChange ( const QPalette & );

  bool eventFilter(QObject *, QEvent *);

  // convenience methods...
  void getSelectedArticles(QPtrList<KNArticle> &l);
  void getSelectedArticles(QPtrList<KNRemoteArticle> &l);
  void getSelectedThreads(QPtrList<KNRemoteArticle> &l);
  void getSelectedArticles(QPtrList<KNLocalArticle> &l);
  void closeCurrentThread();

  //GUI
  KAccel          *a_ccel;
  KNProgress      *p_rogBar;
  KNArticleWidget *a_rtView;
  KNCollectionView *c_olView;
  KNHeaderView      *h_drView;
  KDockWidget     *c_olDock, *h_drDock, *a_rtDock;
  bool b_lockui;
  KToolBar        *q_uicksearch;
  QLineEdit       *s_earchLineEdit;

  //Core
  KNConfigManager   *c_fgManager;
  KNNetAccess       *n_etAccess;
  KNAccountManager  *a_ccManager;
  KNGroupManager    *g_rpManager;
  KNArticleManager  *a_rtManager;
  KNArticleFactory  *a_rtFactory;
  KNFolderManager   *f_olManager;
  KNFilterManager   *f_ilManager;
  KNScoringManager  *s_coreManager;
  KNMemoryManager   *m_emManager;
  Kpgp::Module      *p_gp;

protected slots:
  //listview slots
  void slotArticleSelected(QListViewItem*);
  void slotArticleSelectionChanged();
  void slotCollectionSelected(QListViewItem*);
  void slotCollectionRenamed(QListViewItem*);
  void slotCollectionViewDrop(QDropEvent* e, KNCollectionViewItem* after);
  void slotArticleRMB(KListView*, QListViewItem *i, const QPoint &p);
  void slotCollectionRMB(KListView*, QListViewItem *i, const QPoint &p);
  /** Open selected article in own composer/reader window */
  void slotOpenArticle(QListViewItem *item);
  void slotHdrViewSortingChanged(int i);

  //network slots
  void slotNetworkActive(bool b);

  //dock widget slots
  void slotCheckDockWidgetStatus();
  void slotGroupDockHidden();
  void slotHeaderDockHidden();
  void slotArticleDockHidden();
  void slotDockWidgetFocusChangeRequest(QWidget *w);

  //---------------------------------- <Actions> ----------------------------------

protected:

  //navigation
  KAction   *a_ctNavNextArt,
    *a_ctNavPrevArt,
    *a_ctNavNextUnreadArt,
    *a_ctNavNextUnreadThread,
    *a_ctNavNextGroup,
    *a_ctNavPrevGroup,
    *a_ctNavReadThrough;

  //collection-view - accounts
  KAction   *a_ctAccProperties,
    *a_ctAccRename,
    *a_ctAccSubscribe,
    *a_ctAccExpireAll,
    *a_ctAccGetNewHdrs,
    *a_ctAccGetNewHdrsAll,
    *a_ctAccDelete,
    *a_ctAccPostNewArticle;

  //collection-view - groups
  KAction   *a_ctGrpProperties,
    *a_ctGrpRename,
    *a_ctGrpGetNewHdrs,
    *a_ctGrpExpire,
    *a_ctGrpReorganize,
    *a_ctGrpUnsubscribe,
    *a_ctGrpSetAllRead,
    *a_ctGrpSetAllUnread,
    *a_ctGrpSetUnread;

  //collection-view - folder
  KAction   *a_ctFolNew,
    *a_ctFolNewChild,
    *a_ctFolDelete,
    *a_ctFolRename,
    *a_ctFolCompact,
    *a_ctFolCompactAll,
    *a_ctFolEmpty,
    *a_ctFolMboxImport,
    *a_ctFolMboxExport;

  //header-view - list-handling
  KSelectAction         *a_ctArtSortHeaders;
  KNFilterSelectAction  *a_ctArtFilter;
  KAction               *a_ctArtSortHeadersKeyb,
    *a_ctArtFilterKeyb,
    *a_ctArtSearch,
    *a_ctArtFind,
    *a_ctArtRefreshList,
    *a_ctArtCollapseAll,
    *a_ctArtExpandAll,
    *a_ctArtToggleThread;
  KToggleAction         *a_ctArtToggleShowThreads;

  //header-view - remote articles
  KAction *a_ctArtSetArtRead,
    *a_ctArtSetArtUnread,
    *a_ctArtSetThreadRead,
    *a_ctArtSetThreadUnread,
    *a_ctArtOpenNewWindow;

  // scoring
  KAction *a_ctScoresEdit,
    *a_ctReScore,
    *a_ctScoreLower,
    *a_ctScoreRaise,
    *a_ctArtToggleIgnored,
    *a_ctArtToggleWatched;

  //header-view local articles
  KAction *a_ctArtSendOutbox,
    *a_ctArtDelete,
    *a_ctArtSendNow,
    *a_ctArtEdit;

  //network
  KAction *a_ctNetCancel;

  KAction *a_ctFetchArticleWithID;

  // settings menu
  KToggleAction *a_ctToggleGroupView,
    *a_ctToggleHeaderView,
    *a_ctToggleArticleViewer,
    *a_ctToggleQuickSearch;
  KAction *a_ctSwitchToGroupView,
    *a_ctSwitchToHeaderView,
    *a_ctSwitchToArticleViewer;

protected slots:
  void slotNavNextUnreadArt();
  void slotNavNextUnreadThread();
  void slotNavReadThrough();

  void slotAccProperties();
  void slotAccRename();
  void slotAccSubscribe();
  void slotAccExpireAll();
  void slotAccGetNewHdrs();
  void slotAccGetNewHdrsAll();
  void slotAccDelete();
  void slotAccPostNewArticle();

  void slotGrpProperties();
  void slotGrpRename();
  void slotGrpGetNewHdrs();
  void slotGrpExpire();
  void slotGrpReorganize();
  void slotGrpUnsubscribe();
  void slotGrpSetAllRead();
  void slotGrpSetAllUnread();
  void slotGrpSetUnread();

  void slotFolNew();
  void slotFolNewChild();
  void slotFolDelete();
  void slotFolRename();
  void slotFolCompact();
  void slotFolCompactAll();
  void slotFolEmpty();
  void slotFolMBoxImport();
  void slotFolMBoxExport();

  void slotArtSortHeaders(int i);
  void slotArtSortHeadersKeyb();
  void slotArtSearch();
  void slotArtFind();
  void slotArtRefreshList();
  void slotArtCollapseAll();
  void slotArtExpandAll();
  void slotArtToggleThread();
  void slotArtToggleShowThreads();

  void slotArtSetArtRead();
  void slotArtSetArtUnread();
  void slotArtSetThreadRead();
  void slotArtSetThreadUnread();

  void slotScoreEdit();
  void slotReScore();
  void slotScoreLower();
  void slotScoreRaise();
  void slotArtToggleIgnored();
  void slotArtToggleWatched();

  void slotArtOpenNewWindow();
  void slotArtSendOutbox();
  void slotArtDelete();
  void slotArtSendNow();
  void slotArtEdit();

  void slotNetCancel();

  void slotFetchArticleWithID();

  void slotToggleGroupView();
  void slotToggleHeaderView();
  void slotToggleArticleViewer();
  void slotToggleQuickSearch();
  void slotSwitchToGroupView();
  void slotSwitchToHeaderView();
  void slotSwitchToArticleViewer();
  void slotSettings();

  //--------------------------- </Actions> -----------------------------

private:
  KStatusBarLabel *s_tatusGroup; // widget used in the statusBar() for the group status
  KStatusBarLabel *s_tatusFilter;
  KXMLGUIClient *m_GUIClient;
};


class  FetchArticleIdDlg : public KDialogBase
{
    Q_OBJECT
public:
    FetchArticleIdDlg(QWidget *parent, const char */*name*/ );
    QString messageId() const;

protected slots:
    void slotTextChanged(const QString & );
protected:
    KLineEdit *edit;
};

#endif
