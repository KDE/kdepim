/***************************************************************************
                     knode.h - description
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

#ifndef KNODE_H
#define KNODE_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for QT
#include <qprinter.h>
#include <qpainter.h>
#include <qdir.h>

// include files for KDE
#include <klocale.h>
#include <ktmainwindow.h>
#include <kaccel.h>
#include <kiconloader.h>
#include <qprogressbar.h>
#include <kprogress.h>

#include "resource.h"
#include "knodeview.h"
#include "knnetaccess.h"
#include "knaccountmanager.h"
#include "knfoldermanager.h"
#include "knfiltermanager.h"


class KNProgress : public KProgress    // Ok, this is just a hack to adjust the sizeHint of the progress bar
{
  Q_OBJECT

  public:
    KNProgress (int desiredHeight, int minValue, int maxValue, int value, KProgress::Orientation orient, QWidget *parent=0, const char *name=0);
    ~KNProgress();

    virtual QSize sizeHint() const;

  private:
    int desHeight;
};


class KNodeApp : public KTMainWindow
{
  Q_OBJECT

	public:
  				
    KNodeApp();
    ~KNodeApp();
  	  	
    //GUI
  	void setStatusMsg(const QString& = QString::null, int id=SB_MAIN);
    void setStatusHelpMsg(const QString& text);
    void setCursorBusy(bool b=true);

    void netIsActive(bool b);

    void disableProgressBar();		// 0% and no text
    void setProgressBar(int value,const QString& = QString::null);  // manual operation
    void initProgressBar();       // display 0%
    void stepProgressBar();       // add 10%
    void fullProgressBar();       // display 100%

    void accountSelected(bool b);
    void groupSelected(bool b);
    void groupDisplayed(bool b);
    void fetchArticleSelected(bool b);
    void fetchArticleDisplayed(bool b);
    void folderSelected(bool b);
    void folderDisplayed(bool b);
    void savedArticleSelected(bool b);
    void savedArticleDisplayed(bool b);

    //Member-Access
    KNAccountManager* accManager()				{ return AManager; }
    KNGroupManager* gManager()						{ return GManager; }
    KNFetchArticleManager* fArtManager()	{ return FAManager; }
    KNFolderManager* foManager()					{ return FoManager; }
    KNSavedArticleManager* sArtManager()	{ return SAManager; }
    KNFilterManager* fiManager() 					{ return FiManager; }

    //network
    void jobDone(KNJobData *j);

    	
	protected:

	  //init && update
	  void initView();
	  void initStatusBar();
	  void initActions();
 	  void initPopups();      	
  	
  	void saveOptions();
  	void readOptions();
  	 	
  	//exit
    void cleanup();
    bool queryExit();

	  //actions
    KAction  *actFileSave, *actFilePrint, *actNetSendPending, *actNetStop, *actEditCopy, *actEditFind,
             *actViewExpandAll,*actViewCollapseAll, *actViewRefresh,
             *actAccProperties, *actAccSubscribeGrps, *actAccLoadHdrs, *actAccDelete,
             *actGrpProperties, *actGrpLoadHdrs, *actGrpExpire, *actGrpResort, *actGrpAllRead,
             *actGrpAllUnread, *actGrpUnsubscribe, *actFolderCompact, *actFolderEmpty,
             *actArtPostNew, *actArtPostReply, *actArtMailReply, *actArtForward,
             *actArtRead, *actArtUnread, *actArtOwnWindow, *actArtEdit, *actArtDelete,
             *actArtCancel, *actArtSendNow, *actArtSendLater, *actArtSearch,
             *actThreadRead, *actThreadUnread, *actThreadSetScore, *actThreadWatch,
             *actThreadIgnore, *actThreadToggle;

    KSelectAction *actViewSort;
    KNFilterSelectAction *actViewFilters;
    KToggleAction *actViewShowThreads, *actViewShowAllHdrs;

   	//popups
   	QPopupMenu 	*accPopup, *groupPopup, *folderPopup,
   							*fetchPopup, *savedPopup;

   	KAccel *acc;
   	KNodeView *view;
    KNProgress *progBar;
   	int progr;
		
    KNNetAccess	*NAcc;
    KNAccountManager *AManager;
    KNGroupManager	*GManager;
    KNFetchArticleManager *FAManager;
    KNFolderManager *FoManager;
    KNSavedArticleManager *SAManager;
    KNFilterManager *FiManager;

	public slots:
  	void slotSaveYourself()				{ cleanup(); }

  protected slots:

  	//action-slots	  	
  	void slotFileSave();                // file menu
  	void slotNetSendPending();
  	void slotNetStop();
  	void slotFileQuit();
  	
  	void slotToggleShowThreads();       // view menu
  	void slotToggleShowAllHdrs();
  	void slotViewSort(int id);
  	void slotViewRefresh();
  	void slotViewExpand();
  	void slotViewCollapse();
  	
  	void slotGotoNextArt();              // go menu
  	void slotGotoPrevArt();  	
  	void slotGotoNextUnreadArt();
  	void slotReadThrough();
  	void slotGotoNextThr();
  	void slotGotoNextGroup();
  	void slotGotoPrevGroup();
 	  	
  	void slotAccProperties();           // account menu
  	void slotAccSubscribeGrps();
  	void slotAccLoadHdrs();
  	void slotAccDelete();
  	
  	void slotGrpProperties();           // group menu
  	void slotGrpLoadHdrs();
  	void slotGrpExpire();
  	void slotGrpResort();
   	void slotGrpAllRead();
  	void slotGrpAllUnread();  	
   	void slotGrpUnsubscribe();
  	void slotFolderCompact();
  	void slotFolderEmpty();
  	  	
  	void slotArtNew();                  // article menu
  	void slotArtReply();
  	void slotArtRemail();
  	void slotArtForward();
  	void slotArtOwnWindow();
  	void slotArtMarkRead();
  	void slotArtMarkUnread();
  	void slotArtEdit();
  	void slotArtDelete();
  	void slotArtCancel();
  	void slotArtSendNow();
  	void slotArtSendLater();
  	void slotArtSearch();
  	void slotArtThrRead();
  	void slotArtThrUnread();
  	void slotArtThrScore();
  	void slotArtThrWatch();
  	void slotArtThrIgnore();
  	void slotArtThrToggle();
  	  		
  	void slotToggleToolBar();            // settings menu
  	void slotToggleStatusBar();
  	void slotConfKeys();
  	void slotConfToolbar();
  	void slotSettings();  	  	
  	  	
  	//view-slots  	
	 	void slotCollectionSelected(QListViewItem *it);
  	void slotHeaderSelected(QListViewItem *it);
  	void slotHeaderDoubleClicked(QListViewItem *it);
  	void slotSortingChanged(int newCol);
  	void slotArticlePopup(QListViewItem *it, const QPoint &p, int c);
  	void slotCollectionPopup(QListViewItem *it, const QPoint &p, int c);
		void slotSelectionChanged();
};

#endif // KNODE_H
