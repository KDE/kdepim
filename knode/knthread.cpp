/***************************************************************************
                          knthread.cpp  -  description
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


#include "knthread.h"

KNThread::KNThread()
{
	hdrs = new QList<KNFetchArticle>;
	hdrs->setAutoDelete(false);
	src=0;
}



KNThread::KNThread(KNGroup *g, KNFetchArticle *a)
{
	hdrs = new QList<KNFetchArticle>;
	hdrs->setAutoDelete(false);
	src=g;
	createThreadOf(a);
}



KNThread::~KNThread()
{
	delete hdrs;
}



void KNThread::createThreadOf(KNFetchArticle *a)
{
	KNFetchArticle *ref, *tmp;
	int topID, idRef=a->idRef();
	
	ref=a;
					
	while(idRef!=0) {
		ref=src->byId(idRef);
		idRef=ref->idRef();
	}
	hdrs->clear();	
	topID=ref->id();
	hdrs->append(ref);
		
	for(int i=0; i<src->length(); i++) {
		tmp=src->at(i);
		if(tmp->idRef()!=0) {
			idRef=tmp->idRef();
		  while(idRef!=0) {
		  	ref=src->byId(idRef);
		  	idRef=ref->idRef();
		  }
			if(ref->id()==topID) hdrs->append(tmp);
		}
	}
}



int KNThread::setRead(bool r, int &newCnt)
{
	KNFetchArticle *ref;
	int idRef, changeCnt=0, n_ew=0;

	for(KNFetchArticle *a=hdrs->first(); a; a=hdrs->next()) {
		
		if(r) {
			a->setUnreadFollowUps(0);
			a->setNewFollowUps(0);
		}
	
		if(a->isRead()!=r) {
		
			changeCnt++;
			a->setRead(r);
			a->setHasChanged(true);
			if(a->isNew()) n_ew++;
					
			
			if(!r) {
				
				idRef=a->idRef();
				
				while(idRef!=0) {
					ref=src->byId(idRef);
					ref->incUnreadFollowUps();
					if(a->isNew()) ref->incNewFollowUps();
					idRef=ref->idRef();
		    }
	  	}       	
	  }
	}
	
	if(changeCnt>0)
		for(KNFetchArticle *a=hdrs->first(); a; a=hdrs->next())
			if(a->listItem()) a->updateListItem();
	
	newCnt=n_ew;		
	return changeCnt;
	 	
}



void KNThread::setScore(short s)
{
	
	if(hdrs->first()->score()!=s)

		for(KNFetchArticle *a=hdrs->first(); a; a=hdrs->next()) {
	   	a->setScore(s);
			if(a->listItem()) a->updateListItem();
			a->setHasChanged(true);
		}

}



void KNThread::toggleWatched()
{
	if(hdrs->first()->score()!=100) setScore(100);
	else setScore(50);
}



void KNThread::toggleIgnored()
{
	if(hdrs->first()->score()!=0) setScore(0);
	else setScore(50);
}










