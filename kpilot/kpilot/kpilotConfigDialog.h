#ifndef _KPILOT_KPILOTCONFIGDIALOG_H
#define _KPILOT_KPILOTCONFIGDIALOG_H
/* kpilotConfigDialog.h                 KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines a specialization of KPilotDeviceLink
** that can actually handle some HotSync tasks, like backup
** and restore. It does NOT do conduit stuff.
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


class KPilotConfigWidget;
class SyncConfigWidget;
class DeviceConfigWidget;

class DeviceConfigPage : public ConduitConfigBase
{
Q_OBJECT
public:
	DeviceConfigPage( QWidget *, const char * );

protected:
	virtual bool validate();
	virtual void load();
	virtual void commit();

protected slots:
	void changePortType(int);

private:
	DeviceConfigWidget *fConfigWidget;

	// Read and write the values of Encoding
	// and EncodingDD.
	void getEncoding();
	void setEncoding();
} ;


class SyncConfigPage : public ConduitConfigBase
{
Q_OBJECT
public:
	SyncConfigPage( QWidget *, const char * );

protected:
	virtual bool validate();
	virtual void load();
	virtual void commit();

protected slots:
	void slotSelectNoBackupDBs();
	void slotSelectNoRestoreDBs();

private:
	SyncConfigWidget *fConfigWidget;
} ;

class KPilotConfigPage : public ConduitConfigBase
{
Q_OBJECT
public:
	KPilotConfigPage( QWidget *, const char * );

protected:
	virtual bool validate();
	virtual void load();
	virtual void commit();

private:
	KPilotConfigWidget *fConfigWidget;
} ;

#endif
