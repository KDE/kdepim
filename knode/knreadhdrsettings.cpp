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
#include <qcombobox.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "utilities.h"
#include "knviewheader.h"
#include "knreadhdrsettings.h"


KNReadHdrSettings::ConfDialog::ConfDialog(KNViewHeader *header, QWidget *parent, char *name)
  : KDialogBase(Plain, i18n("Header Properties"),Ok|Cancel|Help, Ok, parent, name),
    hdr(header)
{
  QFrame* page=plainPage();
  QGridLayout *topL=new QGridLayout(page, 2, 2, 0, 5);

  QWidget *nameW = new QWidget(page);
  QGridLayout *nameL=new QGridLayout(nameW, 2, 2, 5);

  nameL->addWidget(new QLabel(i18n("Header:"),nameW),0,0);
  hdrC=new QComboBox(true, nameW);
  hdrC->lineEdit()->setMaxLength(64);
  connect(hdrC, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
  nameL->addWidget(hdrC,0,1);

  nameL->addWidget(new QLabel(i18n("Displayed Name:"),nameW),1,0);
  nameE=new QLineEdit(nameW);
  connect(nameE, SIGNAL(textChanged(const QString&)), SLOT(slotNameChanged(const QString&)));
  nameE->setMaxLength(64);
  nameL->addWidget(nameE,1,1);
  nameL->setColStretch(1,1);

  topL->addMultiCellWidget(nameW,0,0,0,1);

  QGroupBox *ngb=new QGroupBox(i18n("Name"), page);
  QVBoxLayout *ngbL = new QVBoxLayout(ngb, 8, 5);
  ngbL->setAutoAdd(true);
  ngbL->addSpacing(fontMetrics().lineSpacing()-4);
  nameCB[0]=new QCheckBox(i18n("large"), ngb);
  nameCB[1]=new QCheckBox(i18n("bold"), ngb);
  nameCB[2]=new QCheckBox(i18n("italic"), ngb);
  nameCB[3]=new QCheckBox(i18n("underlined"), ngb);
  topL->addWidget(ngb,1,0);

  QGroupBox *vgb=new QGroupBox(i18n("Value"), page);
  QVBoxLayout *vgbL = new QVBoxLayout(vgb, 8, 5);
  vgbL->setAutoAdd(true);
  vgbL->addSpacing(fontMetrics().lineSpacing()-4);
  valueCB[0]=new QCheckBox(i18n("large"), vgb);
  valueCB[1]=new QCheckBox(i18n("bold"), vgb);
  valueCB[2]=new QCheckBox(i18n("italic"), vgb);
  valueCB[3]=new QCheckBox(i18n("underlined"), vgb);
  topL->addWidget(vgb,1,1);

  topL->setColStretch(0,1);
  topL->setColStretch(1,1);

  // preset values...
  hdrC->insertStrList(KNViewHeader::predefs());
  hdrC->lineEdit()->setText(hdr->header());
  nameE->setText(hdr->translatedName());
  for(int i=0; i<4; i++) {
    nameCB[i]->setChecked(hdr->flag(i));
    valueCB[i]->setChecked(hdr->flag(i+4));
  }

  setFixedHeight(sizeHint().height());
  restoreWindowSize("accReadHdrPropDLG", this, sizeHint());

  setHelp("anc-knode-headers");
}


KNReadHdrSettings::ConfDialog::~ConfDialog()
{
  saveWindowSize("accReadHdrPropDLG", size());
}


void KNReadHdrSettings::ConfDialog::slotOk()
{
  hdr->setHeader(hdrC->currentText());
  hdr->setTranslatedName(nameE->text());
  for(int i=0; i<4; i++) {
    if (hdr->hasName())
      hdr->setFlag(i, nameCB[i]->isChecked());
    else
      hdr->setFlag(i,false);
    hdr->setFlag(i+4, valueCB[i]->isChecked());
  }
  accept();
}


// the user selected one of the presets, insert the *translated* string as display name:
void KNReadHdrSettings::ConfDialog::slotActivated(int pos)
{
  nameE->setText(i18n(hdrC->text(pos).local8Bit()));  // I think it's save here, the combobox has only english defaults
}


// disable the name format options when the name is empty
void KNReadHdrSettings::ConfDialog::slotNameChanged(const QString& str)
{
  for(int i=0; i<4; i++)
    nameCB[i]->setEnabled(!str.isEmpty());
}


//=============================================================================================


KNReadHdrSettings::HdrItem::HdrItem( const QString &text, KNViewHeader * header)
  : QListBoxText(text), hdr(header)
{
}


KNReadHdrSettings::HdrItem::~HdrItem()
{
}


//=============================================================================================


KNReadHdrSettings::KNReadHdrSettings(QWidget *p)
  : KNSettingsWidget(p), save(false)
{
  QGridLayout *topL=new QGridLayout(this, 7,2, 5,5);

  // account listbox
  lb=new QListBox(this);
  connect(lb, SIGNAL(selected(int)), this, SLOT(slotItemSelected(int)));
  connect(lb, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
  topL->addMultiCellWidget(lb, 0,6, 0,0);

  // buttons
  addBtn=new QPushButton(i18n("&Add"), this);
  connect(addBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(addBtn, 0,1);

  delBtn=new QPushButton(i18n("&Delete"), this);
  connect(delBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(delBtn, 1,1);

  editBtn=new QPushButton(i18n("modify something","&Edit"), this);
  connect(editBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(editBtn, 2,1);

  upBtn=new QPushButton(i18n("&Up"), this);
  connect(upBtn, SIGNAL(clicked()), this, SLOT(slotUpBtnClicked()));
  topL->addWidget(upBtn, 4,1);

  downBtn=new QPushButton(i18n("D&own"), this);
  connect(downBtn, SIGNAL(clicked()), this, SLOT(slotDownBtnClicked()));
  topL->addWidget(downBtn, 5,1);

  topL->addRowSpacing(3,20);        // separate up/down buttons
  topL->setRowStretch(6,1);         // stretch the listbox

  for(KNViewHeader *h=KNViewHeader::first(); h; h=KNViewHeader::next())
    lb->insertItem(generateItem(h));

  slotSelectionChanged();     // disable buttons initially
}



KNReadHdrSettings::~KNReadHdrSettings()
{
  if (save)
    KNViewHeader::saveAll();
}



KNReadHdrSettings::HdrItem* KNReadHdrSettings::generateItem(KNViewHeader *h)
{
  QString text;
  if(h->hasName()) {
    text=h->translatedName();
    text+=": <";
  } else
    text="<";
  text+=h->header();
  text+=">";
  return new HdrItem(text,h);
}



void KNReadHdrSettings::slotItemSelected(int)
{
  slotEditBtnClicked();
}



void KNReadHdrSettings::slotSelectionChanged()
{
  int curr = lb->currentItem();
  delBtn->setEnabled(curr!=-1);
  editBtn->setEnabled(curr!=-1);
  upBtn->setEnabled(curr>0);
  downBtn->setEnabled((curr!=-1)&&(curr+1!=(int)lb->count()));
}



void KNReadHdrSettings::slotAddBtnClicked()
{
  KNViewHeader *h=KNViewHeader::newItem();

  ConfDialog* dlg=new ConfDialog(h, this, 0);
  if (dlg->exec()) {
    lb->insertItem(generateItem(h));
    h->createTags();
    save=true;
  } else
    KNViewHeader::remove(h);
}



void KNReadHdrSettings::slotDelBtnClicked()
{
  if (lb->currentItem()==-1) return;
  if (KMessageBox::questionYesNo(this, i18n("Really delete this header?"))==KMessageBox::Yes) {
    KNViewHeader *h = static_cast<HdrItem*>(lb->item(lb->currentItem()))->hdr;
    KNViewHeader::remove(h);
    lb->removeItem(lb->currentItem());
    save=true;
  } 
}



void KNReadHdrSettings::slotEditBtnClicked()
{
  if (lb->currentItem()==-1) return;
  KNViewHeader *h = static_cast<HdrItem*>(lb->item(lb->currentItem()))->hdr;

  ConfDialog* dlg=new ConfDialog(h, this, 0);
  if (dlg->exec()) {
    lb->changeItem(generateItem(h),lb->currentItem());
    h->createTags();
    save=true;
  }
}



void KNReadHdrSettings::slotUpBtnClicked()
{
  int c=lb->currentItem();
  if(c==0 || c==-1) return;

  KNViewHeader *h = static_cast<HdrItem*>(lb->item(c))->hdr;

  KNViewHeader::up(h);  
  lb->insertItem(generateItem(h), c-1);
  lb->removeItem(c+1);
  lb->setCurrentItem(c-1);  
  save=true;
}



void KNReadHdrSettings::slotDownBtnClicked()
{
  int c=lb->currentItem();
  if(c==-1 || c==(int) lb->count()-1) return;

  KNViewHeader *h = static_cast<HdrItem*>(lb->item(c))->hdr;

  KNViewHeader::down(h);
  lb->insertItem(generateItem(h), c+2);
  lb->removeItem(c);
  lb->setCurrentItem(c+1);
  save=true;
}


//--------------------------------

#include "knreadhdrsettings.moc"

