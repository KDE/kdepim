/***************************************************************************
                          knfiltersettings.cpp  -  description
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

#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qbitarray.h>
#include <qcombobox.h>

#include <kiconloader.h>
#include <klocale.h>

#include "knstringfilter.h"
#include "knrangefilter.h"
#include "knstatusfilter.h"
#include "knarticlefilter.h"
#include "knfiltermanager.h"
#include "knlistbox.h"
#include "knfiltersettings.h"


//=============================================================================================



KNFilterSettings::KNFilterSettings(KNFilterManager *fm, QWidget *p)
 : KNSettingsWidget(p), fiManager(fm)
{
  QGridLayout *topL=new QGridLayout(this, 6,2, 5,5);

  // == Filters =================================================

  topL->addWidget(new QLabel(i18n("<b>Filters:</b>"),this),0,0);

  flb=new KNListBox(this);
  connect(flb, SIGNAL(selectionChanged()), SLOT(slotSelectionChangedFilter()));
  connect(flb, SIGNAL(selected(int)), SLOT(slotItemSelectedFilter(int)));
  topL->addMultiCellWidget(flb,1,5,0,0);

  addBtn=new QPushButton(i18n("&Add"), this);
  connect(addBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(addBtn,1,1);

  delBtn=new QPushButton(i18n("&Delete"), this);
  connect(delBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(delBtn,2,1);

  editBtn=new QPushButton(i18n("&Edit"), this);
  connect(editBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(editBtn,3,1);

  copyBtn=new QPushButton(i18n("C&opy"), this);
  connect(copyBtn, SIGNAL(clicked()), this, SLOT(slotCopyBtnClicked()));
  topL->addWidget(copyBtn,4,1);

  // == Menu ====================================================

  topL->addWidget(new QLabel(i18n("<b>Menu:</b>"),this),6,0);

  mlb=new KNListBox(this);
  connect(mlb, SIGNAL(selectionChanged()), SLOT(slotSelectionChangedMenu()));
  topL->addMultiCellWidget(mlb,7,11,0,0);

  upBtn=new QPushButton(i18n("&Up"), this);
  connect(upBtn, SIGNAL(clicked()), this, SLOT(slotUpBtnClicked()));
  topL->addWidget(upBtn,7,1);

  downBtn=new QPushButton(i18n("D&own"), this);
  connect(downBtn, SIGNAL(clicked()), this, SLOT(slotDownBtnClicked()));
  topL->addWidget(downBtn,8,1);

  sepAddBtn=new QPushButton(i18n("Add\n&Separator"), this);
  connect(sepAddBtn, SIGNAL(clicked()), this, SLOT(slotSepAddBtnClicked()));
  topL->addWidget(sepAddBtn,9,1);

  sepRemBtn=new QPushButton(i18n("&Remove\nSeparator"), this);
  connect(sepRemBtn, SIGNAL(clicked()), this, SLOT(slotSepRemBtnClicked()));
  topL->addWidget(sepRemBtn,10,1);

  topL->setRowStretch(5,1);
  topL->setRowStretch(11,1);

  active = UserIcon("fltrblue");
  disabled = UserIcon("fltrgrey");

  fiManager->startConfig(this);

  slotSelectionChangedFilter();
  slotSelectionChangedMenu();
}



KNFilterSettings::~KNFilterSettings()
{
  fiManager->endConfig();
}



void KNFilterSettings::apply()
{
  fiManager->commitChanges();
}



void KNFilterSettings::addItem(KNArticleFilter *f)
{
  if(f->isEnabled())
    flb->insertItem(new KNLBoxItem(f->translatedName(),f,&active));
  else
    flb->insertItem(new KNLBoxItem(f->translatedName(),f,&disabled));
  slotSelectionChangedFilter();
}



void KNFilterSettings::removeItem(KNArticleFilter *f)
{
  int i=findItem(flb, f);
  if (i!=-1) flb->removeItem(i);
  slotSelectionChangedFilter();
}



void KNFilterSettings::updateItem(KNArticleFilter *f)
{
  int i=findItem(flb, f);
  
  if (i!=-1) {
    if(f->isEnabled()) {
      flb->changeItem(new KNLBoxItem(f->translatedName(),f,&active), i);
      mlb->changeItem(new KNLBoxItem(f->translatedName()), findItem(mlb, f));
    } else
      flb->changeItem(new KNLBoxItem(f->translatedName(),f,&disabled), i);
  }
  slotSelectionChangedFilter();
}



void KNFilterSettings::addMenuItem(KNArticleFilter *f)
{
  if (f) {
    if (findItem(mlb, f)==-1)
      mlb->insertItem(new KNLBoxItem(f->translatedName(),f));
  } else   // separator
    mlb->insertItem(new KNLBoxItem("==="));
  slotSelectionChangedMenu();
}



void KNFilterSettings::removeMenuItem(KNArticleFilter *f)
{
  int i=findItem(mlb, f);
  if(i!=-1) mlb->removeItem(i);
  slotSelectionChangedMenu();
}



QValueList<int> KNFilterSettings::menuOrder()
{
  KNArticleFilter *f;
  QValueList<int> lst;
  
  for(uint i=0; i<mlb->count(); i++) {
    f= static_cast<KNArticleFilter*>(mlb->itemAt(i)->data());
    if(f)
      lst << f->id();
    else
      lst << -1;
  }
 return lst;
}



int KNFilterSettings::findItem(KNListBox *l, KNArticleFilter *f)
{
  int idx=0;
  bool found=false;
  while(!found && idx < (int) l->count()) {
    found=(static_cast<KNArticleFilter*>(l->itemAt(idx)->data())==f);
    if(!found) idx++;
  }
  if(found) return idx;
  else return -1;
}



void KNFilterSettings::slotAddBtnClicked()
{
  fiManager->newFilter();
}



void KNFilterSettings::slotDelBtnClicked()
{
  if (flb->currentItem()!=-1)
    fiManager->deleteFilter(static_cast<KNArticleFilter*>(flb->itemAt(flb->currentItem())->data()));
}



void KNFilterSettings::slotEditBtnClicked()
{
  if (flb->currentItem()!=-1)
    fiManager->editFilter(static_cast<KNArticleFilter*>(flb->itemAt(flb->currentItem())->data()));
}



void KNFilterSettings::slotCopyBtnClicked()
{
  if (flb->currentItem()!=-1)
    fiManager->copyFilter(static_cast<KNArticleFilter*>(flb->itemAt(flb->currentItem())->data()));
}



void KNFilterSettings::slotUpBtnClicked()
{
  int c=mlb->currentItem();
  KNArticleFilter *f=0;
  
  if(c==0 || c==-1) return;
  f=static_cast<KNArticleFilter*>(mlb->itemAt(c)->data());
  if (f)
    mlb->insertItem(new KNLBoxItem(f->translatedName(),f),c-1);
  else
    mlb->insertItem(new KNLBoxItem("==="),c-1);
  mlb->removeItem(c+1);
  mlb->setCurrentItem(c-1);
}



void KNFilterSettings::slotDownBtnClicked()
{
  int c=mlb->currentItem();
  KNArticleFilter *f=0;
  
  if(c==-1 || c+1==(int)mlb->count()) return;
  f=static_cast<KNArticleFilter*>(mlb->itemAt(c)->data());
  if (f)
    mlb->insertItem(new KNLBoxItem(f->translatedName(),f),c+2);
  else
    mlb->insertItem(new KNLBoxItem("==="),c+2);
  mlb->removeItem(c);
  mlb->setCurrentItem(c+1);
}



void KNFilterSettings::slotSepAddBtnClicked()
{
  mlb->insertItem(new KNLBoxItem("==="), mlb->currentItem());
  slotSelectionChangedMenu();
}



void KNFilterSettings::slotSepRemBtnClicked()
{
  int c=mlb->currentItem();

  if ((c!=-1) && (mlb->itemAt(c)->data()==0))
     mlb->removeItem(c);
  slotSelectionChangedMenu();
}



void KNFilterSettings::slotItemSelectedFilter(int)
{
  slotEditBtnClicked();
}



void KNFilterSettings::slotSelectionChangedFilter()
{
  int curr = flb->currentItem();

  delBtn->setEnabled(curr!=-1);
  editBtn->setEnabled(curr!=-1);
  copyBtn->setEnabled(curr!=-1);
}



void KNFilterSettings::slotSelectionChangedMenu()
{
  int curr = mlb->currentItem();

  upBtn->setEnabled(curr>0);
  downBtn->setEnabled((curr!=-1)&&(curr+1!=(int)mlb->count()));
  sepRemBtn->setEnabled((curr!=-1) && (mlb->itemAt(curr)->data()==0));
}



//--------------------------------

#include "knfiltersettings.moc"
