/***************************************************************************
                          knstatusfilter.cpp  -  description
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

#include <qlayout.h>
#include <qbitarray.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <ksimpleconfig.h>

#include "knfetcharticle.h"
#include "knstatusfilter.h"


KNStatusFilter::KNStatusFilter()
{
  data.fill(false,8);
}



KNStatusFilter::~KNStatusFilter()
{
}



void KNStatusFilter::load(KSimpleConfig *conf)
{
  data.setBit(EN_R, conf->readBoolEntry("EN_R", false));
  data.setBit(DAT_R, conf->readBoolEntry("DAT_R", false));
  
  data.setBit(EN_N, conf->readBoolEntry("EN_N", false));
  data.setBit(DAT_N, conf->readBoolEntry("DAT_N", false));
  
  data.setBit(EN_US, conf->readBoolEntry("EN_US", false));
  data.setBit(DAT_US, conf->readBoolEntry("DAT_US", false));
  
  data.setBit(EN_NS, conf->readBoolEntry("EN_NS", false));
  data.setBit(DAT_NS, conf->readBoolEntry("DAT_NS", false));
  
}



void KNStatusFilter::save(KSimpleConfig *conf)
{
  conf->writeEntry("EN_R", data.at(EN_R));
  conf->writeEntry("DAT_R", data.at(DAT_R));
  
  conf->writeEntry("EN_N", data.at(EN_N));
  conf->writeEntry("DAT_N", data.at(DAT_N));
  
  conf->writeEntry("EN_US", data.at(EN_US));
  conf->writeEntry("DAT_US", data.at(DAT_US));
  
  conf->writeEntry("EN_NS", data.at(EN_NS));
  conf->writeEntry("DAT_NS", data.at(DAT_NS));
}



bool KNStatusFilter::doFilter(KNFetchArticle *a)
{
  bool ret=true;  

  if(data.at(EN_R) && ret)
    ret=(a->isRead() == data.at(DAT_R));
  
  if(data.at(EN_N) && ret)
    ret=(a->isNew() == data.at(DAT_N));
  
  if(data.at(EN_US) && ret)
    ret=(a->hasUnreadFollowUps() == data.at(DAT_US));
  
  if(data.at(EN_NS) && ret)
    ret=(a->hasNewFollowUps() == data.at(DAT_NS));
  
  return ret;
}



//==============================================================================

KNStatusFilterWidget::KNStatusFilterWidget(QWidget *parent) :
  QButtonGroup(0, parent)
{
  setFrameStyle(NoFrame);
  enR=new QCheckBox(i18n("is read"), this);
  enN=new QCheckBox(i18n("is new"), this);
  enUS=new QCheckBox(i18n("has unread followups"), this);
  enNS=new QCheckBox(i18n("has new followups"), this);
  
  rCom=new TFCombo(this);
  nCom=new TFCombo(this);
  usCom=new TFCombo(this);
  nsCom=new TFCombo(this);
    
  QGridLayout *topL=new QGridLayout(this, 5, 3, 15,5);
  topL->addWidget(enR,0,0); topL->addWidget(rCom,0,1);
  topL->addWidget(enN,1,0); topL->addWidget(nCom,1,1);
  topL->addWidget(enUS,2,0); topL->addWidget(usCom,2,1);
  topL->addWidget(enNS,3,0); topL->addWidget(nsCom,3,1);
  topL->setColStretch(2,1);
  topL->setRowStretch(4,1);

  connect(this, SIGNAL(clicked(int)), this, SLOT(slotEnabled(int)));
}



KNStatusFilterWidget::~KNStatusFilterWidget()
{
}



KNStatusFilter KNStatusFilterWidget::filter()
{
  KNStatusFilter f;
  
  f.data.setBit(EN_R, enR->isChecked());
  f.data.setBit(DAT_R, rCom->value());
  
  f.data.setBit(EN_N, enN->isChecked());
  f.data.setBit(DAT_N, nCom->value());
  
  f.data.setBit(EN_US, enUS->isChecked());
  f.data.setBit(DAT_US, usCom->value());
  
  f.data.setBit(EN_NS, enNS->isChecked());
  f.data.setBit(DAT_NS, nsCom->value());
  
  return f;
}



void KNStatusFilterWidget::setFilter(KNStatusFilter &f)
{
  enR->setChecked(f.data.at(EN_R));
  rCom->setValue(f.data.at(DAT_R));
  
  enN->setChecked(f.data.at(EN_N));
  nCom->setValue(f.data.at(DAT_N));
  
  enUS->setChecked(f.data.at(EN_US));
  usCom->setValue(f.data.at(DAT_US));
  
  enNS->setChecked(f.data.at(EN_NS));
  nsCom->setValue(f.data.at(DAT_NS));
  
  for(int i=0; i<4; i++) slotEnabled(i);
}


void KNStatusFilterWidget::clear()
{
  enR->setChecked(false);
  enN->setChecked(false);
  enUS->setChecked(false);
  enNS->setChecked(false);
  rCom->setValue(true);
  nCom->setValue(true);
  nsCom->setValue(true);
  usCom->setValue(true);
  
  for(int i=0; i<4; i++) slotEnabled(i);
}



void KNStatusFilterWidget::slotEnabled(int c)
{
  switch(c) {
  
    case 0: rCom->setEnabled(enR->isChecked());     break;
    case 1: nCom->setEnabled(enN->isChecked());     break;
    case 2: usCom->setEnabled(enUS->isChecked());   break;
    case 3: nsCom->setEnabled(enNS->isChecked());   break;
  };
}


//==============================================================================


KNStatusFilterWidget::TFCombo::TFCombo(QWidget *parent) : QComboBox(parent)
{
  insertItem(i18n("true"));
  insertItem(i18n("false"));
}



KNStatusFilterWidget::TFCombo::~TFCombo()
{
}



//--------------------------------

#include "knstatusfilter.moc"
















