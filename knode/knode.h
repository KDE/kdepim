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
    KAccel *accel()												{ return acc; }

    //network
    void jobDone(KNJobData *j);

    	
	protected:

	  //init && update
		void initMenuBar();
 	  void initPopups();
 	  void initToolBar();
 	  void initStatusBar();
 	  void initView();
 	  void initAccel();
 	   	        	
    void ViewToolBar();
    void ViewStatusBar();
  	void setMenuAccels();

  	void updateMenus(int *idArr, bool e);
  	void updateAccels(const char **idArr, bool e);
  	
  	void saveOptions();
  	void readOptions();
  	
  	void showSettings();  	  	

  	
  	//exit
    void cleanup();
    bool queryExit();

    //menuBar
    QPopupMenu 	*file_menu,
    						*groups_menu, *groups_menu_folders,
    						*message_menu, *message_menu_thread,
     						*message_menu_mark,
    						*goto_menu,
    						*view_menu, *view_menu_sort,
    						*help_menu;

   	QPopupMenu *test_menu; //DEBUG
   	
   	//popups
   	QPopupMenu 	*accPopup, *groupPopup, *folderPopup,
   							*fetchPopup, *savedPopup;
   	
   	KAccel *acc;
   	
   	KToolBar *tb0;
   	KNodeView *view;
    KNProgress *progBar;
   	int progr;
   	
    bool bViewToolbar;
  	bool bViewStatusbar;
//    KMenuBar::menuPosition menu_bar_pos;
//    KToolBar::BarPosition tool_bar_pos;  	
		
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
  	//callback
  	void slotMainCallback(int id_);
  	  	
  	//menus  	  	
  	void slotFileSaveAs();
  	void slotFileQuit();
  	void slotFilePrint();
  	  	
  	void slotArtNew();
  	void slotArtReply();
  	void slotArtRemail();
  	void slotArtForward();
  	void slotArtOwnWindow();
  	void slotArtMarkRead();
  	void slotArtMarkUnread();
  	void slotArtThrRead();
  	void slotArtThrUnread();
  	void slotArtThrScore();
  	void slotArtThrWatch();
  	void slotArtThrIgnore();
  	void slotArtThrToggle();
  	void slotArtEdit();
  	void slotArtDelete();
  	void slotArtSearch();
  	  	
  	void slotGotoNextArt();
  	void slotGotoPrevArt();  	
  	void slotGotoNextUnreadArt();
  	void slotReadThrough();
  	void slotGotoNextThr();
  	void slotGotoNextGroup();
  	void slotGotoPrevGroup();
  	
  	void slotViewSort(int id);
  	void slotViewRefresh();
  	void slotViewZoom();
  	  	
  	//view-slots  	
	 	void slotCollectionSelected(QListViewItem *it);
  	void slotHeaderSelected(QListViewItem *it);
  	void slotHeaderDoubleClicked(QListViewItem *it);
  	void slotSortingChanged(int oldCol, int newCol);
  	void slotArticlePopup(QListViewItem *it, const QPoint &p, int c);
  	void slotCollectionPopup(QListViewItem *it, const QPoint &p, int c);
};

#endif // KNODE_H
