#ifndef _KPILOT_JPilotProxy_SETUP_H
#define _KPILOT_JPilotProxy_SETUP_H
/* JPilotProxy-setup.h                         KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the class for the behavior of the setup dialog.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "plugin.h"
#include "jplugin.h"
#include "ConduitDialog.h"

class JPilotProxyWidgetSetup : public ConduitConfig {
Q_OBJECT
public:
	JPilotProxyWidgetSetup(QWidget *,const char *,const QStringList &);
	virtual ~JPilotProxyWidgetSetup();
	JPlugin*findPlugin(QString fn);

protected:
	virtual void readSettings();
	virtual void commitChanges();
	virtual const QString getSettingsGroup() { return QString("JPilotPluginProxy");};
	virtual bool addConduit(QString file, bool on);

protected slots:
	virtual void slotOk();
	virtual void slotApply();
	
	virtual void slotAddConduit();
	virtual void slotConfigureConduit();
	virtual void slotConfigureConduit(QListViewItem*item);

	virtual void slotSelectPluginPath();
	virtual void slotUpdatePluginPath(const QString &newpath);
	virtual void slotBrowse();
	virtual void slotAddPluginPath();
	virtual void slotRemovePluginPath();
	

private:
	JPilotProxyWidget *fConfigWidget;
	bool updatePluginPathSel;
} ;

#endif
