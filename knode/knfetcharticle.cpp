/***************************************************************************
                          knfetcharticle.cpp  -  description
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

#include "knhdrviewitem.h"
#include "knfetcharticle.h"


KNFetchArticle::KNFetchArticle() : t_hreaded(true), i_dRef(-1), l_ines(-1), thrLevel(0), s_core(50),  newFups(0), unrFups(0)
{
  flags.fill(false,8);
}



KNFetchArticle::~KNFetchArticle()
{
}



void KNFetchArticle::clear()
{
  m_Id.resize(0);
  f_rom.resize(0);
  e_mail.resize(0);
    
  KNArticle::clear();
}



// Attention: this method is called from the network thread!
void KNFetchArticle::parse()
{
  if(f_rom.isEmpty()) parseFrom(headerLine("From"));
  if(m_Id.isEmpty()) m_Id=headerLine("Message-ID");
  KNArticle::parse();
}



void KNFetchArticle::parseFrom(const QCString &s)
{
  FromLineParser p(s);
  p.parse();
  if(p.hasValidEmail()) {
    e_mail=p.email();
    if(!p.hasValidFrom()) f_rom=e_mail;
    else f_rom=p.from();
  }
  else {
    if(!p.hasValidFrom()) f_rom="no name";
    else f_rom=p.from();
    e_mail="no email";
  }
}



void KNFetchArticle::initListItem()
{
  i_tem->setText(0, s_ubject);
  i_tem->setText(1, f_rom);
  i_tem->setText(3, timeString());
  updateListItem();
}



void KNFetchArticle::updateListItem()
{
  if(!i_tem) return;                
    
  if(isRead()) {
    if(hasContent()) i_tem->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTgreyBallChkd));
    else i_tem->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTgreyBall));
  }
  else {
    if(hasContent()) i_tem->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTredBallChkd));
    else i_tem->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTredBall));
  }
  
  if(hasNewFollowUps()) i_tem->setPixmap(1, KNLVItemBase::icon(KNLVItemBase::PTnewFups));
  else i_tem->setPixmap(1, KNLVItemBase::icon(KNLVItemBase::PTnull));
  
  if(s_core==100) i_tem->setPixmap(2, KNLVItemBase::icon(KNLVItemBase::PTeyes));
  else i_tem->setPixmap(2, KNLVItemBase::icon(KNLVItemBase::PTnull));
  
  i_tem->setText(2, QString("%1").arg(s_core,3));
  
  i_tem->setExpandable(t_hreaded && hasFollowUps());

  i_tem->repaint(); //force repaint
}





