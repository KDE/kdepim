/***************************************************************************
                          knjobdata.cpp  -  description
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

#include <qstrlist.h>

#include "kngroup.h"
#include "knfetcharticle.h"
#include "knsavedarticle.h"
#include "knjobdata.h"


KNJobData::KNJobData(jobType t, KNServerInfo *a, void *d)
{
	t_ype=t;
	a_ccount=a;   // this may cause race conditions, check again.... (CG)
	d_ata=d;
	c_anceled=false;
	
	if(t_ype==JTfetchNewHeaders) ((KNGroup*)d_ata)->setLocked(true);
	else if(t_ype==JTfetchArticle) ((KNFetchArticle*)d_ata)->setLocked(true);
	else if(t_ype==JTpostArticle || t_ype==JTmail) ((KNSavedArticle*)d_ata)->setLocked(true);
}



KNJobData::~KNJobData()
{
	if(t_ype==JTlistGroups) delete (QStrList*)d_ata;
	else if(t_ype==JTfetchNewHeaders) ((KNGroup*)d_ata)->setLocked(false);
	else if(t_ype==JTfetchArticle) ((KNFetchArticle*)d_ata)->setLocked(false);
	else if(t_ype==JTpostArticle || t_ype==JTmail) ((KNSavedArticle*)d_ata)->setLocked(false);
}
