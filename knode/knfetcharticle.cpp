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

#include "knfetcharticle.h"
#include "knstringsplitter.h"
#include "utilities.h"
#include "knhdrviewitem.h"

KNFetchArticle::KNFetchArticle()
{
	l_ines=-1;
	fTimeT=0;
	flags.fill(false,7);
	i_dRef=-1;
	thrLevel=0;
	unrFups=0;
	newFups=0;
	s_core=50;
}



KNFetchArticle::~KNFetchArticle()
{
}



void KNFetchArticle::clear()
{
	m_Id.resize(0);
	f_rom.resize(0);
	e_mail.resize(0);
	
	for(int i=0; i<5; i++) r_eferences[i].resize(0);
	KNArticle::clear();
}



void KNFetchArticle::parse()
{
	if(f_rom.isEmpty()) parseFrom(headerLine("From"));
	if(r_eferences[0].isEmpty()) parseReferences(headerLine("References"));
	if(m_Id.isEmpty()) m_Id=headerLine("Message-ID").copy();
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



void KNFetchArticle::parseReferences(const QCString &s)
{
	int RefNr=0;
	KNStringSplitter refs;
	QCString tmp;
	bool isRef;
		
	if(!s.isEmpty()) {
		refs.setIncludeSep(false);		
		refs.init(s, " ");
		isRef=refs.last();
	
		if(!isRef) {
			tmp=s.stripWhiteSpace();
			if(tmp!=m_Id) r_eferences[0]=tmp;
		}
		else {
		 	while(isRef && RefNr < 5) {
				tmp=refs.string().stripWhiteSpace();	
				if(m_Id!=tmp) {	
					r_eferences[RefNr]=tmp;
					RefNr++;
				}									
				isRef=refs.prev();
			}
		}
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
	
	char tmp[5];
	sprintf(tmp,"%3d", s_core);
	i_tem->setText(2, tmp);
}





