/***************************************************************************
                          knarticlefilter.cpp  -  description
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

#include <qcombobox.h>

#include <kstddirs.h>
#include <ksimpleconfig.h>

#include "kngroup.h"
#include "knfetcharticle.h"
#include "knstatusfilter.h"
#include "knrangefilter.h"
#include "knstringfilter.h"
#include "utilities.h"
#include "knarticlefilter.h"


KNArticleFilter::KNArticleFilter(int id)
:	i_d(id), c_ount(0), l_oaded(false), e_nabled(false), apon(articles)
{}



KNArticleFilter::~KNArticleFilter()
{}



bool KNArticleFilter::loadInfo()
{
 	if (i_d!=-1) {
 		QString fname(KGlobal::dirs()->findResource("appdata",QString("filters/%1.fltr").arg(i_d)));
 		if (fname==QString::null)
 			return false;
	  KSimpleConfig conf(fname,true);

		conf.setGroup("GENERAL");
		n_ame=conf.readEntry("name");
		e_nabled=conf.readBoolEntry("enabled", true);
		apon=(ApOn) conf.readNumEntry("applyOn", 0);
		return true;
	}
	return false;
}



void KNArticleFilter::load()
{
	QString fname(KGlobal::dirs()->findResource("appdata",QString("filters/%1.fltr").arg(i_d)));
	if (fname==QString::null)
		return;
  KSimpleConfig conf(fname,true);
	
	conf.setGroup("STATUS");
	status.load(&conf);
	
	conf.setGroup("SCORE");
	score.load(&conf);
	
	conf.setGroup("AGE");
	age.load(&conf);
	
	conf.setGroup("LINES");
	lines.load(&conf);
	
	conf.setGroup("SUBJECT");
	subject.load(&conf);
	
	conf.setGroup("FROM");
	from.load(&conf);
	
	l_oaded=true;
	
	qDebug("KNMessageFilter: filter loaded \"%s\" ", n_ame.latin1());
	
}



void KNArticleFilter::save()
{
	if (i_d==-1)
		return;
	QString dir(KGlobal::dirs()->saveLocation("appdata","filters/"));
	if (dir==QString::null) {
		displayInternalFileError();
		return;
	}
	KSimpleConfig conf(dir+QString("%1.fltr").arg(i_d));
	
	conf.setGroup("GENERAL");
	conf.writeEntry("name", QString(n_ame));
	conf.writeEntry("enabled", e_nabled);
	conf.writeEntry("applyOn", (int) apon);
	
	conf.setGroup("STATUS");
	status.save(&conf);
	
	conf.setGroup("SCORE");
	score.save(&conf);
	
	conf.setGroup("AGE");
	age.save(&conf);
	
	conf.setGroup("LINES");
	lines.save(&conf);
	
	conf.setGroup("SUBJECT");
	subject.save(&conf);
	
	conf.setGroup("FROM");
	from.save(&conf);
	
	qDebug("KNMessageFilter: filter saved \"%s\" ", n_ame.latin1());
}



void KNArticleFilter::doFilter(KNGroup *g)
{
	c_ount=0;
	KNFetchArticle *art, *ref;
	int idRef;
	
	if(!l_oaded) load();
	
		
	for(int idx=0; idx<g->length(); idx++) {
	
		art=g->at(idx);
	
	  if(!art->filtered()) {
	  	
	  	if(applyFilter(art)) {
	  		
	  	 	if(apon==threads) {
	  			idRef=art->idRef();
	  		
	  			while(idRef!=0) {
	  				ref=g->byId(idRef);
	  				ref->setFilterResult(true);
	  				ref->setFiltered(true);
	  				idRef=ref->idRef();
	  			}
	  		}
	  		else c_ount++;
	  	}
	  }
	}
		
	  	
	if(apon==threads) {
		bool inThread;
		for(int idx=0; idx<g->length(); idx++) {
	 		art=g->at(idx);
	 		if(!art->filterResult()) {
	 			inThread=false;
	 			idRef=art->idRef();
	 			
	 			while(idRef!=0 && !inThread) {
	  			ref=g->byId(idRef);
	  			inThread=ref->filterResult();
	  			idRef=ref->idRef();
	  		}
	  		
	  		art->setFilterResult(inThread);
	  		if(inThread) c_ount++;
	  	}
	  	else c_ount++;
	  }
	}
}



bool KNArticleFilter::applyFilter(KNFetchArticle *a)
{
	bool result=true;
	
	if(result) result=status.doFilter(a);
	if(result) result=score.doFilter(a->score());
	if(result) result=lines.doFilter(a->lines());
	if(result) result=age.doFilter(a->age());	
  if(result) result=subject.doFilter(a->subject());
  if(result) result=from.doFilter(a->fromName());
	
	a->setFilterResult(result);
	a->setFiltered(true);
		
	return result;
}



/*bool KNArticleFilter::applyFilter(KNSavedArticle *a)
{
	bool result=true;
	
	if(result) result=age.doFilter(a->age());	
  if(result) result=subject.doFilter(a->subject());
  //if(result) result=from.doFilter(a->fromName());
			
	return result;
} */
