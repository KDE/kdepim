#ifndef _SysInfo_SysInfo_SETUP_H
#define _SysInfo_SysInfo_SETUP_H
/* sysinfo-setup.h                       KPilot
**
** Copyright (C) 2003 by Reinhold Kainhofer
**
** This file defines the widget and behavior for the config dialog
** of the KNotes conduit.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "plugin.h"

class SysInfoWidget;

class SysInfoWidgetConfig : public ConduitConfigBase
{
public:
	SysInfoWidgetConfig(QWidget *parent, const char *);
	virtual void commit(KConfig *);
	virtual void load(KConfig *);
	virtual bool isModified();
protected:
	SysInfoWidget *fConfigWidget;
} ;

class SysInfoWidgetSetup : public ConduitConfig
{
Q_OBJECT
public:
	SysInfoWidgetSetup(QWidget *,const char *,const QStringList &);
	virtual ~SysInfoWidgetSetup();

	virtual void readSettings();

protected:
	virtual void commitChanges();

private:
	SysInfoWidgetConfig *fConfigBase;
} ;

#endif
