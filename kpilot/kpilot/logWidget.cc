/* logWidget.cc                         KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the log window widget, which logs
** sync-messages during a HotSync.
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
** Bug reports and questions can be sent to kde-pim@kde.org.
*/
static const char *logw_id =
	"$Id$";

#include "options.h"

#include <qfile.h>
#include <qlayout.h>
#include <qtextview.h>
#include <qtooltip.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qtimer.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <kprogress.h>

#include "logWidget.moc"

LogWidget::LogWidget(QWidget * parent) :
	PilotComponent(parent, "component_log", QString::null),
	DCOPObject("KPilotIface"),
	fLog(0L), 
	fShowTime(false),
	fSplash(0L),
	fLabel(0L),
	fProgress(0L)
{
	FUNCTIONSETUP;
	QGridLayout *grid = new QGridLayout(this, 4, 4, SPACING);

	grid->addRowSpacing(0, SPACING);
	grid->addRowSpacing(1, 100);
	grid->addColSpacing(2, 100);
	grid->addRowSpacing(3, SPACING);
	grid->addColSpacing(0, SPACING);
	grid->addColSpacing(3, SPACING);
	grid->setRowStretch(1, 50);
	grid->setColStretch(2, 50);

	fLog = new QTextView(this);
	QToolTip::add(fLog, i18n("This lists all the messages received "
			"during the current HotSync"));
	grid->addMultiCellWidget(fLog, 1, 1,1,2);

	fLog->setText(i18n("<qt><B>HotSync Log</B></qt>"));


	fLabel = new QLabel(i18n("Sync Progress:"),this);
	grid->addWidget(fLabel,2,1);
	fProgress = new KProgress(0,100,0,KProgress::Horizontal,this);
	grid->addWidget(fProgress,2,2);


	QString splashPath =
		KGlobal::dirs()->findResource("data",
			"kpilot/kpilot-splash.png");

	if (!splashPath.isEmpty() && QFile::exists(splashPath))
	{
		fLog->hide();
		fLabel->hide();
		fProgress->hide();

		fSplash = new QLabel(this);
		fSplash->setPixmap(QPixmap(splashPath));
		QTimer::singleShot(3000,this,SLOT(hideSplash()));
		grid->addMultiCellWidget(fSplash,1,2,1,2);
	}

	(void) logw_id;
}

void LogWidget::addMessage(const QString & s)
{
	FUNCTIONSETUP;

	if (fShowTime)
	{
		QString t;

		t = QTime::currentTime().toString();
		t.append("  ");
		t.append(s);
		fLog->append(t);
	}
	else
	{
		fLog->append(s);
	}
}

void LogWidget::addError(const QString & s)
{
	FUNCTIONSETUP;

	QString t("<qt><B>");

	if (fShowTime)
	{

		t = QTime::currentTime().toString();
		t.append("  ");
	}

	t.append(s);
	t.append("</B></qt>");

	fLog->append(s);
}

void LogWidget::syncDone()
{
	FUNCTIONSETUP;

	addMessage(i18n("<b>HotSync Finished!</b>"));
}

void LogWidget::initialize()
{
	FUNCTIONSETUP;
}

void LogWidget::hideSplash()
{
	FUNCTIONSETUP;

	if (fSplash)
	{
		fSplash->hide();
		KPILOT_DELETE(fSplash);
	}

	fLog->show();
	fLabel->show();
	fProgress->show();
}


/* DCOP */ ASYNC LogWidget::logMessage(QString s)
{
	FUNCTIONSETUP;

	if (fLog)
	{
		QTime t = QTime::currentTime();
		QString s1 = t.toString();

		s1.append("  ");
		s1.append(s);
		fLog->append(s1);
	}
}

/* DCOP */ ASYNC LogWidget::logProgress(QString s, int i)
{
	FUNCTIONSETUP;

	logMessage(s);

	if ((i >= 0) && (i <= 100))
	{
		fProgress->setValue(i);
	}
}


// $Log: $
