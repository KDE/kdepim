/* logWindow.cc                 KitchenSync
**
** Copyright (C) 2001 by Adriaan de Groot
**
** This program is part of KitchenSync, the KDE handheld-device
** synchronisation framework.
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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#include "config.h"
#include "lib/debug.h"

#include <qlabel.h>
#include <qmultilineedit.h>
#include <qvbox.h>
#include <qdatetime.h>

#include <kprogress.h>
#include <kconfig.h>
#include <kapp.h>

#include "configdialog.h"

#include "logWindow.moc"

LogWindow::LogWindow(QWidget *parent, int p) : 
	KDialogBase(parent,"logWindow",false,
		i18n("Sync Status"),0),
	fProgress(0L),
	fText(0L),
	fLog(0L)
{
	FUNCTIONSETUP;

	createParts();
	showParts(p);
}

LogWindow::LogWindow(QWidget *parent, Config *c) : 
	KDialogBase(parent,"logWindow",false,QString::null,0),
	fProgress(0L),
	fText(0L),
	fLog(0L),
	fAddTime(false)
{
	FUNCTIONSETUP;


	if (c)
	{
		QRect p = c->logWindowPos();
		move(p.topLeft());
		resize(p.size());
	}

	createParts();
	configChanged(c);
}


void LogWindow::createParts()
{
	FUNCTIONSETUP;

	QVBox *v = makeVBoxMainWidget();

	fProgress = new KProgress(0,100,0,KProgress::Horizontal,v);
	fText = new QLabel(v);
	fLog = new QMultiLineEdit(v);

	fText->setAlignment(QLabel::AlignCenter | 
		QLabel::AlignVCenter | 
		QLabel::ExpandTabs);
	fText->setText(i18n("Waiting for Daemon..."));
	fLog->setReadOnly(true);

	v->adjustSize();
}

void LogWindow::showParts(int p)
{
	FUNCTIONSETUP;

	if (p & Progress)
	{
		fProgress->show();
	}
	else
	{
		fProgress->hide();
	}

	if (p & Message)
	{
		fText->show();
	}
	else
	{
		fText->hide();
	}

	if (p & Log)
	{
		fLog->show();
	}
	else
	{
		fLog->hide();
	}

	mainWidget()->adjustSize();
	adjustSize();

	kapp->processEvents();

	mainWidget()->adjustSize();
	adjustSize();
}

void LogWindow::setValue(int i)
{
	fProgress->setValue(i);
}

void LogWindow::setMessage(const QString &s)
{
	fText->setText(s);
}

void LogWindow::appendLog(const QString &s)
{
	if (fAddTime)
	{
		QTime t = QTime::currentTime();
		QString s1 = t.toString();
		s1.append("  ");
		s1.append(s);

		fLog->append(s1);
	}
	else
	{
		fLog->append(s);
	}
}

void LogWindow::showEvent(QShowEvent *e)
{
	FUNCTIONSETUP;

	KDialogBase::showEvent(e);
	emit shown();
}


void LogWindow::saveYourself(Config *c)
{
	FUNCTIONSETUP;

	c->setLogWindowShown(!(isHidden() || isMinimized()));
	c->setLogWindowPos(geometry());
	
	int s = 0;
	if (!fLog->isHidden()) s |= Log;
	if (!fProgress->isHidden()) s |= Progress;
	if (!fText->isHidden()) s |= Message;

	c->setLogWindowParts(s);
}

void LogWindow::configChanged(Config *c)
{
	if (!c) return;

	showParts(c->logWindowParts());
	fAddTime = c->logWindowTime();
}

// $Log:$
