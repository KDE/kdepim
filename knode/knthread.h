/***************************************************************************
                          knthread.h  -  description
                             -------------------
    
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


#ifndef KNTHREAD_H
#define KNTHREAD_H

#include <qlist.h>

class KNGroup;
class KNFetchArticle;

class KNThread {
	
	public:
		KNThread();
		KNThread(KNGroup *g, KNFetchArticle *a);
		~KNThread();
	
		void setSource(KNGroup *g)						{ src=g; }
		void createThreadOf(KNFetchArticle *a);
		
		int count()														{ return hdrs->count(); }
		
		KNFetchArticle* rootArticle()					{ return hdrs->first(); }
		void add(KNFetchArticle *a)						{ hdrs->append(a); }
		bool remove(KNFetchArticle *a)				{ return hdrs->remove(a); }
		
		int setRead(bool r, int &newCnt);
		void setScore(short s);
		void toggleWatched();
		void toggleIgnored();
		//void kill() {}
				
  protected:
  	QList<KNFetchArticle> *hdrs;
  	KNGroup *src;
  	
};

#endif




