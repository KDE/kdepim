/*
* dropdlg.cpp -- Implementation of class KDropCfgDialog.
* Author:	Sirtaj Singh Kang
* Author:	Rik Hemsley
* Version:	$Id$
* Generated:	Wed Jul 29 04:02:28 EST 1998
*/

#include"dropdlg.h"

#include"moncfg.h"

#include<assert.h>
#include<qlayout.h>

KDropCfgDialog::KDropCfgDialog(const QString & caption)
	:
	KDialogBase(
	  Tabbed,
	  caption,
	  Ok|Cancel|Apply,
	  Ok,
	  0,
	  "KDropCfgDialog",
	  true,
	  true
	)
{
	// notifier list
	_notifiers.setAutoDelete( true );
	connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
	connect(this, SIGNAL(applyClicked()), this, SLOT(slotApplyClicked()));
}

KDropCfgDialog::~KDropCfgDialog()
{
}

void KDropCfgDialog::addConfigPage( KMonitorCfg *page )
{
	assert( page );

	QWidget * w = addPage(page->name());
	QVBoxLayout * layout = new QVBoxLayout(w);
	QWidget * tab = page->makeWidget(w);
	layout->addWidget(tab);
	tab->show();

	_notifiers.append( page );
}

void KDropCfgDialog::slotApplyClicked()
{
  for (QPtrListIterator<KMonitorCfg> it(_notifiers); it.current(); ++it)
    it.current()->updateConfig();
}

void KDropCfgDialog::slotOkClicked()
{
  slotApplyClicked();
  setResult(Ok);
  emit disassociate( this );
}
/* vim: set noexpandtab tabstop=8 softtabstop=0: */
#include "dropdlg.moc"
