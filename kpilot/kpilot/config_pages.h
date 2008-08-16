#ifndef CONFIG_PAGES_H
#define CONFIG_PAGES_H
/* KPilot
**
** Copyright (C) 2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines logic for the pages that make up the configuration
** dialog for KPilot.
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
#include "ui_config_page_device.h"
#include "ui_config_page_backup.h"
#include "ui_config_page_sync.h"
#include "ui_config_page_startup.h"

// class KPilotConfigWidget;
class StartExitConfigWidget;
class SyncConfigWidget;
class DeviceConfigWidget;
class BackupConfigWidget;

class ConfigPage : public ConduitConfigBase
{
public:
	ConfigPage( QWidget *w, QVariantList &args ) : ConduitConfigBase( w, args ) { } ;
protected:
	// Override base class virtual function.
	virtual QString maybeSaveText() const;
} ;

class DeviceConfigPage : public ConfigPage
{
Q_OBJECT
public:
	DeviceConfigPage( QWidget *, QVariantList &args );

protected:
	virtual void load();
	virtual void commit();

protected slots:
	void changePortType(int);
	void autoDetectDevice();

private:
	Ui::DeviceConfigWidget fConfigWidget;

	// Read and write the values of Encoding
	// and EncodingDD.
	void getEncoding();
	void setEncoding();
} ;

class SyncConfigWidget : public QWidget, public Ui::SyncConfigWidget
{
public:
  SyncConfigWidget( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

class KDE_EXPORT SyncConfigPage : public ConfigPage
{
public:
	SyncConfigPage( QWidget *, QVariantList &args );

protected:
	virtual void load();
	virtual void commit();

private:
	SyncConfigWidget *fConfigWidget;
} ;

class BackupConfigPage : public ConfigPage
{
Q_OBJECT
public:
	BackupConfigPage( QWidget *, QVariantList &args );

protected:
	virtual void load();
	virtual void commit();

protected slots:
	void slotSelectNoBackupDBs();
	void slotSelectNoRestoreDBs();

private:
	Ui::BackupConfigWidget fConfigWidget;
} ;

class StartExitConfigPage : public ConfigPage
{
public:
	StartExitConfigPage( QWidget *, QVariantList &args );

protected:
	virtual void load();
	virtual void commit();

private:
	Ui::StartExitConfigWidget fConfigWidget;
} ;

#endif
