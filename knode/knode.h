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
#include <kiconloader.h>
#include <qprogressbar.h>
#include <kprogress.h>

#include "resource.h"
#include "knodeview.h"
#include "knnetaccess.h"
#include "knaccountmanager.h"
#include "knfoldermanager.h"
#include "knfiltermanager.h"


class KNProgress : public KProgress
{
  Q_OBJECT

  public:
    KNProgress (int desiredHeight, int minValue, int maxValue, int value, KProgress::Orientation orient, QWidget *parent=0, const char *name=0);
    ~KNProgress();

    void disableProgressBar();		                                  // 0% and no text
    void setProgressBar(int value,const QString& = QString::null);  // manual operation
    void initProgressBar();                                         // display 0%
    void stepProgressBar();                                         // add 10%
    void fullProgressBar();                                         // display 100%

    virtual QSize sizeHint() const;

  protected:
    int desHeight, progVal;
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
  	 	
  	//exit
    void cleanup();
    bool queryExit();

	  //actions
	  KAction *actCancel, *actSupersede;       // located here, because KNSaved- *and* KNReadArticleManager provide cancel/supersede
    KToggleAction *actShowAllHdrs;

   	//popups
   	QPopupMenu 	*accPopup, *groupPopup, *folderPopup,
   							*fetchPopup, *savedPopup;

   	KAccel *acc;
   	KNodeView *view;
    KNProgress *progBar;
		
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
  	void slotFileQuit();
  	void slotToggleShowAllHdrs();
  	void slotCancel();
  	void slotSupersede();
   	void slotToggleToolBar();
  	void slotToggleStatusBar();
  	void slotConfKeys();
  	void slotConfToolbar();
  	void slotSettings();  	
  	  	
  	//view-slots  	
	 	void slotCollectionSelected(QListViewItem *it);
  	void slotHeaderSelected(QListViewItem *it);
  	void slotHeaderDoubleClicked(QListViewItem *it);
  	void slotArticlePopup(QListViewItem *it, const QPoint &p, int c);
  	void slotCollectionPopup(QListViewItem *it, const QPoint &p, int c);

};

#endif // KNODE_H
