/***************************************************************************
                          knscoredialog.cpp  -  description
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
#include <qlabel.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qbuttongroup.h>

#include <klocale.h>

#include "utilities.h"
#include "knscoredialog.h"


KNScoreDialog::KNScoreDialog(short sc, QWidget *parent, const char *name )
  : KDialogBase(Plain, i18n("Set Score"), Ok|Cancel, Ok, parent, name)
{
  QFrame* page=plainPage();
  QVBoxLayout *topL=new QVBoxLayout(page, 10);

  QButtonGroup *bg=new QButtonGroup(page);
  topL->addWidget(bg);
  
  QGridLayout *bgL=new QGridLayout(bg, 4,2,10);
  
  iBtn=new QRadioButton("0", bg);
  bgL->addWidget(iBtn, 0,0);
  
  nBtn=new QRadioButton("50", bg);
  bgL->addWidget(nBtn, 1,0);

  wBtn=new QRadioButton("100", bg);
  bgL->addWidget(wBtn, 2,0);

  cBtn=new QRadioButton(i18n("custom"), bg);
  bgL->addWidget(cBtn, 3,0);
  spin=new QSpinBox(0,100,1,bg);
  connect(cBtn, SIGNAL(toggled(bool)), spin, SLOT(setEnabled(bool)));
  bgL->addWidget(spin, 3,1);

  bgL->setColStretch(1,1);    
  topL->activate();
  
  int b;
  
  switch(sc) {
  
    case 0:   b=0; break;
    case 50:  b=1; break;
    case 100: b=2; break;
    default:  b=3; break;
  }
  
  spin->setValue(sc);
  bg->setButton(b);
  spin->setEnabled(cBtn->isChecked());

  setFixedHeight(sizeHint().height());
  restoreWindowSize("scoreDlg", this, sizeHint());
}



KNScoreDialog::~KNScoreDialog()
{
  saveWindowSize("scoreDlg", size());
}



short KNScoreDialog::score()
{
  short ret=0;
  
  if (iBtn->isChecked())      ret=0;
  else if (nBtn->isChecked()) ret=50;
  else if (wBtn->isChecked()) ret=100;
  else if (cBtn->isChecked()) ret=spin->value();
  
  return ret;
}


//--------------------------------

#include "knscoredialog.moc"

