/***************************************************************************
                          knreadhdrsettings.cpp  -  description
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

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "knglobals.h"
#include "knlistbox.h"
#include "knviewheader.h"
#include "knreadhdrsettings.h"


KNReadHdrSettings::KNReadHdrSettings(QWidget *p) : KNSettingsWidget(p)
{
  QGroupBox *ngb=new QGroupBox(i18n("Name"), this);
  QGroupBox *hgb=new QGroupBox(i18n("Header"), this);
  lb=new KNListBox(this);
  lb->setFocusPolicy(NoFocus);
  addBtn=new QPushButton(i18n("Add"), this);
  delBtn=new QPushButton(i18n("Delete"), this);
  upBtn=new QPushButton(i18n("Up"), this);
  downBtn=new QPushButton(i18n("Down"), this);
  okBtn=new QPushButton(i18n("OK"), this);
  name=new QLineEdit(ngb);
  name->setMaxLength(64);
  hdr=new QLineEdit(hgb);
  hdr->setMaxLength(64);
  createCBs(ngb, nameCB);
  createCBs(hgb, hdrCB);
  
  QVBoxLayout *topL=new QVBoxLayout(this, 7);
  QGridLayout *lbL=new QGridLayout(4,2, 3); 
  QGridLayout *ngbL=new QGridLayout(ngb, 4,2, 5,5); 
  QGridLayout *hgbL=new QGridLayout(hgb, 4,2, 5,5);
  QHBoxLayout *btnL=new QHBoxLayout(5);
  
  
  topL->addLayout(lbL, 1);
  topL->addWidget(ngb);
  topL->addWidget(hgb);
  topL->addLayout(btnL);
  lbL->addMultiCellWidget(lb, 0,3,0,0);
  lbL->addWidget(addBtn, 0,1);
  lbL->addWidget(delBtn, 1,1);
  lbL->addWidget(upBtn, 2,1);
  lbL->addWidget(downBtn, 3,1);
  lbL->setColStretch(0,1);
  ngbL->addMultiCellWidget(name, 1,1,0,1);
  ngbL->addWidget(nameCB[0], 2,0);
  ngbL->addWidget(nameCB[1], 2,1);
  ngbL->addWidget(nameCB[2], 3,0);
  ngbL->addWidget(nameCB[3], 3,1);
  ngbL->addRowSpacing(0, 10);
  hgbL->addMultiCellWidget(hdr, 1,1,0,1);
  hgbL->addWidget(hdrCB[0], 2,0);
  hgbL->addWidget(hdrCB[1], 2,1);
  hgbL->addWidget(hdrCB[2], 3,0);
  hgbL->addWidget(hdrCB[3], 3,1);
  hgbL->addRowSpacing(0, 10);
  btnL->addStretch(1);
  btnL->addWidget(okBtn);
  topL->setResizeMode(QLayout::Minimum);
  topL->activate();
  
  connect(lb, SIGNAL(highlighted(int)), this, SLOT(slotItemSelected(int)));
  connect(addBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  connect(delBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  connect(upBtn, SIGNAL(clicked()), this, SLOT(slotUpBtnClicked()));
  connect(downBtn, SIGNAL(clicked()), this, SLOT(slotDownBtnClicked()));
  connect(okBtn, SIGNAL(clicked()), this, SLOT(slotOkBtnClicked()));
  
  init(); 
}



KNReadHdrSettings::~KNReadHdrSettings()
{
  if(save) KNViewHeader::saveAll();
}



void KNReadHdrSettings::init()
{
  KNLBoxItem *it;
  KNViewHeader *h;
  QString text;
  
  enableEdit(false);
  
  for(h=KNViewHeader::first(); h; h=KNViewHeader::next()) {
    if(h->hasName()) {
      text=h->name();
      text+=": <";
    }
    else text="<";
    text+=h->header();
    text+=">";
    it=new KNLBoxItem(text, h, 0);
    lb->insertItem(it);
  }
  currentItem=-1;
  save=false;
}



void KNReadHdrSettings::apply()
{
}



void KNReadHdrSettings::enableEdit(bool b)
{
  name->clear();
  hdr->clear();
  if(nameCB[0]->isEnabled()!=b) {
    for(int i=0; i<4; i++) {
      nameCB[i]->setEnabled(b);
      hdrCB[i]->setEnabled(b);
    }
    name->setEnabled(b);
    hdr->setEnabled(b);
    okBtn->setEnabled(b);
  }   
}



void KNReadHdrSettings::createCBs(QWidget *p, QCheckBox **ptrs)
{
  ptrs[0]=new QCheckBox(i18n("large"), p);
  ptrs[1]=new QCheckBox(i18n("bold"), p);
  ptrs[2]=new QCheckBox(i18n("italic"), p);
  ptrs[3]=new QCheckBox(i18n("underlined"), p);
}


void KNReadHdrSettings::slotItemSelected(int idx)
{
  currentItem=idx;
  KNViewHeader *h;
  
  enableEdit((idx!=-1));    
  if(idx!=-1) {
    h=(KNViewHeader*)lb->itemAt(idx)->data();
    name->setText(h->name());
    hdr->setText (h->header());
    for(int i=0; i<4; i++) {
      nameCB[i]->setChecked(h->flag(i));
      hdrCB[i]->setChecked(h->flag(i+4));
    }
  }
}



void KNReadHdrSettings::slotAddBtnClicked()
{
  enableEdit(true);
  currentItem=lb->currentItem();
  if(currentItem!=-1) {
    lb->setSelected(currentItem, false);
    currentItem=-1;
  }
}



void KNReadHdrSettings::slotDelBtnClicked()
{
  KNViewHeader *h;
  if(currentItem==-1) return;
  if(KMessageBox::questionYesNo(this, i18n("Really delete this line?"))==KMessageBox::Yes) {
    enableEdit(false);
    h=(KNViewHeader*)lb->itemAt(currentItem)->data();
    KNViewHeader::remove(h);
    lb->removeItem(currentItem);
    currentItem=lb->currentItem();
    save=true;
  } 
}



void KNReadHdrSettings::slotUpBtnClicked()
{
  int c=lb->currentItem();
  KNViewHeader *h=0;
  KNLBoxItem *it;
  
  if(c==0 || c==-1) return;
  it=lb->itemAt(c);
  h=(KNViewHeader*) it->data();
  KNViewHeader::up(h);  
  lb->insertItem(new KNLBoxItem(it->text(), h,0), c-1);
  lb->removeItem(c+1);
  lb->setCurrentItem(c-1);  
  save=true;
}



void KNReadHdrSettings::slotDownBtnClicked()
{
  int c=lb->currentItem();
  KNViewHeader *h=0;
  KNLBoxItem *it;
  
  if(c==-1 || c==(int) lb->count()-1) return;
  it=lb->itemAt(c);
  h=(KNViewHeader*) it->data();
  KNViewHeader::down(h);  
  lb->insertItem(new KNLBoxItem(it->text(), h,0), c+2);
  lb->removeItem(c);
  lb->setCurrentItem(c+1);
  save=true;
}



void KNReadHdrSettings::slotOkBtnClicked()
{
  KNViewHeader *h;
  KNLBoxItem *it;
  QString text;
  qDebug("KNReadHdrSettings::slotOkBtnClicked()");  
  if(currentItem==-1) h=KNViewHeader::newItem();
  else h=(KNViewHeader*)lb->itemAt(currentItem)->data();  
  h->setName(name->text());
  h->setHeader(hdr->text());
  for(int i=0; i<4; i++) {
    if(h->hasName()) h->setFlag(i, nameCB[i]->isChecked());
    else h->setFlag(i,false);
    h->setFlag(i+4, hdrCB[i]->isChecked());
  }
  if(h->hasName()) {
    text=h->name();
    text+=": <";
  }
  else text="<";
  text+=h->header();
  text+=">";
  
  it=new KNLBoxItem(text, h, 0);
  if(currentItem==-1) lb->insertItem(it);
  else {
    lb->changeItem(it, currentItem);
    lb->clearSelection();
  }
  h->createTags();
  save=true;
  enableEdit(false);    
}



//--------------------------------

#include "knreadhdrsettings.moc"
