/***************************************************************************
                          knsavedarticlemanager.h  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
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


#ifndef KNSAVEDARTICLEMANAGER_H
#define KNSAVEDARTICLEMANAGER_H

#include <qobject.h>
#include <qlist.h>

#include <kaction.h>

#include "knarticlemanager.h"
#include "knarticlebase.h"

class KNAccountManager;
class KNNntpAccount;
class KNGroup;
class KNJobData;
class KNComposer;
class KNFolder;
class KNFetchArticle;
class KNSavedArticle;
class KNSearchDialog;
class KNSendErrorDialog;
class KNUserEntry;


class KNSavedArticleManager : public QObject, public KNArticleManager  {
  
  Q_OBJECT  

  public:
    KNSavedArticleManager(KNListView *v, KNAccountManager *am, QObject * parent=0, const char * name=0);
    ~KNSavedArticleManager();
    
    const KActionCollection& actions()    { return actionCollection; }    
    
    void readConfig();
    void setStandardFolders(KNFolder *d, KNFolder *o, KNFolder *s);
    void setFolder(KNFolder *f);
    void showHdrs();
    //void search();
    
    bool hasCurrentArticle() const  { return (c_urrentArticle!=0); }
    KNSavedArticle* currentArticle() const  { return c_urrentArticle; }
    
    void setCurrentArticle(KNSavedArticle *a);
    void post(KNNntpAccount *acc);
    void post(KNGroup *g);
    void reply(KNArticle *a, KNGroup *g);
    void forward(KNArticle *a);
    void editArticle(KNSavedArticle *a=0);
    void saveArticle(KNSavedArticle *a);
    bool deleteArticle(KNSavedArticle *a=0, bool ask=false);
    void sendArticle(KNSavedArticle *a=0, bool now=true);
    void sendOutbox();
    void cancel(KNSavedArticle *a=0);
    void cancel(KNFetchArticle *a, KNGroup *g);   
    void supersede(KNSavedArticle *a=0);
    void supersede(KNFetchArticle *a, KNGroup *g);    
    
    void jobDone(KNJobData *job);
    
    void mailToClicked(KNArticleWidget *aw);        
    
    bool closeComposeWindows();    // try to close all composers, return false if user objects
    
  protected:
    KNSavedArticle* newArticle(KNNntpAccount *acc=0);
    KNNntpAccount* getAccount(KNSavedArticle *a);
    void openInComposer(KNSavedArticle *a);
    bool getComposerData(KNComposer *c);
    void showArticle(KNArticle *a, bool force=false);
    void showError(KNArticle *a, const QString &error);
    void updateStatusString();
    bool cancelAllowed(KNSavedArticle *a);
    bool cancelAllowed(KNFetchArticle *a, KNGroup *g);
    bool generateCancel(KNArticle *a, KNNntpAccount *acc);
    bool generateSupersede(KNArticle *a, KNNntpAccount *acc);
        
    KNSavedArticle *c_urrentArticle;
    KNFolder *f_older, *fDrafts, *fOutbox, *fSent;
    KNUserEntry *defaultUser;
    bool incSig, genMId;
    QCString MIdhost, intro, quotSign;
    KNSendErrorDialog *sedlg;
    KNSearchDialog *sDlg;
    //KNArticleFilter *f_ilter;
    KNAccountManager *accM;
    QList<KNComposer> *comList;
    KAction *actSendOutbox, *actEdit, *actDelete, *actSendNow, *actSendLater;
    KActionCollection actionCollection;   
        
  protected slots:
    void slotComposerDone(KNComposer *com);
    void slotSendErrorDialogDone();
    //void slotSearchDialogDone();
    //void slotDoSearch(KNArticleFilter *f);
    void slotSendOutbox()        { sendOutbox(); }
    void slotEdit()              { editArticle(); }
    void slotDelete()            { deleteArticle(0, true); }
    void slotCancel()            { cancel(); }
    void slotSendNow()           { sendArticle(); }
    void slotSendLater()         { sendArticle(0, false); }
      
};

#endif
