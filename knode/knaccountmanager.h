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
#include "kngroupmanager.h"
#include "knnntpaccount.h"

class KNAccNewsSettings;

class KNAccountManager {
	
	public:
		KNAccountManager(KNGroupManager *gm, KNListView *v);
		~KNAccountManager();
		void readConfig();
		void saveYourself();
		void setCurrentAccount(KNNntpAccount *a);
		
		void startConfig(KNAccNewsSettings *s);
		
		void newAccount();
		void applySettings(KNNntpAccount *a);
		void removeAccount(KNNntpAccount *a);
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
		int lastId;
		KNListView *view;		
};

#endif
