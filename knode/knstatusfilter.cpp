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

#include "knstatusfilter.h"

#include <QCheckBox>
#include <QGridLayout>

#include <kdialog.h>
#include <klocale.h>

#define EN_R  0
#define EN_N  1
#define EN_US 2
#define EN_NS 3

#define DAT_R   4
#define DAT_N   5
#define DAT_US  6
#define DAT_NS  7

using namespace KNode;

KNode::StatusFilter::StatusFilter()
{
  data.fill(false,8);
}



KNode::StatusFilter::~StatusFilter()
{
}



void KNode::StatusFilter::load( const KConfigGroup &group )
{
  data.setBit(EN_R, group.readEntry("EN_R", false));
  data.setBit(DAT_R, group.readEntry("DAT_R", false));

  data.setBit(EN_N, group.readEntry("EN_N", false));
  data.setBit(DAT_N, group.readEntry("DAT_N", false));

  data.setBit(EN_US, group.readEntry("EN_US", false));
  data.setBit(DAT_US, group.readEntry("DAT_US", false));

  data.setBit(EN_NS, group.readEntry("EN_NS", false));
  data.setBit(DAT_NS, group.readEntry("DAT_NS", false));

}



void KNode::StatusFilter::save( KConfigGroup &group )
{
  group.writeEntry("EN_R", data.at(EN_R));
  group.writeEntry("DAT_R", data.at(DAT_R));

  group.writeEntry("EN_N", data.at(EN_N));
  group.writeEntry("DAT_N", data.at(DAT_N));

  group.writeEntry("EN_US", data.at(EN_US));
  group.writeEntry("DAT_US", data.at(DAT_US));

  group.writeEntry("EN_NS", data.at(EN_NS));
  group.writeEntry("DAT_NS", data.at(DAT_NS));
}



bool KNode::StatusFilter::doFilter( KNRemoteArticle::Ptr a )
{
  bool ret=true;

  if(data.at(EN_R) && ret)
    ret=(a->isRead() == data.at(DAT_R));

  if(data.at(EN_N) && ret)
    ret=(a->isNew() == data.at(DAT_N));

  if(data.at(EN_US) && ret)
    ret=(a->hasUnreadFollowUps() == data.at(DAT_US));

  if(data.at(EN_NS) && ret)
    ret=(a->hasNewFollowUps() == data.at(DAT_NS));

  return ret;
}



//==============================================================================

KNode::StatusFilterWidget::StatusFilterWidget( QWidget *parent ) :
  QWidget( parent )
{
  enR=new QCheckBox(i18n("Is read:"), this);
  enN=new QCheckBox(i18n("Is new:"), this);
  enUS=new QCheckBox(i18n("Has unread followups:"), this);
  enNS=new QCheckBox(i18n("Has new followups:"), this);

  rCom=new TFCombo(this);
  nCom=new TFCombo(this);
  usCom=new TFCombo(this);
  nsCom=new TFCombo(this);

  QGridLayout *topL = new QGridLayout( this );
  topL->setSpacing( KDialog::spacingHint() );
  topL->addWidget(enR,0,0); topL->addWidget(rCom,0,1);
  topL->addWidget(enN,1,0); topL->addWidget(nCom,1,1);
  topL->addWidget(enUS,2,0); topL->addWidget(usCom,2,1);
  topL->addWidget(enNS,3,0); topL->addWidget(nsCom,3,1);
  topL->setColumnStretch(2,1);
  topL->setRowStretch(4,1);

  connect( enR, SIGNAL(toggled(bool)), SLOT(slotEnabled()) );
  connect( enN, SIGNAL(toggled(bool)), SLOT(slotEnabled()) );
  connect( enUS, SIGNAL(toggled(bool)), SLOT(slotEnabled()) );
  connect( enNS, SIGNAL(toggled(bool)), SLOT(slotEnabled()) );
}



KNode::StatusFilterWidget::~StatusFilterWidget()
{
}



StatusFilter KNode::StatusFilterWidget::filter()
{
  StatusFilter f;

  f.data.setBit(EN_R, enR->isChecked());
  f.data.setBit(DAT_R, rCom->value());

  f.data.setBit(EN_N, enN->isChecked());
  f.data.setBit(DAT_N, nCom->value());

  f.data.setBit(EN_US, enUS->isChecked());
  f.data.setBit(DAT_US, usCom->value());

  f.data.setBit(EN_NS, enNS->isChecked());
  f.data.setBit(DAT_NS, nsCom->value());

  return f;
}



void KNode::StatusFilterWidget::setFilter( StatusFilter &f )
{
  enR->setChecked(f.data.at(EN_R));
  rCom->setValue(f.data.at(DAT_R));

  enN->setChecked(f.data.at(EN_N));
  nCom->setValue(f.data.at(DAT_N));

  enUS->setChecked(f.data.at(EN_US));
  usCom->setValue(f.data.at(DAT_US));

  enNS->setChecked(f.data.at(EN_NS));
  nsCom->setValue(f.data.at(DAT_NS));

  slotEnabled();
}


void KNode::StatusFilterWidget::clear()
{
  enR->setChecked(false);
  enN->setChecked(false);
  enUS->setChecked(false);
  enNS->setChecked(false);
  rCom->setValue(true);
  nCom->setValue(true);
  nsCom->setValue(true);
  usCom->setValue(true);

  slotEnabled();
}



void KNode::StatusFilterWidget::slotEnabled()
{
  rCom->setEnabled( enR->isChecked() );
  nCom->setEnabled( enN->isChecked() );
  usCom->setEnabled( enUS->isChecked() );
  nsCom->setEnabled( enNS->isChecked() );
}


//==============================================================================


KNode::StatusFilterWidget::TFCombo::TFCombo( QWidget *parent ) : QComboBox( parent )
{
  addItem(i18n("True"));
  addItem(i18n("False"));
}



KNode::StatusFilterWidget::TFCombo::~TFCombo()
{
}



//--------------------------------

