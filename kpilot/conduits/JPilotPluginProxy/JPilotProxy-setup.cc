/* JPilotProxy-setup.cc                        KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the JPilotProxy-conduit plugin.
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

#include "options.h"

#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <kconfig.h>
#include <klistview.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include "JPilotProxy-conduit.h"
#include "JPilotProxy-factory.h"
#include "JPilotProxy-setup.moc"



JPilotProxyWidgetSetup::JPilotProxyWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) : ConduitConfig(w,n,a) {
	FUNCTIONSETUP;

	fConfigWidget = new JPilotProxyWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false, JPilotProxyConduitFactory::fAbout);
	

	QObject::connect(fConfigWidget->ListPlugins, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(slotConfigureConduit(QListViewItem*)));
//	QObject::connect(fConfigWidget->ListPlugins, SIGNAL(doubleClicked()), this, SLOT(slotConfigureConduit()));
	QObject::connect(fConfigWidget->PushAddPlugin,SIGNAL(clicked()), this, SLOT(slotAddConduit()));
	QObject::connect(fConfigWidget->PushConfigure,SIGNAL(clicked()), this, SLOT(slotConfigureConduit()));
	
//	QObject::connect(fConfigWidget->ListPluginPathes, SIGNAL(selected(QListBoxItem*)), this, SLOT(slotBrowse(QListBoxItem*)));
//	QObject::connect(fConfigWidget->ListPluginPathes, SIGNAL(selectionChanged(QListBoxItem*)), this, SLOT(slotSelectPluginPath(QListBoxItem*)));
	QObject::connect(fConfigWidget->ListPluginPathes, SIGNAL(selectionChanged()), this, SLOT(slotSelectPluginPath()));
	QObject::connect(fConfigWidget->DirEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdatePluginPath(const QString&)));
	QObject::connect(fConfigWidget->BrowseButton, SIGNAL(clicked()), this, SLOT(slotBrowse()));
	
	QObject::connect(fConfigWidget->AddButton, SIGNAL(clicked()), this, SLOT(slotAddPluginPath()));
	QObject::connect(fConfigWidget->RemoveButton, SIGNAL(clicked()), this, SLOT(slotRemovePluginPath()));
	fConfigWidget->SearchPluginsButton->setEnabled(false);
//	QObject::connect(fConfigWidget->SearchPluginsButton, SIGNAL(clicked()), this, SLOT(slotScanPluginPathes()));
	updatePluginPathSel=true;
}

JPilotProxyWidgetSetup::~JPilotProxyWidgetSetup() {
	FUNCTIONSETUP;
}

void JPilotProxyWidgetSetup::slotOk() {
	FUNCTIONSETUP;
	commitChanges();
	ConduitConfig::slotOk();
}

void JPilotProxyWidgetSetup::slotApply() {
	FUNCTIONSETUP;
	commitChanges();
	ConduitConfig::slotApply();
}

void JPilotProxyWidgetSetup::slotAddConduit() {
	FUNCTIONSETUP;
	QString fn=KFileDialog::getOpenFileName(0, i18n("*.so|JPilot plugins\n*.*|All files"), this);
	if(fn.isNull()) return;
	// TODO: check of the plugin has already been loaded...
	if (	addConduit(fn, false)) {
		KMessageBox::sorry(this, i18n("Loading the JPilot plugin failed"));
	}
}

bool JPilotProxyWidgetSetup::addConduit(QString file, bool on) {
	JPlugin*newplug=JPilotProxyConduitFactory::addPlugin(file, on);
	if (!newplug) return false;
	QCheckListItem*plugitem=(QCheckListItem*)new QCheckListItem(fConfigWidget->ListPlugins,
		newplug->info.name, QCheckListItem::CheckBox);
	if (newplug->lib) plugitem->setText(1, newplug->info.fullpath);
	jp_startup_info si;
	si.base_dir="/usr/local/";
	newplug->startup(&si);
	if (on) plugitem->setOn(on);
}
void JPilotProxyWidgetSetup::slotConfigureConduit() {
	FUNCTIONSETUP;
	QListViewItem*item=fConfigWidget->ListPlugins->selectedItem();
	slotConfigureConduit(item);
}

JPlugin*JPilotProxyWidgetSetup::findPlugin(QString fn) {
	PluginIterator_t it(*JPilotProxyConduitFactory::plugins); // iterator for plugin list
	for ( ; it.current(); ++it ) {
		if (it.current()->info.fullpath==fn) return it.current();
	}
	return NULL;
}

void JPilotProxyWidgetSetup::slotConfigureConduit(QListViewItem*item) {
	FUNCTIONSETUP;
	if (!item) return;
	
	#ifdef DEBUG
	DEBUGCONDUIT<<"Configuring conduit "<<item->text(0)<<endl;
	#endif
	JPlugin*plg=findPlugin(item->text(1));
	if (!plg) {
		KMessageBox::sorry(this, i18n("Error finding the plugin in memory."));
		return;
	}
	if (plg->hasGui()) {
		// TODO: configure the plugin
		KMessageBox::sorry(this, i18n("Configuring JPilot plugins has not yet been implemented. "
			"This would mean embedding a GtkWidget inside a KDE dialog box, so that the whole message "
			"loop of the modal dialog box needs to be rewritten (see QGtkApplication)"));
	} else {
		KMessageBox::sorry(this, i18n("This JPilot plugin does not have a configuration dialog"));
	}
}

void JPilotProxyWidgetSetup::slotBrowse() {
	FUNCTIONSETUP;
	QString oldname=fConfigWidget->DirEdit->text();
	QString fn=KFileDialog::getExistingDirectory(oldname, this, i18n("Change Plugin Directory"));
	if(fn.isNull()) return;
	fConfigWidget->DirEdit->setText(fn);
}

void JPilotProxyWidgetSetup::slotSelectPluginPath() {
	FUNCTIONSETUP;
	QString path=fConfigWidget->ListPluginPathes->currentText();
	if (! path.isNull()) {
		updatePluginPathSel=false;
		fConfigWidget->DirEdit->setText(path);
	}
}

void JPilotProxyWidgetSetup::slotAddPluginPath() {
	FUNCTIONSETUP;
	QString fn=KFileDialog::getExistingDirectory(QString::null, this, i18n("Add Plugin Directory"));
	if (!fn.isNull()) {
		fConfigWidget->ListPluginPathes->insertItem(fn);
		fConfigWidget->ListPluginPathes->setCurrentItem(-1);
	}
}
void JPilotProxyWidgetSetup::slotRemovePluginPath(){ 
	FUNCTIONSETUP;
	fConfigWidget->ListPluginPathes->removeItem(fConfigWidget->ListPluginPathes->currentItem());
}

void JPilotProxyWidgetSetup::slotUpdatePluginPath(const QString &newpath) {
	FUNCTIONSETUP;
	if (updatePluginPathSel) 
		fConfigWidget->ListPluginPathes->changeItem(newpath, fConfigWidget->ListPluginPathes->currentItem());
	updatePluginPathSel=true;
}

/* virtual */ void JPilotProxyWidgetSetup::commitChanges() {
	FUNCTIONSETUP;

	if (!fConfig) return;
	KConfigGroupSaver s(fConfig, getSettingsGroup());
	
	// First save the list of plugin pathes
	QStringList plugpathes;
	for (int i=0; i<fConfigWidget->ListPluginPathes->count(); i++) {
		plugpathes<<fConfigWidget->ListPluginPathes->text(i);
	}
	fConfig->writeEntry(JPilotProxyConduitFactory::PluginPathes, plugpathes);

	// now save the list of all loaded/found plugins
	QStringList pluginfiles;
	QListViewItem *item=fConfigWidget->ListPlugins->firstChild();
	while (item) {
		pluginfiles << item->text(1);
		fConfig->writeEntry(item->text(1), (dynamic_cast<QCheckListItem*>(item))->isOn());
		item=item->nextSibling();
	}
	fConfig->writeEntry(JPilotProxyConduitFactory::LoadedPlugins, pluginfiles);
}

/* virtual */ void JPilotProxyWidgetSetup::readSettings() {
	FUNCTIONSETUP;

	if (!fConfig) {
		DEBUGCONDUIT << fname << ": !fConfig..." << endl;
		return;
	}
//TODO: Activate:
//	JPilotProxyConduitFactory::loadPlugins(fConfig);

	KConfigGroupSaver s(fConfig, getSettingsGroup());
	QStringList plugpathes=fConfig->readListEntry(JPilotProxyConduitFactory::PluginPathes);
	fConfigWidget->ListPluginPathes->insertStringList(plugpathes);
	
	// TODO: Use the plugin list? or use the list stored in the config?
	QStringList pluginfiles=fConfig->readListEntry(JPilotProxyConduitFactory::LoadedPlugins);
	for (QStringList::Iterator it = pluginfiles.begin(); it != pluginfiles.end(); ++it ) {
		addConduit(*it, fConfig->readBoolEntry(*it));
	}
}


// $Log$
// Revision 1.4  2002/04/09 01:11:55  kainhofe
// Renamed JPilotPluginProxy.ui (should not have the same name as the directory, since that creates problems when linking), some more debug message, when successfully loading a conduit the is no error message any more,
//
// Revision 1.3  2002/04/08 12:56:43  mhunter
// Corrected typographical errors
//
// Revision 1.2  2002/04/07 20:19:48  cschumac
// Compile fixes.
//
// Revision 1.1  2002/04/07 11:17:54  kainhofe
// First Version of the JPilotPlugin Proxy conduit. it can be activated, but loading a plugin or syncing a plugin crashes the palm (if no plugin is explicitely enabled, this conduit can be enabled and it won't crash KPIlot). A lot of work needs to be done, see the TODO
//
// Revision 1.7  2002/04/07 11:11:18  reinhold
// If the plugin is removed, this conduit does no longer crash
//
// Revision 1.6  2002/04/07 00:53:36  reinhold
// Loading plugins works, callbacks are resolved, dependencies on libplugin are not and crash the palm
//
// Revision 1.5  2002/04/07 00:10:49  reinhold
// Settings are now saved
//
// Revision 1.4  2002/04/06 19:08:02  reinhold
// the plugin compiles now and the plugins can be loaded (except that they crash if they access jp_init or jpilog_printf etc.)
//
// Revision 1.3  2002/04/01 14:37:33  reinhold
// Use DirListIterator to find the plugins in a directorz
// User KLibLoader to load the JPilot plugins
//
// Revision 1.2  2002/03/20 01:27:29  reinhold
// The plugin's setup dialog now runs without crashes. loading JPilot plugins still fails because of inresolved dependencies like jp_init or gtk_... functions.
//
// Revision 1.1  2002/03/18 23:16:11  reinhold
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
