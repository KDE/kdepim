/*
    knstringfilter.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
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
#include <qregexp3.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <klocale.h>
#include <ksimpleconfig.h>
#include <klineedit.h>

#include "kngroup.h"
#include "knnntpaccount.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "knstringfilter.h"


KNStringFilter& KNStringFilter::operator=(const KNStringFilter &sf)
{
  con=sf.con;
  data=sf.data.copy();
  regExp=sf.regExp;
    
  return (*this);
}



bool KNStringFilter::doFilter(const QString &s)
{
  bool ret=true;

  if(!expanded.isEmpty()) {
    if(regExp) {
      QRegExp3 matcher(expanded);
      ret=(matcher.search(s) > 0);
    } else
      ret=(s.find(expanded,0,false)!=-1);

    if(!con) ret=!ret;
  }

  return ret;
}



// replace placeholders
void KNStringFilter::expand(KNGroup *g)
{
  KNConfig::Identity  *id=0,
                      *defId=knGlobals.cfgManager->identity(),
                      *grpId=g->identity(),
                      *accId=g->account()->identity();

  expanded = data;

  if ((grpId) && grpId->hasName())
    id=grpId;
  else
    id= ((accId) && accId->hasName())? accId:defId;
  expanded.replace(QRegExp("%MYNAME"), id->name());

  if ((grpId) && grpId->hasEmail())
    id=grpId;
  else
    id= ((accId) && accId->hasEmail())? accId:defId;
  expanded.replace(QRegExp("%MYEMAIL"), id->email());
}



void KNStringFilter::load(KSimpleConfig *conf)
{
  con=conf->readBoolEntry("contains", true);
  data=conf->readEntry("Data");
  regExp=conf->readBoolEntry("regX", false);
}



void KNStringFilter::save(KSimpleConfig *conf)
{
  conf->writeEntry("contains", con);
  conf->writeEntry("Data", data);
  conf->writeEntry("regX", regExp);
}


//===============================================================================

KNStringFilterWidget::KNStringFilterWidget(const QString& title, QWidget *parent)
  : QGroupBox(title, parent)
{
  fType=new QComboBox(this);
  fType->insertItem(i18n("does contain"));
  fType->insertItem(i18n("does NOT contain"));
  
  fString=new KLineEdit(this);
  
  regExp=new QCheckBox(i18n("regular expression"), this);
  
  QGridLayout *topL=new QGridLayout(this, 3,3, 8,5 );
  topL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  topL->addWidget(fType, 1,0);
  topL->addColSpacing(1, 10);
  topL->addWidget(regExp, 1,1);
  topL->addMultiCellWidget(fString, 2,2, 0,2);
  topL->setColStretch(2,1);
}



KNStringFilterWidget::~KNStringFilterWidget()
{
}



KNStringFilter KNStringFilterWidget::filter()
{
  KNStringFilter ret;
  ret.con=(fType->currentItem()==0);
  ret.data=fString->text();
  ret.regExp=regExp->isChecked();
  
  return ret;
}



void KNStringFilterWidget::setFilter(KNStringFilter &f)
{
  if(f.con) fType->setCurrentItem(0);
  else fType->setCurrentItem(1);
  fString->setText(f.data);
  regExp->setChecked(f.regExp);
}



void KNStringFilterWidget::clear()
{
  fString->clear();
  fType->setCurrentItem(0);
  regExp->setChecked(false);
}


void KNStringFilterWidget::setStartFocus()
{
  fString->setFocus();
}


// -----------------------------------------------------------------------------+

#include "knstringfilter.moc"

