/***************************************************************************
                          knstringfilter.cpp  -  description
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
#include <qregexp.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>

#include <klocale.h>
#include <ksimpleconfig.h>

#include "knstringfilter.h"


KNStringFilter& KNStringFilter::operator=(const KNStringFilter &sf)
{
  con=sf.con;
  data=sf.data;
  enabled=sf.enabled;
  regExp=sf.regExp;
    
  return (*this);
}



bool KNStringFilter::doFilter(const QCString &s)
{
  bool ret=true;
  
  
  if(enabled) {
    if(regExp) ret=(s.contains(QRegExp(data)) > 0);
    else ret=(s.find(data,0,false)!=-1);

    if(!con) ret=!ret;

  }

  return ret;
}



void KNStringFilter::load(KSimpleConfig *conf)
{
  enabled=conf->readBoolEntry("enabled", false);
  con=conf->readBoolEntry("contains", true);
  data=conf->readEntry("Data").local8Bit();
  regExp=conf->readBoolEntry("regX", false);
}


#warning uhmm, shouldnt be data a QString?
void KNStringFilter::save(KSimpleConfig *conf)
{
  conf->writeEntry("enabled", enabled);
  conf->writeEntry("contains", con);
  conf->writeEntry("Data", data.data());
  conf->writeEntry("regX", regExp);
}


//===============================================================================

KNStringFilterWidget::KNStringFilterWidget(const QString& title, QWidget *parent)
  : QGroupBox(title, parent)
{
  enabled=new QCheckBox(this);
  
  fType=new QComboBox(this);
  fType->insertItem(i18n("does contain"));
  fType->insertItem(i18n("does NOT contain"));
  
  fString=new QLineEdit(this);
  
  regExp=new QCheckBox(i18n("regular Expression"), this);
  
  QGridLayout *topL=new QGridLayout(this, 2,3, 20,10);
  
  topL->addWidget(enabled, 0,0);
  topL->addWidget(fType, 0,1);
  topL->addWidget(regExp, 0,2);
  topL->addMultiCellWidget(fString, 1,1, 1,2);

  topL->setColStretch(2,1); 
  topL->activate();
  
  connect(enabled, SIGNAL(toggled(bool)), this, SLOT(slotEnabled(bool)));
  enabled->setChecked(false);
  slotEnabled(false);   
}



KNStringFilterWidget::~KNStringFilterWidget()
{
}



KNStringFilter KNStringFilterWidget::filter()
{
  KNStringFilter ret;
  ret.con=(fType->currentItem()==0);
  ret.data=fString->text().local8Bit();
  ret.enabled=enabled->isChecked();
  ret.regExp=regExp->isChecked();
  
  return ret;
}



void KNStringFilterWidget::setFilter(KNStringFilter &f)
{
  enabled->setChecked(f.enabled);
  if(f.con) fType->setCurrentItem(0);
  else fType->setCurrentItem(1);
  fString->setText(f.data);
  regExp->setChecked(f.regExp);
}



void KNStringFilterWidget::clear()
{
  fString->clear();
  enabled->setChecked(false);
  fType->setCurrentItem(0);
  regExp->setChecked(false);
}



void KNStringFilterWidget::slotEnabled(bool e)
{
  fType->setEnabled(e);
  fString->setEnabled(e);
  regExp->setEnabled(e);
}


// -----------------------------------------------------------------------------+

#include "knstringfilter.moc"

