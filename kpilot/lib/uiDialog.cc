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
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qtextview.h>
#include <qpushbutton.h>

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

UIDialog::UIDialog(QWidget *parent, const char *name,
	int buttonmask, bool modal) :
	KDialogBase(parent,name,modal,QString::null,
		buttonmask | KDialogBase::Ok,
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

/* static */ QPushButton *UIDialog::addAboutPage(QTabWidget *tw,
	KAboutData *ad,
	bool aboutbutton)
{
	FUNCTIONSETUP;
	
	Q_ASSERT(tw);

	QWidget *w = new QWidget(tw, "aboutpage");
	QPushButton *but = 0L;

	QString s;
	QLabel *text;
	KIconLoader *l = KGlobal::iconLoader();
	const KAboutData *p = ad ? ad : KGlobal::instance()->aboutData();

	QGridLayout *grid = new QGridLayout(w, 5, 4, SPACING);

	grid->addColSpacing(0, SPACING);
	grid->addColSpacing(4, SPACING);


#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Looking for icon for "
		<< p->appName()
		<< endl;
#endif

	QPixmap applicationIcon =
		l->loadIcon(QString::fromLatin1(p->appName()),
		KIcon::Desktop,
		0, KIcon::DefaultState, 0L,
		true);

	if (applicationIcon.isNull())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Looking for icon for "
			<< "kpilot"
			<< endl;
#endif
		applicationIcon = l->loadIcon(QString::fromLatin1("kpilot"),
			KIcon::Desktop);
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
	s = CSL1("<qt>") + p->shortDescription() + CSL1("</qt>");
	text->setText(s);
	grid->addMultiCellWidget(text, 1, 1, 2, 3);

	text = new QLabel(w);
	// Experiment with a long non-<qt> string. Use that to find
	// sensible widths for the columns.
	//
	text->setText(i18n("Send questions and comments to kde-pim@kde.org"));
	text->adjustSize();
#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Text size "
		<< text->size().width()
		<< ","
		<< text->size().height()
		<< endl;
#endif
	grid->addColSpacing(2,SPACING+text->size().width()/2);
	grid->addColSpacing(3,SPACING+text->size().width()/2);

	s = "<qt>";
	if (!p->homepage().isEmpty())
	{
		s += p->homepage();
		s += CSL1("<br>");
	}
	s += i18n("Send questions and comments to <i>kde-pim@kde.org</i>");
	s += CSL1("<br>");
	s += i18n("Send bug reports to <i>%1</i>").arg(p->bugAddress());
	s += CSL1("</qt>");

	text->setText(s);
	grid->addMultiCellWidget(text, 2, 2, 2, 3);



	if (aboutbutton)
	{
		but = new QPushButton(i18n("More About"),w);

		but->adjustSize();
		grid->addWidget(but, 4, 2);
		grid->setRowStretch(3, 100);
	}
	else
	{
		QValueList<KAboutPerson> l = p->authors();
		QValueList<KAboutPerson>::ConstIterator i;
		s = i18n("<qt><b>Authors:</b> ");

		QString comma = CSL1(", ");

		unsigned int count=1;
		for (i=l.begin(); i!=l.end(); ++i)
		{
			s.append(CSL1("%1 (<i>%2</i>)%3")
				.arg((*i).name())
				.arg((*i).task())
				.arg(count<l.count() ? comma : QString::null)
				);
			count++;
		}

		l = p->credits();
		if (l.count()>0)
		{
			count=1;
			s.append(i18n("<br><b>Credits:</b> "));
			for (i=l.begin(); i!=l.end(); ++i)
			{
				s.append(CSL1("%1 (<i>%2</i>)%3")
					.arg((*i).name())
					.arg((*i).task())
					.arg(count<l.count() ? comma : QString::null)
					);
				count++;
			}
		}


		s.append(CSL1("</qt>"));

		text = new QLabel(w);
		text->setText(s);
		text->adjustSize();

		grid->addMultiCellWidget(text,4,4,2,3);

		grid->setRowStretch(4,100);
		grid->addRowSpacing(5,SPACING);
	}


	grid->setColStretch(3, 100);

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Size "
		<< w->size().width()
		<< ","
		<< w->size().height()
		<< endl;
#endif

	w->adjustSize();
#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Adjusted size "
		<< w->size().width()
		<< ","
		<< w->size().height()
		<< endl;
#endif

	QSize sz = w->size();

	if (sz.width() < tw->size().width())
	{
		sz.setWidth(tw->size().width());
	}
	if (sz.height() < tw->size().height())
	{
		sz.setHeight(tw->size().height());
	}

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Final size "
		<< sz.width()
		<< ","
		<< sz.height()
		<< endl;
#endif

	tw->resize(sz);
	tw->addTab(w, i18n("About"));
	return but;
}

void UIDialog::addAboutPage(bool aboutbutton,KAboutData *ad)
{
	FUNCTIONSETUP;
	QPushButton *but = addAboutPage(tabWidget(),ad,aboutbutton);
	if (but)
	{
		connect(but, SIGNAL(clicked()), this, SLOT(showAbout()));
	}
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

	if (validate())
	{
		commitChanges();
		KDialogBase::slotOk();
	}
}
