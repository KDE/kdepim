#ifndef _KPILOT_JPilotProxy_SETUP_H
#define _KPILOT_JPilotProxy_SETUP_H
/* JPilotProxy-setup.h                         KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
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

// $Log$
// Revision 1.1  2002/04/07 11:17:54  kainhofe
// First Version of the JPilotPlugin Proxy conduit. it can be activated, but loading a plugin or syncing a plugin crashes the palm (if no plugin is explicitely enabled, this conduit can be enabled and it won't crash KPIlot). A lot of work needs to be done, see the TODO
//
// Revision 1.4  2002/04/07 00:53:36  reinhold
// Loading plugins works, callbacks are resolved, dependencies on libplugin are not and crash the palm
//
// Revision 1.3  2002/04/07 00:10:49  reinhold
// Settings are now saved
//
// Revision 1.2  2002/03/20 01:27:29  reinhold
// The plugin's setup dialog now runs without crashes. loading JPilot plugins still fails because of inresolved dependencies like jp_init or gtk_... functions.
//
// Revision 1.1  2002/03/18 23:15:55  reinhold
// Plugin compiles now
//
// Revision 1.5  2002/03/15 20:43:17  reinhold
// Fixed the crash on loading (member function not defined)...
//
// Revision 1.4  2002/03/13 22:14:40  reinhold
// GUI should work now...
//
// Revision 1.3  2002/03/10 23:58:32  reinhold
// Made the conduit compile...
//
// Revision 1.2  2002/03/10 16:06:43  reinhold
// Cleaned up the class hierarchy, implemented some more features (should be quite finished now...)
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the generic project manager / List manager conduit.
//
//
//

#endif
