/***************************************************************************
                          knsettingsdialog.cpp  -  description
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

#include <qlayout.h>
#include <qwidgetstack.h>
#include <qsplitter.h>
#include <klocale.h>
#include <kconfig.h>
#include <kseparator.h>
#include <qheader.h>

#include "knsettingsdialog.h"
#include "knaccnewssettings.h"
#include "knaccmailsettings.h"
#include "knreadgensettings.h"
#include "knreadhdrsettings.h"
#include "knreadappsettings.h"
#include "knfiltersettings.h"
#include "knpostcomsettings.h"
#include "knposttechsettings.h"
#include "kncleanupsettings.h"
#include "knkeysettings.h"
#include "knusersettings.h"
#include "knglobals.h"
#include "utilities.h"

#define ACC_NEWS		0
#define ACC_MAIL		1
#define USER			2
#define READ_GEN		3
#define READ_APP		4
#define READ_HDR		5
#define READ_FIL		6
#define POST_TECH	7
#define POST_COM		8
#define CLEAN			9
#define ACCELS		10


KNSettingsDialog::KNSettingsDialog() : QDialog(xTop,0, true)
{
	lvItem *it, *p;
	split=new QSplitter(this);
	lv=new QListView(split);
	lv->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	lv->addColumn(i18n("Settings"));
	lv->setRootIsDecorated(true);
	lv->setSorting(-1);
	lv->header()->setClickEnabled(false);
	stack=new QWidgetStack(split);
	stack->setFrameStyle(QFrame::Box | QFrame::Sunken);
	okBtn=new QPushButton(i18n("Ok"), this);
	cancelBtn=new QPushButton(i18n("Cancel"), this);
	helpBtn=new QPushButton(i18n("Help"), this);
	KSeparator *sep=new KSeparator(this);
		
	it=new lvItem(lv, i18n("Key bindings"), ACCELS);
	it=new lvItem(lv, i18n("Cleanup"), CLEAN);
	p=new lvItem(lv, i18n("Post News"), -1);
	it=new lvItem(p, i18n("Composer"), POST_COM);
	it=new lvItem(p, i18n("Technical"), POST_TECH);
	p=new lvItem(lv, i18n("Read News"), -1);
	it=new lvItem(p, i18n("Filters"), READ_FIL);
	it=new lvItem(p, i18n("Appearance"), READ_APP);
	it=new lvItem(p, i18n("Headers"), READ_HDR);
	it=new lvItem(p, i18n("General"), READ_GEN);
	it=new lvItem(lv, i18n("User"), USER);
	p=new lvItem(lv, i18n("Accounts"), -1);
	it=new lvItem(p, i18n("Mail"), ACC_MAIL);
	it=new lvItem(p, i18n("News"), ACC_NEWS);
		
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	QHBoxLayout *btnL=new QHBoxLayout(10);
	
	topL->addWidget(split, 1);
	topL->addWidget(sep);
	topL->addLayout(btnL);
	btnL->addWidget(helpBtn);
	btnL->addStretch(1);
	btnL->addWidget(okBtn);
	btnL->addWidget(cancelBtn);
	topL->activate();
	
	connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
	connect(helpBtn, SIGNAL(clicked()), this, SLOT(slotHelpBtnClicked()));
	connect(lv, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(slotLVChanged(QListViewItem*)));
	
	setCaption(i18n("Settings"));
	setDialogSize("settingsDlg", this);
	KConfig *c=CONF();
	c->setGroup("DIALOGS");
	QValueList<int> l=c->readIntListEntry("settingsDlgSplitter");
	split->setSizes(l);
}



KNSettingsDialog::~KNSettingsDialog()
{
	saveDialogSize("settingsDlg", this->size());
	KConfig *c=CONF();
	c->setGroup("DIALOGS");
	c->writeEntry("settingsDlgSplitter", split->sizes());
}



void KNSettingsDialog::apply()
{
	KNSettingsWidget *sw;
	for(int i=0; i<10; i++) {
		sw=(KNSettingsWidget*) (stack->widget(i));
		if(sw) sw->apply();
	}
}



void KNSettingsDialog::slotHelpBtnClicked()
{
}



void KNSettingsDialog::slotLVChanged(QListViewItem *it)
{
	lvItem *i=(lvItem*)it;
	QWidget *w;
	KNSettingsWidget *sw;
	int id;
	
	if(!i) return;
	
	id=i->id;
	if(id==-1) return;
	w=stack->widget(id);
	
	if(!w) {
	
		switch(id) {
			case ACC_NEWS:
				sw=new KNAccNewsSettings(stack, xTop->accManager());
				stack->addWidget(sw, ACC_NEWS);
			break;
			case ACC_MAIL:
				sw=new KNAccMailSettings(stack);
				stack->addWidget(sw, ACC_MAIL);
			break;
			case READ_GEN:
				sw=new KNReadGenSettings(stack);
				stack->addWidget(sw, READ_GEN);
			break;
			case READ_APP:
				sw=new KNReadAppSettings(stack);
				stack->addWidget(sw, READ_APP);
			break;
			case READ_HDR:
				sw=new KNReadHdrSettings(stack);
				stack->addWidget(sw, READ_HDR);
			break;
			case READ_FIL:
				sw=new KNFilterSettings(xTop->fiManager(), stack);
				stack->addWidget(sw, READ_FIL);
			break;
			case USER:
				sw=new KNUserSettings(stack);
				stack->addWidget(sw, USER);
			break;				
			case POST_COM:
				sw=new KNPostComSettings(stack);
				stack->addWidget(sw, POST_COM);
			break;				
			case POST_TECH:
				sw=new KNPostTechSettings(stack);
				stack->addWidget(sw, POST_TECH);
			break;				
			case CLEAN:
				sw=new KNCleanupSettings(stack);
				stack->addWidget(sw, CLEAN);
			break;
			case ACCELS:
				sw=new KNKeySettings(stack);
				stack->addWidget(sw, ACCELS);
			break;
		}
		stack->setMinimumSize(sw->minimumSize());
	}	
	else stack->setMinimumSize(w->minimumSize());
	stack->raiseWidget(id);	
}



//===============================================================================



KNSettingsDialog::lvItem::lvItem(QListView *p, const QString& t, int i)
	: QListViewItem(p, t)
{
	id=i;
}



KNSettingsDialog::lvItem::lvItem(QListViewItem *p, const QString& t, int i)
	: QListViewItem(p, t)
{
	id=i;
}

