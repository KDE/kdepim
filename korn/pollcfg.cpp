/*
* pollcfg.cpp -- Implementation of class KPollCfg.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Mon Aug  3 01:51:07 EST 1998
*/

#include <assert.h>
#include <stdlib.h>

#include <klocale.h>
#include <kdialog.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qlayout.h>

//#include "typolayout.h"
#include "pollcfg.h"
#include "polldrop.h"

KPollCfg::KPollCfg( KPollableDrop *drop ) 
	: KMonitorCfg(drop)
{
}

QString KPollCfg::name() const
{
	return i18n( "&Poll" );
}

QWidget *KPollCfg::makeWidget(QWidget * parent)
{
	KPollableDrop * d = dynamic_cast<KPollableDrop *>(drop());
  assert(0 != d);

	QWidget *dlg = new QWidget(parent);
	
  _freq = new QSpinBox(1, 7200, 1, dlg);
  _freq->setValue(d->freq());

  QVBoxLayout * topLayout =
    new QVBoxLayout(dlg, KDialog::marginHint(), KDialog::spacingHint());

  QHBoxLayout * l = new QHBoxLayout(topLayout);

  topLayout->addStretch(1);

	l->addWidget(new QLabel(i18n("Check frequency (sec):"), dlg));
  l->addWidget(_freq);

	return dlg;
}

void KPollCfg::updateConfig()
{
	KPollableDrop * d = dynamic_cast<KPollableDrop *>(drop());
  assert(0 != d);
  d->setFreq(_freq->value());
}
