/*
    knjobdata.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qstrlist.h>

#include "kngroup.h"
#include "knmime.h"
#include "knjobdata.h"
#include "knglobals.h"
#include "knnetaccess.h"

KNJobConsumer::KNJobConsumer()
{
  j_obs.setAutoDelete(false);
}


KNJobConsumer::~KNJobConsumer()
{
  for(KNJobData *j=j_obs.first(); j; j=j_obs.next())
    j->c_onsumer=0;
}


void KNJobConsumer::emitJob(KNJobData *j)
{
  if(j) {
    j_obs.append(j);
    knGlobals.netAccess->addJob(j);
  }
}


void KNJobConsumer::jobDone(KNJobData *j)
{
  if(j && j_obs.removeRef(j))
    processJob(j);
}


void KNJobConsumer::processJob(KNJobData *j)
{
  delete j;
}


// the assingment of a_ccount may cause race conditions, check again.... (CG)
KNJobData::KNJobData(jobType t, KNJobConsumer *c, KNServerInfo *a, KNJobItem *i)
 : t_ype(t), d_ata(i), a_ccount(a), c_anceled(false), a_uthError(false), c_onsumer(c)
{
  d_ata->setLocked(true);
}



KNJobData::~KNJobData()
{
  d_ata->setLocked(false);
}


void KNJobData::notifyConsumer()
{

  if(c_onsumer)
    c_onsumer->jobDone(this);
  else
    delete this;
}
