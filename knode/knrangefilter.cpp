/*
    knrangefilter.cpp

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

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <ksimpleconfig.h>
#include <knuminput.h>

#include "knrangefilter.h"


bool KNRangeFilter::doFilter(int a)
{
  bool ret=true;
  if(enabled) {
    switch (op1) {
      case gt:
      case gtoeq:
          if (op2 != dis)
            ret=( matchesOp(val1,op1,a) && matchesOp(a,op2,val2) );
          else
            ret = matchesOp(val1,op1,a);;
          break;
      case eq:
      case lt:
      case ltoeq:
          ret = matchesOp(val1,op1,a);
          break;
      default:
          ret = false;
    }
  }

  return ret;
}



bool KNRangeFilter::matchesOp(int v1, Op o, int v2)
{
  bool ret=false;

  switch(o) {
    case eq:      ret=(v1==v2);   break;
    case gt:      ret=(v1<v2);    break;
    case gtoeq:   ret=(v1<=v2);   break;
    case ltoeq:   ret=(v1>=v2);   break;
    case lt:      ret=(v1>v2);    break;
    default:      ret=false;      break;
  };

  return ret;
}



void KNRangeFilter::load(KSimpleConfig *conf)
{
  enabled=conf->readBoolEntry("enabled", false);
  val1=conf->readNumEntry("val1",0);
  op1=(Op) conf->readNumEntry("op1",0);
  val2=conf->readNumEntry("val2",0);
  op2=(Op) conf->readNumEntry("op2",0);
}



void KNRangeFilter::save(KSimpleConfig *conf)
{
  conf->writeEntry("enabled", enabled);
  conf->writeEntry("val1", val1);
  conf->writeEntry("op1", (int)op1);
  conf->writeEntry("op2", (int)op2);
  conf->writeEntry("val2", val2);
}




//=====================================================================================
//=====================================================================================

KNRangeFilterWidget::KNRangeFilterWidget(const QString& value, int min, int max, QWidget* parent, const QString &unit)
  : QGroupBox(value, parent)
{
  enabled=new QCheckBox(this);

  val1=new KIntSpinBox(min, max, 1, min, 10, this);
  val1->setSuffix(unit);
  val2=new KIntSpinBox(min, max, 1, min, 10, this);
  val2->setSuffix(unit);

  op1=new QComboBox(this);
  op1->insertItem("<");
  op1->insertItem("<=");
  op1->insertItem("=");
  op1->insertItem(">=");
  op1->insertItem(">");
  op2=new QComboBox(this);
  op2->insertItem("");
  op2->insertItem("<");
  op2->insertItem("<=");

  des=new QLabel(value, this);
  des->setAlignment(AlignCenter);

  QGridLayout *topL=new QGridLayout(this, 2,6, 8,5 );

  topL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  topL->addWidget(enabled,1,0, Qt::AlignHCenter);
  topL->addColSpacing(0, 30);
  topL->addWidget(val1,1,1);
  topL->addWidget(op1,1,2);
  topL->addWidget(des,1,3);
  topL->addColSpacing(3, 45);
  topL->addWidget(op2,1,4);
  topL->addWidget(val2,1,5);
  topL->setColStretch(1,1);
  topL->setColStretch(5,1);

  connect(op1, SIGNAL(activated(int)), SLOT(slotOp1Changed(int)));
  connect(op2, SIGNAL(activated(int)), SLOT(slotOp2Changed(int)));
  connect(enabled, SIGNAL(toggled(bool)), SLOT(slotEnabled(bool)));

  slotEnabled(false);
}



KNRangeFilterWidget::~KNRangeFilterWidget()
{
}



KNRangeFilter KNRangeFilterWidget::filter()
{
  KNRangeFilter r;
  r.val1=val1->value();
  r.val2=val2->value();

  r.op1=(KNRangeFilter::Op) op1->currentItem();
  if (op2->currentText().isEmpty())
    r.op2=KNRangeFilter::dis;
  else if (op2->currentText()=="<")
    r.op2=KNRangeFilter::gt;
  else if (op2->currentText()=="<=")
    r.op2=KNRangeFilter::gtoeq;

  r.enabled=enabled->isChecked();

  return r;
}



void KNRangeFilterWidget::setFilter(KNRangeFilter &f)
{
  val1->setValue(f.val1);
  val2->setValue(f.val2);

  op1->setCurrentItem((int)f.op1);
  if (f.op2 == KNRangeFilter::dis)
    op2->setCurrentItem(0);
  else if (f.op2 == KNRangeFilter::gt)
    op2->setCurrentItem(1);
  else if (f.op2 == KNRangeFilter::gtoeq)
    op2->setCurrentItem(2);

  enabled->setChecked(f.enabled);
}



void KNRangeFilterWidget::clear()
{
  val1->setValue(val1->minValue());
  val2->setValue(val2->minValue());
  enabled->setChecked(false);
}



void KNRangeFilterWidget::slotOp1Changed(int id)
{
    bool state = (op1->isEnabled() && (id<2));
  op2->setEnabled(state);
  des->setEnabled(state);
  slotOp2Changed(op2->currentItem());
}



void KNRangeFilterWidget::slotOp2Changed(int id)
{
  val2->setEnabled(op1->isEnabled() && (op1->currentItem()<2) && (id>0));
}



void KNRangeFilterWidget::slotEnabled(bool e)
{
  op1->setEnabled(e);
  val1->setEnabled(e);
  des->setEnabled(e);
  slotOp1Changed(op1->currentItem());
}

// -----------------------------------------------------------------------------

#include "knrangefilter.moc"

