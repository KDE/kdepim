/***************************************************************************
                          knpostcomsettings.cpp  -  description
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
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kfiledialog.h>

#include "knpostcomsettings.h"


KNPostComSettings::KNPostComSettings(QWidget *p) : KNSettingsWidget(p)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  // === general ===========================================================

  QGroupBox *generalB=new QGroupBox(i18n("General"), this);
  topL->addWidget(generalB);
  QGridLayout *generalL=new QGridLayout(generalB, 3,3, 8,5);

  generalL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  generalL->addWidget(new QLabel(i18n("word warp at column:"), generalB),1,0);
  maxLen=new QSpinBox(20, 100, 1, generalB);
  generalL->addWidget(maxLen,1,2);

  ownSigCB=new QCheckBox(i18n("append signature automatically"), generalB);
  generalL->addMultiCellWidget(ownSigCB,2,2,0,1);

  generalL->setColStretch(1,1);

  // === reply =============================================================

  QGroupBox *replyB=new QGroupBox(i18n("Reply"), this);
  topL->addWidget(replyB);
  QGridLayout *replyL=new QGridLayout(replyB, 6,2, 8,5);

  replyL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  replyL->addMultiCellWidget(new QLabel(i18n("Introduction Phrase:"), replyB),1,1,0,1);
  intro=new QLineEdit(replyB);
  replyL->addMultiCellWidget(intro, 2,2,0,1);
  replyL->addMultiCellWidget(new QLabel(i18n("Placeholders: %NAME=name, %EMAIL=email address,\n%DATE=date, %MSID=msgid"), replyB),3,3,0,1);

  rewarpCB=new QCheckBox(i18n("rewarp quoted text automatically"), replyB);
  replyL->addMultiCellWidget(rewarpCB, 5,5,0,1);

  authSigCB=new QCheckBox(i18n("include the authors signature"), replyB);
  replyL->addMultiCellWidget(authSigCB, 6,6,0,1);

  replyL->setColStretch(1,1);

  // === external editor ========================================================

  QGroupBox *editorB=new QGroupBox(i18n("External Editor"), this);
  topL->addWidget(editorB);
  QGridLayout *editorL=new QGridLayout(editorB, 6,3, 8,5);

  editorL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  editorL->addWidget(new QLabel(i18n("Specify Editor:"), editorB),1,0);
  editor=new QLineEdit(editorB);
  editorL->addWidget(editor,1,1);
  QPushButton *btn = new QPushButton(i18n("Ch&oose..."),editorB);
  connect(btn, SIGNAL(clicked()), SLOT(slotChooseEditor()));
  editorL->addWidget(btn,1,2);

  editorL->addMultiCellWidget(new QLabel(i18n("%f will be replaced with the filename to edit."), editorB),2,2,0,2);

  externCB=new QCheckBox(i18n("start external editor automatically"), editorB);
  editorL->addMultiCellWidget(externCB, 3,3,0,2);

  editorL->setColStretch(1,1);

  topL->addStretch(1);

  init();
}



KNPostComSettings::~KNPostComSettings()
{
}


void KNPostComSettings::slotChooseEditor()
{
  QString path=editor->text().simplifyWhiteSpace();
  if (path.right(3) == " %f")
    path.truncate(path.length()-3);

  path=KFileDialog::getOpenFileName(path, QString::null, this, i18n("Choose Editor"));

  if (!path.isEmpty())
    editor->setText(path+" %f");
}


void KNPostComSettings::init()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("POSTNEWS");

  maxLen->setValue(conf->readNumEntry("maxLength", 76));
  rewarpCB->setChecked(conf->readBoolEntry("rewarp",true));
  ownSigCB->setChecked(conf->readBoolEntry("appSig",true));
  intro->setText(conf->readEntry("Intro","%NAME wrote:"));
  authSigCB->setChecked(conf->readBoolEntry("incSig",false));
  editor->setText(conf->readEntry("externalEditor","kwrite %f"));
  externCB->setChecked(conf->readBoolEntry("useExternalEditor",false));
}



void KNPostComSettings::apply()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("POSTNEWS");
  
  conf->writeEntry("maxLength", maxLen->value());
  conf->writeEntry("rewarp",rewarpCB->isChecked());
  conf->writeEntry("appSig", ownSigCB->isChecked());
  conf->writeEntry("Intro", intro->text());
  conf->writeEntry("incSig", authSigCB->isChecked());
  conf->writeEntry("externalEditor",editor->text());
  conf->writeEntry("useExternalEditor",externCB->isChecked());
}

//--------------------------------

#include "knpostcomsettings.moc"
