/*
    knodeview.h

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

#ifndef KNODEVIEW_H
#define KNODEVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qsplitter.h>
#include <kaction.h>
#include <qlistview.h>

class KNArticleWidget;
class KNListView;
class KNFocusWidget;
class KNArticle;
class KNNntpAccount;
class KNGroup;
class KNFolder;
class KNConfigManager;
class KNAccountManager;
class KNGroupManager;
class KNFolderManager;
class KNArticleManager;
class KNArticleFactory;
class KNFilterManager;
class KNFilterSelectAction;
class KNNetAccess;
class KNRemoteArticle;


class KNodeView : public QSplitter
{
  Q_OBJECT

  friend class KNMainWindow;
  
  public:
      
    KNodeView(KNMainWindow *w, const char * name=0);
    ~KNodeView();

    void readOptions();
    void saveOptions();

    bool requestShutdown();
    void prepareShutdown();

    void blockUI(bool b=true)   { b_lockui=b; }
    void configChanged();   // switch between long & short group list, update fonts and colors

    //access to GUI-elements
    KNListView*       collectionView()  { return c_olView; }
    KNListView*       headerView()      { return h_drView; }
    KNArticleWidget*  articleView()     { return a_rtView; }

  protected:
    void initActions();
    void initPopups(KNMainWindow *w);

    virtual void paletteChange ( const QPalette & );
    virtual void fontChange ( const QFont & );

    void getSelectedArticles(QList<KNRemoteArticle> &l);
    void getSelectedThreads(QList<KNRemoteArticle> &l);

    //GUI
    QSplitter       *s_ecSplitter;
    KNArticleWidget *a_rtView;
    KNListView      *c_olView, *h_drView;
    KNFocusWidget   *c_olFocus, *h_drFocus, *a_rtFocus;
    bool l_ongView, b_lockui;

    //Popups
    QPopupMenu  *a_ccPopup,
                *g_roupPopup,
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


  protected slots:
    //listview slots
    void slotArticleSelected(QListViewItem*);
    void slotArticleDoubleClicked(QListViewItem*);
    void slotArticleSelectionChanged();
    void slotCollectionSelected(QListViewItem*);
    void slotArticleRMB(QListViewItem *i, const QPoint &p, int);
    void slotCollectionRMB(QListViewItem *i, const QPoint &p, int);
    void slotHdrViewSortingChanged(int i);
    
    //network slots
    void slotNetworkActive(bool b);
    
    
  //--------------------------- <Actions> -----------------------------
    
  protected:

    KActionCollection *a_ctions;

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
              *a_ctAccSubscribe,
              *a_ctAccGetNewHdrs,
              *a_ctAccDelete,
              *a_ctAccPostNewArticle;

    //collection-view - groups
    KAction   *a_ctGrpProperties,
              *a_ctGrpGetNewHdrs,
              *a_ctGrpExpire,
              *a_ctGrpReorganize,
              *a_ctGrpUnsubscribe,
              *a_ctGrpSetAllRead,
              *a_ctGrpSetAllUnread;
  
    //collection-view - folder
    KAction   *a_ctFolCompact,
              *a_ctFolEmpty,
              *a_ctFolProperties;
        
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
            *a_ctSetArtScore,
            *a_ctArtSetThreadScore,
            *a_ctArtToggleIgnored,
            *a_ctArtToggleWatched,
            *a_ctArtOpenNewWindow;
                            
    //header-view local articles
    KAction *a_ctArtSendOutbox,
            *a_ctArtDelete,
            *a_ctArtSendNow,
            *a_ctArtEdit;

    //network
    KAction *a_ctNetCancel;

    KAction *a_ctFetchArticleWithID;

  protected slots:
    void slotNavNextArt();
    void slotNavPrevArt();
    void slotNavNextUnreadArt();
    void slotNavNextUnreadThread();
    void slotNavNextGroup();
    void slotNavPrevGroup();
    void slotNavReadThrough();

    void slotAccProperties();
    void slotAccSubscribe();
    void slotAccGetNewHdrs();
    void slotAccDelete();
    void slotAccPostNewArticle();

    void slotGrpProperties();
    void slotGrpGetNewHdrs();
    void slotGrpExpire();
    void slotGrpReorganize();
    void slotGrpUnsubscribe();
    void slotGrpSetAllRead();
    void slotGrpSetAllUnread();
    
    void slotFolCompact();
    void slotFolEmpty();
    void slotFolProperties();

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
    void slotArtSetArtScore();
    void slotArtSetThreadScore();
    void slotArtToggleIgnored();
    void slotArtToggleWatched();
    void slotArtOpenNewWindow();
    
    void slotArtSendOutbox();
    void slotArtDelete();
    void slotArtSendNow();
    void slotArtEdit();

    void slotNetCancel();

    void slotFetchArticleWithID();

  //--------------------------- </Actions> -----------------------------      

};

#endif // KNODEVIEW_H

