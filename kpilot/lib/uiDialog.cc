/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2004 by Adriaan de Groot
**
** This class defines a way to add an "about widget" to a tab widget.
**
** None of Dan's original code is left.
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3ValueList>
#include <QGridLayout>

#include <kaboutapplication.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kiconloader.h>
#include <kactivelabel.h>

#include "uiDialog.h"

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
		K3Icon::Desktop,
		64, K3Icon::DefaultState, 0L,
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
			K3Icon::Desktop);
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
	grid->addRowSpacing(1,qMax(100,6*lineheight));
	grid->addRowSpacing(2,qMax(100,6*lineheight));
	grid->addColSpacing(2,SPACING+linewidth/2);
	grid->addColSpacing(3,SPACING+linewidth/2);
	grid->setRowStretch(1,50);
	grid->setRowStretch(2,50);
	grid->setColStretch(2,50);
	grid->setColStretch(3,50);
	linktext->setMinimumSize(linewidth,qMax(260,60+12*lineheight));
	linktext->setFixedHeight(qMax(260,60+12*lineheight));
	linktext->setVScrollBarMode(Q3ScrollView::Auto/*AlwaysOn*/);
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
	s += i18n("Send questions and comments to <a href=\"mailto:%1\">%2</a>.",
		  CSL1("kdepim-users@kde.org") ,
		  CSL1("kdepim-users@kde.org") );
	s += ' ';
	s += i18n("Send bug reports to <a href=\"mailto:%1\">%2</a>.",
		 p->bugAddress(),
		 p->bugAddress());
	s += ' ';
	s += i18n("For trademark information, see the "
		"<a href=\"help:/kpilot/trademarks.html\">KPilot User's Guide</a>.");
	s += CSL1("<br>");
	linktext->append(s);
	linktext->append(QString::null);



	Q3ValueList<KAboutPerson> pl = p->authors();
	Q3ValueList<KAboutPerson>::ConstIterator i;

	s = i18n("<b>Authors:</b> ");

	QString comma = CSL1(", ");

	int count=1;
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

