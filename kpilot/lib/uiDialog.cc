/* KPilot
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
#include <kactivelabel.h>

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

/* static */ QWidget *UIDialog::aboutPage(QWidget *parent, KAboutData *ad)
{
	FUNCTIONSETUP;

	QWidget *w = new QWidget(parent, "aboutpage");

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
		64, KIcon::DefaultState, 0L,
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
	// Experiment with a long non-<qt> string. Use that to find
	// sensible widths for the columns.
	//
	text->setText(i18n("Send questions and comments to kdepim-users@kde.org"));
	text->adjustSize();
#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Text size "
		<< text->size().width()
		<< ","
		<< text->size().height()
		<< endl;
#endif
	int linewidth = text->size().width();
	int lineheight = text->size().height();

	// Use the label to display the applciation icon
	text->setText(QString::null);
	text->setPixmap(applicationIcon);
	text->adjustSize();
	grid->addWidget(text, 0, 1);


	KActiveLabel *linktext = new KActiveLabel(w);
	grid->addRowSpacing(1,QMAX(100,6*lineheight));
	grid->addRowSpacing(2,QMAX(100,6*lineheight));
	grid->addColSpacing(2,SPACING+linewidth/2);
	grid->addColSpacing(3,SPACING+linewidth/2);
	grid->setRowStretch(1,50);
	grid->setRowStretch(2,50);
	grid->setColStretch(2,50);
	grid->setColStretch(3,50);
	linktext->setMinimumSize(linewidth,QMAX(260,60+12*lineheight));
	linktext->setFixedHeight(QMAX(260,60+12*lineheight));
	linktext->setVScrollBarMode(QScrollView::Auto/*AlwaysOn*/);
	text = new QLabel(w);
	grid->addMultiCellWidget(text,0,0,2,3);
	grid->addMultiCellWidget(linktext,1,2,1,3);

	// Now set the program and copyright information.
	s = CSL1("<qt><h3>");
	s += p->programName();
	s += ' ';
	s += p->version();
	s += CSL1("</h3>");
	s += p->copyrightStatement() + CSL1("<br></qt>");
	text->setText(s);

	linktext->append(p->shortDescription() + CSL1("<br>"));

	if (!p->homepage().isEmpty())
	{
		s = QString::null;
		s += CSL1("<a href=\"%1\">").arg(p->homepage());
		s += p->homepage();
		s += CSL1("</a><br>");
		linktext->append(s);
	}

	s = QString::null;
	s += i18n("Send questions and comments to <a href=\"mailto:%1\">%2</a>.")
		.arg( CSL1("kdepim-users@kde.org") )
		.arg( CSL1("kdepim-users@kde.org") );
	s += ' ';
	s += i18n("Send bug reports to <a href=\"mailto:%1\">%2</a>.")
		.arg(p->bugAddress())
		.arg(p->bugAddress());
	s += ' ';
	s += i18n("For trademark information, see the "
		"<a href=\"help:/kpilot/trademarks.html\">KPilot User's Guide</a>.");
	s += CSL1("<br>");
	linktext->append(s);
	linktext->append(QString::null);



	QValueList<KAboutPerson> pl = p->authors();
	QValueList<KAboutPerson>::ConstIterator i;

	s = i18n("<b>Authors:</b> ");

	QString comma = CSL1(", ");

	unsigned int count=1;
	for (i=pl.begin(); i!=pl.end(); ++i)
	{
		s.append(CSL1("%1 (<i>%2</i>)%3")
			.arg((*i).name())
			.arg((*i).task())
			.arg(count<pl.count() ? comma : QString::null)
			);
		count++;
	}
	linktext->append(s);

	s = QString::null;
	pl = p->credits();
	if (pl.count()>0)
	{
		count=1;
		s.append(i18n("<b>Credits:</b> "));
		for (i=pl.begin(); i!=pl.end(); ++i)
		{
			s.append(CSL1("%1 (<i>%2</i>)%3")
				.arg((*i).name())
				.arg((*i).task())
				.arg(count<pl.count() ? comma : QString::null)
				);
			count++;
		}
	}
	linktext->append(s);
	linktext->ensureVisible(0,0);

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

	return w;
}

/* static */ void UIDialog::addAboutPage(QTabWidget *tw,
	KAboutData *ad,
	bool /* aboutbutton */)
{
	FUNCTIONSETUP;

	Q_ASSERT(tw);

	QWidget *w = UIDialog::aboutPage(tw,ad);
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
	tw->adjustSize();
}

void UIDialog::addAboutPage(bool aboutbutton,KAboutData *ad)
{
	FUNCTIONSETUP;
	addAboutPage(tabWidget(),ad,aboutbutton);
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
