/***************************************************************************
                          knsavedarticle.cpp  -  description
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

#include <qpixmap.h>

#include "knhdrviewitem.h"
#include "knsavedarticle.h"


KNSavedArticle::KNSavedArticle(articleStatus s)
{
  s_tatus=s;
  f_older=0;
  s_erverId=-1;
  s_tartOffset=-1;
  e_ndOffset=-1;
  mInfo=new KNMimeInfo();
  l_ocked=false;
}



KNSavedArticle::~KNSavedArticle()
{
}



void KNSavedArticle::parse()
{
  QCString tmp;
  if(s_tatus==ASunknown) {
    tmp=headerLine("X-KNode-Status");
    if(tmp.isEmpty()) s_tatus=ASsaved;
    else s_tatus=stringToArticleStatus(tmp.data());
  }
  if(d_estination.isEmpty()) {
    if((this->isMail())) d_estination=headerLine("To");
    else d_estination=headerLine("Newsgroups");
  }
  KNArticle::parse();
}



void KNSavedArticle::assemble()
{
  QCString tmp;

  KNArticle::assemble();

  tmp=articleStatusToString(s_tatus);
  setHeader(HTxknstatus, tmp);

  if(isMail())
    setHeader(HTto, d_estination, !allow8bit);
  else
    setHeader(HTnewsgroups, d_estination, false);

}



void KNSavedArticle::clear()
{
  d_estination.resize(0);
  s_tatus=ASunknown;
  KNArticle::clear(); 
}



void KNSavedArticle::updateListItem()
{
  if(!i_tem) return;
  i_tem->setText(1, d_estination);
  i_tem->setText(0, s_ubject);
  i_tem->setText(3, timeString());
  
  if (isMail())
    i_tem->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTmail));
  else {
    if (type()==ATsaved)
      i_tem->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTposting));
    else
      i_tem->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTcontrol));
  }

  if(sent() && !canceled()) i_tem->setPixmap(1, KNLVItemBase::icon(KNLVItemBase::PTstatusSent));
  else if(canceled()) i_tem->setPixmap(1, KNLVItemBase::icon(KNLVItemBase::PTstatusCanceled));
  else i_tem->setPixmap(1, KNLVItemBase::icon(KNLVItemBase::PTstatusEdit));
}



const QCString& KNSavedArticle::firstDestination()
{
  static QCString ret;
  int pos=0;
  if(!d_estination.isEmpty()) {
    pos=d_estination.find(',');
    if(pos==-1) ret=d_estination;
    else ret=d_estination.left(pos);
  }
  else ret="";
    
  return ret;
}


//==============================================================================================


KNControlArticle::KNControlArticle(controlType t, articleStatus s)
  : KNSavedArticle(s), c_tlType(t)
{
}



KNControlArticle::~KNControlArticle()
{
}
