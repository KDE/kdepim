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
** Bug reports and questions can be sent to groot@kde.org
*/
static const char *logw_id="$Id$";

#include "options.h"

#include <qlayout.h>
#include <qtextview.h>
#include <qtooltip.h>
#include <qdatetime.h>

#include "logWidget.moc"

LogWidget::LogWidget(QWidget *parent) :
	PilotComponent(parent,"component_log",QString::null),
	fLog(0L),
	fShowTime(false)
{
	QGridLayout *grid = new QGridLayout(this,3,3,SPACING);
	grid->addRowSpacing(0,SPACING);
	grid->addRowSpacing(2,SPACING);
	grid->addColSpacing(0,SPACING);
	grid->addColSpacing(2,SPACING);
	grid->setRowStretch(1,100);
	grid->setColStretch(1,100);

	fLog = new QTextView(this);
	QToolTip::add(fLog,i18n("This lists all the messages received "
		"during the current HotSync"));
	grid->addWidget(fLog,1,1);

	fLog->setText(i18n("<qt><B>HotSync Log</B></qt>"));
	(void) logw_id;
}

void LogWidget::addMessage(const QString &s)
{
	if (fShowTime)
	{
		QString t;

		t=QTime::currentTime().toString();
		t.append("  ");
		t.append(s);
		fLog->append(t);
	}
	else
	{
		fLog->append(s);
	}
}

void LogWidget::addError(const QString &s)
{
	QString t("<qt><B>");

	if (fShowTime)
	{

		t=QTime::currentTime().toString();
		t.append("  ");
	}

	t.append(s);
	t.append("</B></qt>");

	fLog->append(s);
}

void LogWidget::syncDone()
{
	addMessage(i18n("<b>HotSync Finished!</b>"));
}

void LogWidget::initialize()
{
}


