/***************************************************************************
                          knfoldermanager.h  -  description
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


#ifndef KNFOLDERMANAGER_H
#define KNFOLDERMANAGER_H

#include <qlist.h>
#include <kaction.h>

class KNListView;
class KNPurgeProgressDialog;
class KNFolder;
class KNSavedArticleManager;


class KNFolderManager : public QObject
{
	Q_OBJECT
	
	public:
		enum stFolder { SFdrafts=0, SFoutbox=1, SFsent=2 };
		KNFolderManager(KNSavedArticleManager *a, KNListView *v, QObject * parent=0, const char * name=0);
		~KNFolderManager();
		
		const KActionCollection& actions()    { return actionCollection; }	
		
		int count()														{ return c_ount; }
		KNFolder* currentFolder()							{ return c_urrentFolder; }
  	bool hasCurrentFolder()				      	{ return (c_urrentFolder!=0); }
		void setCurrentFolder(KNFolder *f);
		KNFolder* standardFolder(stFolder stf);
		KNFolder* folder(int i);
		
		void newFolder(KNFolder *p=0);
		void deleteFolder(KNFolder *f=0);
		void removeFolder(KNFolder *f=0);
		void emptyFolder(KNFolder *f=0);
		void compactFolder(KNFolder *f=0);
		bool timeToCompact();
		void compactAll(KNPurgeProgressDialog *dlg=0);
		void syncFolders();		
	
	protected:
		void createStandardFolders();
		int loadCustomFolders();
		void showListItems();
		void createListItem(KNFolder *f);
				
		KNFolder  *c_urrentFolder;
		QList<KNFolder> *fList;
		KNListView *view;
		KNSavedArticleManager *aManager;
		int lastId, c_ount;
		KAction *actCompactFolder, *actEmptyFolder;
		KActionCollection actionCollection;
		
	protected slots:	
	  void slotCompactFolder()              { compactFolder(); }
	  void slotEmptyFolder()		            { emptyFolder(); }
	
};

#endif
