/***************************************************************************
                          knaccountmanager.h  -  description
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


#ifndef KNACCOUNTMANAGER_H
#define KNACCOUNTMANAGER_H

#include <qlist.h>

#include <kaction.h>

class KNAccNewsSettings;
class KNAccNewsConfDialog;
class KNGroupManager;
class KNListView;
class KNNntpAccount;
class KNServerInfo;

class KNAccountManager : public QObject
{
	Q_OBJECT
	
	public:
		KNAccountManager(KNGroupManager *gm, KNListView *v, QObject * parent=0, const char * name=0);
		~KNAccountManager();
		
		const KActionCollection& actions()      { return actionCollection; }
			
		void readConfig();
		void saveYourself();
		void setCurrentAccount(KNNntpAccount *a);
		
		void startConfig(KNAccNewsSettings *s);
		
		void newAccount(KNAccNewsConfDialog *dlg);
		void applySettings(KNNntpAccount *a, KNAccNewsConfDialog *dlg);
		void removeAccount(KNNntpAccount *a=0);
		void endConfig();
		
		bool hasCurrentAccount()							{ return (c_urrentAccount!=0); }
		KNNntpAccount* currentAccount()				{ return c_urrentAccount; }
		KNServerInfo*	smtp()									{ return s_mtp; }
		KNNntpAccount* first()								{ return accList->first(); }
		KNNntpAccount* next()									{ return accList->next(); }
		KNNntpAccount* account(int i);	
				
	protected:
		void loadAccounts();
		KNGroupManager *gManager;
		QList<KNNntpAccount> *accList;
		KNNntpAccount *c_urrentAccount;
		KNServerInfo *s_mtp;
		
		KNAccNewsSettings *set;
		KNListView *view;		
		KAction *actProperties, *actSubscribe, *actLoadHdrs, *actDelete, *actPostNewArticle;
		KActionCollection actionCollection;
				
	protected slots:	
	  void slotProperties();
	  void slotSubscribe();
	  void slotLoadHdrs();
	  void slotDelete();
	  void slotPostNewArticle();
			
};

#endif
