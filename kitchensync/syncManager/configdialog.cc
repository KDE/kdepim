/* configdialog.cc              KitchenSync
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

#include <config.h>
#include "../lib/debug.h"

#include <qvbox.h>
#include <qcheckbox.h>

#include "logWindow.h"
#include "configdialog.h"

Config::Config() :
	KConfig("kitchensync",false,false,"config")
{
}

bool Config::logWindowShown() const
{
	return readBoolEntry("LogWindowShown",true);
}

void Config::setLogWindowShown(bool b)
{
	writeEntry("LogWindowShown",b);
}

int Config::logWindowParts() const
{
	return readNumEntry("LogWindowParts",7);
}

void Config::setLogWindowParts(int s)
{
	writeEntry("LogWindowParts",s);
}

QRect Config::logWindowPos() const
{
	return readRectEntry("LogWindowPos",0);
}

void Config::setLogWindowPos(const QRect &p)
{
	writeEntry("LogWindowPos",p);
}

bool Config::logWindowTime() const
{
	return readBoolEntry("LogWindowTime",false);
}

void Config::setLogWindowTime(bool t)
{
	writeEntry("LogWindowTime",t);
}

ConfigDialog::ConfigDialog(QWidget *parent, Config *c) :
	KDialogBase(parent,"configdialog",false,
		i18n("Configuration"),
		Ok|Cancel,
		Ok),
	fProgress(0),
	fMessage(0),
	fLog(0)
{
	FUNCTIONSETUP;

	QVBox *v = makeVBoxMainWidget();

	fProgress = new QCheckBox(i18n("Show Progress bar"),v);
	fMessage = new QCheckBox(i18n("Show Sync Message"),v);
	fLog = new QCheckBox(i18n("Show Sync Log"),v);
	fTime = new QCheckBox(i18n("Show time in log messages"),v);

	int p = c->logWindowParts();
	fProgress->setChecked(p & LogWindow::Progress);
	fMessage->setChecked(p & LogWindow::Message);
	fLog->setChecked(p & LogWindow::Log);

	fTime->setChecked(c->logWindowTime());
}

void ConfigDialog::save(Config *c)
{
	int p = 0;
	if (fProgress->isChecked()) p |= LogWindow::Progress;
	if (fMessage->isChecked()) p |= LogWindow::Message;
	if (fLog->isChecked()) p |= LogWindow::Log;

	c->setLogWindowParts(p);

	c->setLogWindowTime(fTime->isChecked());
}
