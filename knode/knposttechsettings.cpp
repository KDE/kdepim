/***************************************************************************
                          knposttechsettings.cpp  -  description
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

#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qtextstream.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include <kstddirs.h>
#include <kcharsets.h>
#include <kconfig.h>
#include <kseparator.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kglobal.h>

#include "utilities.h"
#include "knposttechsettings.h"


KNPostTechSettings::KNPostTechSettings(QWidget *p) : KNSettingsWidget(p)
{
  QGroupBox *ggb=new QGroupBox(i18n("General"), this);
  QGroupBox *xgb=new QGroupBox(i18n("X-Headers"), this);
  QLabel *l1, *l2, *l3, *l4, *l5;
      
  l1=new QLabel(i18n("Charset"), ggb);
  charset=new QComboBox(ggb);
  charset->insertStringList(KGlobal::charsets()->availableCharsetNames());
  l2=new QLabel(i18n("Encoding"), ggb);
  encoding=new QComboBox(ggb);
  encoding->insertItem("7 bit");
  encoding->insertItem("8 bit");
  encoding->insertItem("quoted-printable");
  allow8bitCB=new QCheckBox(i18n("allow 8-bit characters in the header"), ggb);
  genMIdCB=new QCheckBox(i18n("generate Message-Id"), ggb);
  l3=new QLabel(i18n("Hostname"), ggb);
  host=new QLineEdit(ggb);  
  host->setEnabled(false);
  lb=new QListBox(xgb);
  lb->setFocusPolicy(NoFocus);
  addBtn=new QPushButton(i18n("Add"), xgb);
  delBtn=new QPushButton(i18n("Delete"), xgb);
  KSeparator *sep=new KSeparator(xgb);
  l4=new QLabel("X-", xgb);
  hName=new QLineEdit(xgb);
  l5=new QLabel(":", xgb);
  hValue=new QLineEdit(xgb);
  okBtn=new QPushButton(i18n("OK"), xgb);
    
  QVBoxLayout *topL=new QVBoxLayout(this, 10);
  QGridLayout *ggbL=new QGridLayout(ggb, 5,2, 20,5);
  QGridLayout *xgbL=new QGridLayout(xgb, 6,5, 20,5);
  topL->addWidget(ggb);
  topL->addWidget(xgb, 1);
  ggbL->addWidget(l1, 0,0);
  ggbL->addWidget(charset, 0,1);
  ggbL->addWidget(l2, 1,0);
  ggbL->addWidget(encoding, 1,1);
  ggbL->addMultiCellWidget(allow8bitCB, 2,2, 0,1);
  ggbL->addMultiCellWidget(genMIdCB, 3,3, 0,1);
  ggbL->addWidget(l3, 4,0);
  ggbL->addWidget(host, 4,1);
  ggbL->setColStretch(1,1);
  xgbL->addMultiCellWidget(lb, 0,2, 0,3);
  xgbL->addWidget(addBtn, 0,4);
  xgbL->addWidget(delBtn, 1,4);
  xgbL->addMultiCellWidget(sep, 3,3, 0,4);
  xgbL->addWidget(l4, 4,0);
  xgbL->addWidget(hName, 4,1);
  xgbL->addWidget(l5, 4,2);
  xgbL->addMultiCellWidget(hValue, 4,4, 3,4);
  xgbL->addWidget(okBtn, 5,4);
  xgbL->addRowSpacing(3,20);
  xgbL->setRowStretch(2,1);
  xgbL->setColStretch(3,1);
  topL->setResizeMode(QLayout::Minimum);
  topL->activate();
  
  connect(genMIdCB, SIGNAL(toggled(bool)),
    this, SLOT(slotGenMIdCBtoggled(bool)));
  connect(addBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  connect(delBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  connect(okBtn, SIGNAL(clicked()), this, SLOT(slotOkBtnClicked()));
  connect(lb, SIGNAL(highlighted(int)), this, SLOT(slotItemSelected(int)));
  
  init();
}



KNPostTechSettings::~KNPostTechSettings()
{
}



void KNPostTechSettings::init()
{
  KConfig *conf=KGlobal::config();

  conf->setGroup("POSTNEWS");
  encoding->setCurrentItem(conf->readNumEntry("Encoding",1));
  QString tmp=conf->readEntry("Charset");
  allow8bitCB->setChecked(conf->readBoolEntry("allow8bitChars", false));
  if(!tmp.isEmpty()) {
    for(int i=0; i < charset->count(); i++)
      if(charset->text(i) == tmp) {
        charset->setCurrentItem(i);
        break;
      }
  }
  genMIdCB->setChecked(conf->readBoolEntry("generateMId", false));
  host->setText(conf->readEntry("MIdhost"));
  
  QString dir(KGlobal::dirs()->saveLocation("appdata"));
  if (dir==QString::null)
    displayInternalFileError();
  else {
    QFile f(dir+"xheaders");
    if(f.open(IO_ReadOnly)) {
      QTextStream ts(&f);
      while(!ts.eof()) {
        lb->insertItem(ts.readLine());
      }
      f.close();
    }     
  }
  
  saveHdrs=false;
  editEnabled=true;
  enableEdit(false);
  currentItem=-1;
}



void KNPostTechSettings::apply()
{
  KConfig *conf=KGlobal::config();

  conf->setGroup("POSTNEWS");
  conf->writeEntry("Encoding",encoding->currentItem());
  conf->writeEntry("Charset",charset->currentText());
  conf->writeEntry("allow8bitChars", allow8bitCB->isChecked());
  conf->writeEntry("generateMId", genMIdCB->isChecked());
  conf->writeEntry("MIdhost", host->text());
  
  if(saveHdrs) {
    QString dir(KGlobal::dirs()->saveLocation("appdata"));
    if (dir==QString::null)
      displayInternalFileError();
    else {
      QFile f(dir+"xheaders");;
      if(f.open(IO_WriteOnly)) {
        QTextStream ts(&f);
        for(unsigned int i=0; i<lb->count(); i++) ts << lb->text(i) << "\n";
        f.close();
      } 
    }
  } 
}



void KNPostTechSettings::enableEdit(bool e)
{
  hValue->clear();
  hName->clear();
  if(editEnabled!=e) {
    editEnabled=e;
    okBtn->setEnabled(e);
    hName->setEnabled(e);
    hValue->setEnabled(e);
  }
}



void KNPostTechSettings::slotGenMIdCBtoggled(bool b)
{
  host->setEnabled(b);
}



void KNPostTechSettings::slotAddBtnClicked()
{
  enableEdit(true);
  currentItem=-1;
}



void KNPostTechSettings::slotDelBtnClicked()
{
  enableEdit(false);
  int c=lb->currentItem();
  if(c!=-1) lb->removeItem(c);
  currentItem=-1;
  saveHdrs=true;  
}



void KNPostTechSettings::slotOkBtnClicked()
{
  QString item;
  item="X-";
  item+=hName->text();
  item+=": ";
  item+=hValue->text();
  if(currentItem==-1) lb->insertItem(item);
  else {
    lb->changeItem(item, currentItem);
    lb->clearSelection();
  }
  enableEdit(false);
  currentItem=-1;
  saveHdrs=true;
}



void KNPostTechSettings::slotItemSelected(int i)
{
  int pos1=2 ,pos2=0;
  QString txt, tmp;

  currentItem=i;
  if(i==-1) return;
  txt=lb->text(i);
  pos2=txt.find(":", 2);
  if(pos2==-1) return;
  else {
    enableEdit(true);
    tmp=txt.mid(pos1, pos2-pos1);
    hName->setText(tmp);
    tmp=txt.mid(pos2+2, txt.length()-pos2);
    hValue->setText(tmp);
  }
} 



//--------------------------------

#include "knposttechsettings.moc"

