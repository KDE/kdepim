/***************************************************************************
                          kngroupmanager.h  -  description
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


#ifndef KNGROUPMANAGER_H
#define KNGROUPMANAGER_H

#include <qlist.h>

#include <kaction.h>

class KNGroup;
class KNFetchArticleManager;
class KNJobData;
class KNGroupDialog;
class KNPurgeProgressDialog;
class KNNntpAccount;
class KNServerInfo;


class KNGroupManager : public QObject {

	Q_OBJECT
 			
	public:
		KNGroupManager(KNFetchArticleManager *a, QObject * parent=0, const char * name=0);
		~KNGroupManager();
		
		const KActionCollection& actions()    { return actionCollection; }	
		
		void readConfig();		
	 	void loadGroups(KNNntpAccount *a);
	 	void getSubscribed(KNNntpAccount *a, QStrList *l);
	  void getGroupsOfAccount(KNNntpAccount *a, QList<KNGroup> *l);	 	
	 	void showGroupDialog(KNNntpAccount *a);
	  void subscribeGroup(const QCString &gName, KNNntpAccount *a);
		void unsubscribeGroup(KNGroup *g=0);
		void showGroupProperties(KNGroup *g=0);
		void checkGroupForNewHeaders(KNGroup *g=0);
		void expireGroupNow(KNGroup *g=0);
		void resortGroup(KNGroup *g=0);
			
	  void setAutoCheck(bool check)         { a_utoCheck=check;}
		bool autoCheck()                      { return a_utoCheck;}
	 		
		KNGroup* group(const QCString &gName, const KNServerInfo *s);
		KNGroup* currentGroup()               { return c_urrentGroup; }
		bool hasCurrentGroup()                { return (c_urrentGroup!=0); }
		void setCurrentGroup(KNGroup *g);
		
		void checkAll(KNNntpAccount *a);
	  bool timeToExpire();
	  void expireAll(KNPurgeProgressDialog *dlg=0);
	  void syncGroups();		
	  void jobDone(KNJobData *j);			
	
	public slots:
		void slotDialogNewList(KNNntpAccount *a);
		
	protected:
		QList<KNGroup>  *gList;
		KNGroup *c_urrentGroup;
		KNFetchArticleManager *aManager;
		KNGroupDialog *gDialog;				
		int defaultMaxFetch;
		bool a_utoCheck;
		KAction *actProperties, *actLoadHdrs, *actExpire, *actResort, *actUnsubscribe;
		KActionCollection actionCollection;
				
	protected slots:	
	  void slotProperties()                 { showGroupProperties(); }
	  void slotLoadHdrs() 		              { checkGroupForNewHeaders(); }
 	  void slotExpire() 		                { expireGroupNow(); }
 	  void slotResort() 		                { resortGroup(); }
 	  void slotUnsubscribe();	
 	  	
};



#endif
