/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <qcombobox.h>
#include <qcheckbox.h>
#include <QGridLayout>

#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <ksimpleconfig.h>

#include "kngroup.h"
#include "knnntpaccount.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "knstringfilter.h"

using namespace KNode;

StringFilter& KNode::StringFilter::operator=( const StringFilter &sf )
{
  con=sf.con;
  data=sf.data.copy();
  regExp=sf.regExp;

  return (*this);
}



bool KNode::StringFilter::doFilter( const QString &s )
{
  bool ret=true;

  if(!expanded.isEmpty()) {
    if(regExp) {
      QRegExp matcher(expanded);
      ret = ( matcher.search(s) >= 0 );
    } else
      ret=(s.find(expanded,0,false)!=-1);

    if(!con) ret=!ret;
  }

  return ret;
}



// replace placeholders
void KNode::StringFilter::expand( KNGroup *g )
{
  Identity *id = (g) ? g->identity() : 0;

  if (!id) {
    id = (g) ? g->account()->identity() : 0;
    if (!id)
      id = knGlobals.configManager()->identity();
  }

  expanded = data;
  expanded.replace(QRegExp("%MYNAME"), id->name());
  expanded.replace(QRegExp("%MYEMAIL"), id->email());
}



void KNode::StringFilter::load( KSimpleConfig *conf )
{
  con=conf->readEntry("contains", true);
  data=conf->readEntry("Data");
  regExp=conf->readEntry("regX", false);
}



void KNode::StringFilter::save( KSimpleConfig *conf )
{
  conf->writeEntry("contains", con);
  conf->writeEntry("Data", data);
  conf->writeEntry("regX", regExp);
}


//===============================================================================

KNode::StringFilterWidget::StringFilterWidget( const QString& title, QWidget *parent )
  : QGroupBox( title, parent )
{
  fType=new QComboBox(this);
  fType->insertItem(i18n("Does Contain"));
  fType->insertItem(i18n("Does NOT Contain"));

  fString=new KLineEdit(this);

  regExp=new QCheckBox(i18n("Regular expression"), this);

  QGridLayout *topL = new QGridLayout( this );
  topL->setSpacing( KDialog::spacingHint() );
  topL->addWidget( fType, 0, 0 );
  topL->addWidget( regExp, 0, 1 );
  topL->addWidget( fString, 1, 0, 1, 2 );
  topL->setColumnStretch( 2, 1 );
}



KNode::StringFilterWidget::~StringFilterWidget()
{
}



KNode::StringFilter StringFilterWidget::filter()
{
  StringFilter ret;
  ret.con=(fType->currentItem()==0);
  ret.data=fString->text();
  ret.regExp=regExp->isChecked();

  return ret;
}



void KNode::StringFilterWidget::setFilter( StringFilter &f )
{
  if(f.con) fType->setCurrentItem(0);
  else fType->setCurrentItem(1);
  fString->setText(f.data);
  regExp->setChecked(f.regExp);
}



void KNode::StringFilterWidget::clear()
{
  fString->clear();
  fType->setCurrentItem(0);
  regExp->setChecked(false);
}


void KNode::StringFilterWidget::setStartFocus()
{
  fString->setFocus();
}


// -----------------------------------------------------------------------------+

#include "knstringfilter.moc"
