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

#include <kmainwindow.h>
#include <kprogress.h>

#include "resource.h"

class QSize;
class QListViewItem;

class KAccel;
class KAction;
class KToggleAction;

class KNSettingsDialog;
class KNFetchArticleManager;
class KNAccountManager;
class KNGroupManager;
class KNFetchArticleManager;
class KNFolderManager;
class KNSavedArticleManager;
class KNFilterManager;
class KNAppManager;
class KNNetAccess;
class KNJobData;
class KNodeView;


class KNProgress : public KProgress
{
  Q_OBJECT

  public:
    KNProgress (int desiredHeight, int minValue, int maxValue, int value, KProgress::Orientation orient, QWidget *parent=0, const char *name=0);
    ~KNProgress();

    void disableProgressBar();                                      // 0% and no text
    void setProgressBar(int value,const QString& = QString::null);  // manual operation
    void initProgressBar();                                         // display 0%
    void stepProgressBar();                                         // add 10%
    void fullProgressBar();                                         // display 100%

    virtual QSize sizeHint() const;

  protected:
    int desHeight, progVal;
};


class KNodeApp : public KMainWindow
{
  Q_OBJECT

  public:
          
    KNodeApp();
    ~KNodeApp();
        
    //GUI
    void setStatusMsg(const QString& = QString::null, int id=SB_MAIN);
    void setStatusHelpMsg(const QString& text);
    void setCursorBusy(bool b=true);
    void blockEvents();
    void unblockEvents();

    //network
    void jobDone(KNJobData *j);

  public slots:

    void slotSettings();
      
  protected:

    //init && update
    void initView();
    void initStatusBar();
    void initActions();
    void initPopups();        
    
    void saveOptions();

    // checks if run for the first time, sets some global defaults (email configuration)
    bool firstStart();
      
    //exit
    void cleanup();
    bool queryClose();

    virtual void paletteChange ( const QPalette & );

    //even filter for kapp
    bool eventFilter(QObject*, QEvent *e);

    //actions
    KAction *actCancel, *actSupersede;
    KToggleAction *actShowAllHdrs;

    //popups
    QPopupMenu  *accPopup, *groupPopup, *folderPopup,
                *fetchPopup, *savedPopup;

    KAccel *acc;
    KNodeView *view;
    KNProgress *progBar;
    KNSettingsDialog *setDialog;
    
    KNNetAccess *NAcc;
    KNAccountManager *AManager;
    KNGroupManager  *GManager;
    KNFetchArticleManager *FAManager;
    KNFolderManager *FoManager;
    KNSavedArticleManager *SAManager;
    KNFilterManager *FiManager;
    KNAppManager *AppManager;

  protected slots:

    //action-slots      
    void slotToggleShowAllHdrs();
    void slotCancel();
    void slotSupersede();
    void slotToggleToolBar();
    void slotToggleStatusBar();
    void slotConfKeys();
    void slotConfToolbar();
    void slotSettingsFinished();
        
    //view-slots
    void slotCollectionClicked(QListViewItem *it);
    void slotCollectionSelected(QListViewItem *it);
    void slotHeaderSelected(QListViewItem *it);
    void slotHeaderDoubleClicked(QListViewItem *it);
    void slotArticlePopup(QListViewItem *it, const QPoint &p, int c);
    void slotCollectionPopup(QListViewItem *it, const QPoint &p, int c);

};

#endif // KNODE_H
