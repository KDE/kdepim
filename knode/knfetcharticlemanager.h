/***************************************************************************
                          knfetcharticlemanager.h  -  description
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


#ifndef KNFETCHARTICLEMANAGER_H
#define KNFETCHARTICLEMANAGER_H

#include <kaction.h>
#include "knarticlemanager.h"

class QTimer;

class KNArticleFilter;
class KNFetchArticle;
class KNSearchDialog;
class KNJobData;
class KNGroup;
class KNFilterManager;


class KNFetchArticleManager : public QObject, public KNArticleManager  {
	
	Q_OBJECT
	
	public:
		KNFetchArticleManager(KNListView *v, KNFilterManager* fiManager, QObject * parent=0, const char * name=0);
		~KNFetchArticleManager();
		
		const KActionCollection& actions()      { return actionCollection; }
		
		void readConfig();
				
		void setGroup(KNGroup *g);					
		KNGroup* group() const  { return g_roup; }
		KNArticleFilter* filter() const  { return f_ilter; }
		bool hasCurrentArticle() const  { return (c_urrent!=0); }
		KNFetchArticle* currentArticle() const  { return c_urrent; }
		
		void showHdrs(bool clear=true);
	  void expandAllThreads(bool e);
	  void setThreaded(bool t)				{ t_hreaded=t; }
	  void toggleThreaded()						{ t_hreaded=!t_hreaded; showHdrs(); }
		bool threaded()	const { return t_hreaded; }
				
		void setCurrentArticle(KNFetchArticle *a);
		void articleWindow(KNFetchArticle *a=0);
		void setArticleRead(KNFetchArticle *a=0, bool r=true, bool ugli=true);
		void setThreadRead(KNFetchArticle *a=0, bool r=true);
		void setAllRead(KNGroup *g=0, bool r=true);
		void toggleWatched(KNFetchArticle *a=0);
		void toggleIgnored(KNFetchArticle *a=0);
		void setArticleScore(KNFetchArticle *a=0);
		void setThreadScore(KNFetchArticle *a=0, int score=-1);
		
		void search();
			
		void jobDone(KNJobData *j);
		
		void referenceClicked(int refNr, KNArticleWidget *aw,int btn);
		
	protected:
		void showArticle(KNArticle *a);
		void showCancel(KNArticle *a);
		void showError(KNArticle *a, const QString &error);
		void createHdrItem(KNFetchArticle *a);
		void createThread(KNFetchArticle *a);
  	void updateStatusString();
			
		KNGroup* g_roup;
		KNFetchArticle *c_urrent, *n_ext;
		KNArticleFilter *f_ilter;
		QTimer *timer;
		int tOut;
		bool t_hreaded, autoMark;
		KNSearchDialog *sDlg;
    KAction *actExpandAll, *actCollapseAll, *actRefresh,
            *actAllRead, *actAllUnread, *actPostReply, *actMailReply, *actForward,
            *actMarkRead, *actMarkUnread, *actOwnWindow,  *actSearch, *actThreadRead,
            *actThreadUnread, *actThreadSetScore, *actThreadWatch,*actThreadIgnore;
    KToggleAction *actShowThreads;
		KActionCollection actionCollection;
					
	public slots:
		void slotFilterChanged(KNArticleFilter *f);
		void slotSearchDialogDone();
					
	protected slots:
		void slotTimer();
		void slotToggleShowThreads()  { toggleThreaded(); }
		void slotThreadsExpand()      { expandAllThreads(true); }
		void slotThreadsCollapse()    { expandAllThreads(false); }
		void slotRefresh()            { showHdrs(); }
		void slotSearch()             { search(); }
    void slotThreadRead()         { setThreadRead(0, true); }
    void slotThreadUnread()       { setThreadRead(0, false); }
    void slotThreadScore()        { setThreadScore(); }
    void slotThreadWatch()        { toggleWatched(); }
    void slotThreadIgnore()       { toggleIgnored(); }
    void slotAllRead()            { setAllRead(0, true); }
    void slotAllUnread()          {	setAllRead(0, false); }
    void slotReply();
    void slotRemail();
    void slotForward();
    void slotOwnWindow()          { articleWindow(); }
    void slotMarkRead()           { setArticleRead(0, true); }
    void slotMarkUnread()         { setArticleRead(0, false); }
		
};

#endif
