/***************************************************************************
                          kncleanup.cpp  -  description
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

#include <qdatetime.h>

#include <kmessagebox.h>
#include <kglobal.h>

#include "kncleanup.h"
#include "knglobals.h"

KNCleanUp::KNCleanUp()
{
	delCnt=0;
	leftCnt=0;
	KConfig *c=KGlobal::config();
	c->setGroup("EXPIRE");
	rDays=c->readNumEntry("readDays", 10);
	uDays=c->readNumEntry("unreadDays", 15);
	saveThr=c->readBoolEntry("saveThreads", true);
}



KNCleanUp::~KNCleanUp()
{
}



void KNCleanUp::group(KNGroup *g, bool withGUI)
{
	int expDays, idRef, foundId;
	int age;
	QDate today=QDate::currentDate();
	QDateTime fetchDate;
		
	KNFetchArticle *art, *ref;
	delCnt=0;	
 			
	if(!g->loadHdrs()) return;
	
	//find all expired
	for(int i=0; i<g->length(); i++) {
		art=g->at(i);			
		if(art->isRead()) expDays=rDays;
		else expDays=uDays;
		fetchDate.setTime_t(art->timeT());
		age=fetchDate.date().daysTo(today);
		art->setExpired((age >= expDays));
	}
	
	//save threads
	if(saveThr) {
		for(int i=0; i<g->length(); i++) {
			art=g->at(i);
			if(!art->isExpired()) {
				idRef=art->idRef();
				while(idRef!=0) {
					ref=g->byId(idRef);
					ref->setExpired(false);
					idRef=ref->idRef();
				}
			}
		}
	}				
	
	//restore threading		
	for(int i=0; i<g->length(); i++) {
		art=g->at(i);	
		if(!art->isExpired()) {
			idRef=art->idRef();
			foundId=0;
			while(foundId==0 && idRef!=0) {
				ref=g->byId(idRef);
				if(!ref->isExpired()) foundId=ref->id();
				idRef=ref->idRef();
			}
			art->setIdRef(foundId);
		}
	}
	for(int i=0; i<g->length(); i++) {
		art=g->at(i);
		if(art->isExpired()) {
			art->clear();
			if(art->isRead()) g->decReadCount();
			delCnt++;
		}
	}
		
	if(delCnt>0) {
		g->saveStaticData(g->length(), true);
		g->saveDynamicData(g->length(), true);
		g->decCount(delCnt);
		g->setNewCount(0);
		g->clearList();
	}
	else g->syncDynamicData();
	g->saveInfo();
	leftCnt=g->count();
	
	if(withGUI) {
		QCString tmp;
		tmp.sprintf("%s\nexpired:\t%d\nleft:\t%d", g->groupname().data(), delCnt, leftCnt);
		KMessageBox::information(0,tmp);
	}	
}



void KNCleanUp::folder(KNFolder *f)
{
  KNSavedArticle *art;
	QTextStream ts;
  if(!f->loadHdrs()) return;
		
	QString dir(f->path());
	if (dir == QString::null)
		return;	
	
  QString oldName(dir+QString("folder%1.mbox").arg(f->id()));
  KNFile oldFile(oldName);
	QString newName(dir+QString("folder%1.mbox.new").arg(f->id()));
	KNFile newFile(newName);	

  if( (oldFile.open(IO_ReadOnly)) && (newFile.open(IO_WriteOnly)) ) {
  	for(int idx=0; idx<f->length(); idx++) {
  		art=f->at(idx);
  		if(oldFile.at(art->startOffset())) {
  			ts.setDevice(&newFile);
  			ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";
  			art->setStartOffset(newFile.at());
  			while(oldFile.at() < art->endOffset())
  			  ts << oldFile.readLineWnewLine();
  			
  			art->setEndOffset(newFile.at());
  			newFile.putch('\n');
  		}
  	}
 		newFile.close();
 		oldFile.close();
 		f->syncDynamicData(true);
 		f->saveInfo();
  		
 		QString cmd("rm -f "+oldName);
 		system(cmd.local8Bit().data());
 		cmd="mv "+newName+" "+oldName;
 		system(cmd.local8Bit().data());
  }
}



