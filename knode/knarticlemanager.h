/***************************************************************************
                          knarticlemanager.h  -  description
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


#ifndef KNARTICLEMANAGER_H
#define KNARTICLEMANAGER_H

#include <qlist.h>

class QFile;
class QString;

class KURL;
class KTempFile;

class KNListView;
class KNArticleWidget;
class KNMimeContent;
class KNArticle;


//===============================================================================


// handles file saving for KNArticleManager => no duplicated code
class KNSaveHelper {

public:
	
	KNSaveHelper(QString saveName);
	~KNSaveHelper();
	
	// returns a file open for writing
	QFile* getFile();
	
private:

  QString s_aveName;
  KURL url;
  QFile* file;
  KTempFile* tmpFile;	

};


//===============================================================================


class KNArticleManager {
	
	public:
		KNArticleManager(KNListView *v);
		virtual ~KNArticleManager();
		static void deleteTempFiles();
		
		static void saveContentToFile(KNMimeContent *c);
		static void saveArticleToFile(KNArticle *a);
		static QString saveContentToTemp(KNMimeContent *c);
		static void openContent(KNMimeContent *c);
		static void showArticle(KNArticle *a, bool force=false);
		static void showError(KNArticle *a, const QString &error);
		
	protected:	
		KNListView *view;
		KNArticleWidget *mainArtWidget;
		static QList<KTempFile> tempFiles;	
			
};

#endif
