/*
    knstringfilter.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qlayout.h>
#include <qregexp.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>

#include <klocale.h>
#include <ksimpleconfig.h>

#include "kngroup.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "knstringfilter.h"


KNStringFilter& KNStringFilter::operator=(const KNStringFilter &sf)
{
  con=sf.con;
  data=sf.data.copy();
  enabled=sf.enabled;
  regExp=sf.regExp;
    
  return (*this);
}



bool KNStringFilter::doFilter(const QString &s)
{
  bool ret=true;

  if(enabled) {
    if(regExp) ret=(s.contains(QRegExp(expanded)) > 0);
    else ret=(s.find(expanded,0,false)!=-1);

    if(!con) ret=!ret;

  }

  return ret;
}



// replace placeholders
void KNStringFilter::expand(KNGroup *g)
{
  KNConfig::Identity  *id=0,
                      *defId=knGlobals.cfgManager->identity(),
                      *grpId=g->identity();

  expanded = data.copy();

  ((grpId) && grpId->hasName()) ? id=grpId : id=defId;
  expanded.replace(QRegExp("%MYNAME"), id->name());

  ((grpId) && grpId->hasEmail()) ? id=grpId : id=defId;
  expanded.replace(QRegExp("%MYEMAIL"), id->email());
}



void KNStringFilter::load(KSimpleConfig *conf)
{
  enabled=conf->readBoolEntry("enabled", false);
  con=conf->readBoolEntry("contains", true);
  data=conf->readEntry("Data");
  regExp=conf->readBoolEntry("regX", false);
}



void KNStringFilter::save(KSimpleConfig *conf)
{
  conf->writeEntry("enabled", enabled);
  conf->writeEntry("contains", con);
  conf->writeEntry("Data", data);
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
  
  regExp=new QCheckBox(i18n("regular expression"), this);
  
  QGridLayout *topL=new QGridLayout(this, 3,4, 8,5 );
  topL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  topL->addWidget(enabled,1,0, Qt::AlignHCenter);
  topL->addColSpacing(0, 30);
  topL->addWidget(fType, 1,1);
  topL->addColSpacing(2, 10);
  topL->addWidget(regExp, 1,3);
  topL->addMultiCellWidget(fString, 2,2, 1,3);
  topL->setColStretch(3,1);

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

