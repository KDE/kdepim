/*
    knode.h

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

#ifndef KNODE_H
#define KNODE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kdockwidget.h>

#include "resource.h"

class QListViewItem;

class KAccel;
class KAction;
class KToggleAction;
class KSelectAction;

class KNListView;
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
class KNpgp;
class KNArticleWidget;
class KNArticle;
class KNLocalArticle;
class KNRemoteArticle;


class KNMainWindow : public KDockMainWindow
{
  Q_OBJECT

  public:
          
    KNMainWindow();
    ~KNMainWindow();

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
    KNListView*       collectionView()  { return c_olView; }
    KNListView*       headerView()      { return h_drView; }
    KNArticleWidget*  articleView()     { return a_rtView; }

  protected:

    void initActions();
    void initPopups();

    /** checks if run for the first time, sets some global defaults (email configuration) */
    bool firstStart();

    void readOptions();
    void saveOptions();

    bool requestShutdown();
    void prepareShutdown();

    /** exit */
    bool queryClose();

    virtual void showEvent(QShowEvent *);

    /** update appearance */
    virtual void fontChange( const QFont & );
    virtual void paletteChange ( const QPalette & );

    bool eventFilter(QObject *, QEvent *);

    // convenience methods...
    void getSelectedArticles(QList<KNArticle> &l);
    void getSelectedArticles(QList<KNRemoteArticle> &l);
    void getSelectedThreads(QList<KNRemoteArticle> &l);
    void getSelectedArticles(QList<KNLocalArticle> &l);
    void closeCurrentThread();

    //GUI
    KAccel          *a_ccel;
    KNProgress      *p_rogBar;
    KNArticleWidget *a_rtView;
    KNListView      *c_olView, *h_drView;
    KDockWidget     *c_olDock, *h_drDock, *a_rtDock;
    bool b_lockui;

    //Popups
    QPopupMenu  *a_ccPopup,
                *g_roupPopup,
                *r_ootFolderPopup,
                *f_olderPopup,
                *r_emotePopup,
                *l_ocalPopup;

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
    KNpgp             *p_gp;

  protected slots:
    //listview slots
    void slotArticleSelected(QListViewItem*);
    void slotArticleSelectionChanged();
    void slotCollectionSelected(QListViewItem*);
    void slotCollectionRenamed(QListViewItem*);
    void slotCollectionViewDrop(QDropEvent* e, QListViewItem* after);
    void slotArticleRMB(QListViewItem *i, const QPoint &p, int);
    void slotCollectionRMB(QListViewItem *i, const QPoint &p, int);
    void slotArticleMMB(QListViewItem *item);
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
              *a_ctGrpSetAllUnread;

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
    KToggleAction *a_ctWinToggleToolbar,
                  *a_ctWinToggleStatusbar,
                  *a_ctToggleGroupView,
                  *a_ctToggleHeaderView,
                  *a_ctToggleArticleViewer;
    KAction *a_ctSwitchToGroupView,
            *a_ctSwitchToHeaderView,
            *a_ctSwitchToArticleViewer;

  protected slots:
    void slotNavNextArt();
    void slotNavPrevArt();
    void slotNavNextUnreadArt();
    void slotNavNextUnreadThread();
    void slotNavNextGroup();
    void slotNavPrevGroup();
    void slotNavReadThrough();

    void slotAccProperties();
    void slotAccRename();
    void slotAccSubscribe();
    void slotAccExpireAll();
    void slotAccGetNewHdrs();
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

    void slotWinToggleToolbar();
    void slotWinToggleStatusbar();
    void slotToggleGroupView();
    void slotToggleHeaderView();
    void slotToggleArticleViewer();
    void slotSwitchToGroupView();
    void slotSwitchToHeaderView();
    void slotSwitchToArticleViewer();
    void slotConfKeys();
    void slotConfToolbar();
    void slotSettings();

  //--------------------------- </Actions> -----------------------------

};

#endif // KNODE_H
