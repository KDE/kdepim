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
#include <kmessagebox.h>

#include "utilities.h"
#include "knposttechsettings.h"


KNPostTechSettings::KNPostTechSettings(QWidget *p) : KNSettingsWidget(p)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  // ==== General =============================================================

  QGroupBox *ggb=new QGroupBox(i18n("General"), this);
  QGridLayout *ggbL=new QGridLayout(ggb, 6,2, 8,5);
  topL->addWidget(ggb);

  ggbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  ggbL->addWidget(new QLabel(i18n("Charset"), ggb), 1,0);
  charset=new QComboBox(ggb);
  charset->insertItem("us-ascii");
  charset->insertStringList(KGlobal::charsets()->availableCharsetNames());
  ggbL->addWidget(charset, 1,1);

  ggbL->addWidget(new QLabel(i18n("Encoding"), ggb), 2,0);
  encoding=new QComboBox(ggb);
  encoding->insertItem("7 bit");
  encoding->insertItem("8 bit");
  encoding->insertItem("quoted-printable");
  ggbL->addWidget(encoding, 2,1);

  allow8bitCB=new QCheckBox(i18n("don't encode 8-bit characters in the header"), ggb);
  ggbL->addMultiCellWidget(allow8bitCB, 3,3, 0,1);

  genMIdCB=new QCheckBox(i18n("generate Message-Id"), ggb);
  connect(genMIdCB, SIGNAL(toggled(bool)), this, SLOT(slotGenMIdCBtoggled(bool)));
  ggbL->addMultiCellWidget(genMIdCB, 4,4, 0,1);
  hostL=new QLabel(i18n("Hostname"), ggb);
  hostL->setEnabled(false);
  ggbL->addWidget(hostL, 5,0);
  host=new QLineEdit(ggb);
  host->setEnabled(false);
  ggbL->addWidget(host, 5,1);

  ggbL->setColStretch(1,1);

  // ==== X-Headers =============================================================

  QGroupBox *xgb=new QGroupBox(i18n("X-Headers"), this);
  topL->addWidget(xgb, 1);
  QGridLayout *xgbL=new QGridLayout(xgb, 6,2, 8,5);

  xgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  lb=new QListBox(xgb);
  connect(lb, SIGNAL(selected(int)), SLOT(slotItemSelected(int)));
  connect(lb, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
  xgbL->addMultiCellWidget(lb, 1,4, 0,0);

  addBtn=new QPushButton(i18n("&Add"), xgb);
  connect(addBtn, SIGNAL(clicked()), SLOT(slotAddBtnClicked()));
  xgbL->addWidget(addBtn, 1,1);

  delBtn=new QPushButton(i18n("&Delete"), xgb);
  connect(delBtn, SIGNAL(clicked()), SLOT(slotDelBtnClicked()));
  xgbL->addWidget(delBtn, 2,1);

  editBtn=new QPushButton(i18n("&Edit"), xgb);
  connect(editBtn, SIGNAL(clicked()), SLOT(slotEditBtnClicked()));
  xgbL->addWidget(editBtn, 3,1);

  incUaCB=new QCheckBox(i18n("don't add the \"User-Agent\" identification header"), xgb);
  xgbL->addMultiCellWidget(incUaCB, 5,5, 0,1);

  xgbL->setRowStretch(4,1);
  xgbL->setColStretch(0,1);

  init();

  slotSelectionChanged();

  connect(allow8bitCB, SIGNAL(toggled(bool)), this, SLOT(slotHeadEnctoggled(bool)));
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
  incUaCB->setChecked(conf->readBoolEntry("dontIncludeUA", false));

  saveHdrs=false;
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
  conf->writeEntry("dontIncludeUA", incUaCB->isChecked());
  
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



void KNPostTechSettings::slotHeadEnctoggled(bool b)
{
  if (b)
    KMessageBox::information(this,i18n("Please be aware that unencoded 8-bit characters\nare illegal in the header of a usenet message.\nUse this feature with extreme care!"));
}



void KNPostTechSettings::slotGenMIdCBtoggled(bool b)
{
  host->setEnabled(b);
  hostL->setEnabled(b);
}



void KNPostTechSettings::slotSelectionChanged()
{
  delBtn->setEnabled(lb->currentItem()!=-1);
  editBtn->setEnabled(lb->currentItem()!=-1);
}



void KNPostTechSettings::slotItemSelected(int id)
{
  slotEditBtnClicked();
}



void KNPostTechSettings::slotAddBtnClicked()
{
  XHeadDlg *dlg=new XHeadDlg(this);
  if (dlg->exec()) {
    lb->insertItem(dlg->headResult());
    saveHdrs=true;
  }
  delete dlg;

  slotSelectionChanged();
}



void KNPostTechSettings::slotDelBtnClicked()
{
  int c=lb->currentItem();
  if (c == -1)
    return;

  lb->removeItem(c);
  slotSelectionChanged();
  saveHdrs=true;
}



void KNPostTechSettings::slotEditBtnClicked()
{
  int c=lb->currentItem();
  if (c == -1)
    return;

  XHeadDlg *dlg=new XHeadDlg(this,lb->text(c));
  if (dlg->exec()) {
    lb->changeItem(dlg->headResult(),c);
    saveHdrs=true;
  }
  delete dlg;

  slotSelectionChanged();
}


//=====================================================================


KNPostTechSettings::XHeadDlg::XHeadDlg(QWidget *parent, const QString &header)
  : KDialogBase(Plain, i18n("X-Headers"),Ok|Cancel, Ok, parent)
{
  QFrame* page=plainPage();
  QHBoxLayout *topL=new QHBoxLayout(page, 5,8);
  topL->setAutoAdd(true);

  new QLabel("X-", page);
  name=new QLineEdit(page);
  new QLabel(":", page);
  value=new QLineEdit(page);

  int pos=header.find(":", 2);
  if (pos!=-1) {
    name->setText(header.mid(2, pos-2));
    value->setText(header.mid(pos+2, header.length()-pos));
  }

  setFixedHeight(sizeHint().height());
  restoreWindowSize("XHeaderDlg", this, sizeHint());
}



KNPostTechSettings::XHeadDlg::~XHeadDlg()
{
  saveWindowSize("XHeaderDlg", size());
}



QString KNPostTechSettings::XHeadDlg::headResult()
{
  return QString("X-%1: %2").arg(name->text()).arg(value->text());
}



//--------------------------------

#include "knposttechsettings.moc"

