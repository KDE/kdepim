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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
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
	virtual void commit();
	virtual void load();
	virtual bool isModified() const;
protected:
	SysInfoWidget *fConfigWidget;
} ;

#endif
