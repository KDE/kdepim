/* uiDialog.cc                          KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This defines a subclass of KDialogBase that handles the setup for
** KPilot -- and conduits -- configuration dialogs. It also provides
** some support for the default about-page in setup dialogs, for applications
** (like conduits) with no main window or menu.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>

#include <kaboutapplication.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kiconloader.h>

#include "uiDialog.moc"

UIDialog::UIDialog(QWidget * parent, const char *name,
	bool modal) :
	KDialogBase(parent, name, modal, QString::null,
		KDialogBase::Ok | KDialogBase::Cancel, 
		KDialogBase::Ok, false), 
	fP(0L)
{
	FUNCTIONSETUP;

	fMainWidget = makeHBoxMainWidget();
}

UIDialog::~UIDialog()
{
	FUNCTIONSETUP;
}

void UIDialog::addAboutPage(bool includeabout)
{
	FUNCTIONSETUP;
	ASSERT(tabWidget());

	QWidget *w = new QWidget(tabWidget(), "aboutpage");

	w->resize(tabWidget()->size());

	QString s;
	QLabel *text;
	KIconLoader *l = KGlobal::iconLoader();
	const KAboutData *p = KGlobal::instance()->aboutData();

	QGridLayout *grid = new QGridLayout(w, 4, 4, SPACING);

	grid->addColSpacing(0, SPACING);
	grid->addColSpacing(4, SPACING);


	QPixmap applicationIcon =
		l->loadIcon(KGlobal::instance()->instanceName(),
		KIcon::Desktop,
		0, KIcon::DefaultState, 0L,
		true);

	if (applicationIcon.isNull())
	{
		applicationIcon = l->loadIcon("kpilot", KIcon::Desktop);
	}

	text = new QLabel(w);
	text->setPixmap(applicationIcon);
	text->adjustSize();
	grid->addWidget(text, 0, 1);


	text = new QLabel(w);
	s = QString::null;
	s += p->programName();
	s += ' ';
	s += p->version();
	s += '\n';
	s += p->copyrightStatement();
	text->setText(s);
	grid->addMultiCellWidget(text, 0, 0, 2, 3);

	text = new QLabel(w);
	s = p->shortDescription();
	text->setText(s);
	grid->addMultiCellWidget(text, 1, 1, 2, 3);

	text = new QLabel(w);
	s = "<qt>";
	s += p->homepage();
	s += '\n';
	s += i18n("Send bugs reports to <i>%1</i><br>").arg(p->bugAddress());
	s += i18n
		("Send questions and comments to <i>kde-pim@kde.org</i><br>");
	s += "</qt>";
	text->setText(s);
	grid->addMultiCellWidget(text, 2, 2, 2, 3);

	if (includeabout)
	{
		QPushButton *but = new QPushButton(i18n("More About ..."),
			w);

		connect(but, SIGNAL(clicked()), this, SLOT(showAbout()));
		but->adjustSize();
		grid->addWidget(but, 3, 2);
	}

	grid->setRowStretch(4, 100);
	grid->setColStretch(3, 100);

	tabWidget()->addTab(w, i18n("About"));
}

void UIDialog::setTabWidget(QTabWidget * w)
{
	FUNCTIONSETUP;

	widget()->resize(w->size());
	fP = w;
}

/* slot */ void UIDialog::showAbout()
{
	FUNCTIONSETUP;
	KAboutApplication *kap = new KAboutApplication(this);

	kap->exec();
	// Experience crashes when deleting kap
	//
	//
	// delete kap;
}

/* virtual slot */ void UIDialog::slotOk()
{
	FUNCTIONSETUP;

	commitChanges();
	KDialogBase::slotOk();
}


// $Log$
// Revision 1.2  2001/09/24 19:45:44  adridg
// Minor changes to get the dialog to size correctly when other fonts / settings are used. This solves David's complaint about ugly dialogs.
//
// Revision 1.1  2001/09/23 18:34:12  adridg
// New dialog base class for .ui files
//
