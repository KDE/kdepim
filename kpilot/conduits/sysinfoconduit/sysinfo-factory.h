#ifndef _SYSINFO_FACTORY_H
#define _SYSINFO_FACTORY_H
/* SysInfo-factory.h                       KPilot
**
** Copyright (C) 2003 by Reinhold Kainhofer
**
** This file defines the factory for the SysInfo-conduit plugin.
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

#include <klibloader.h>

class KInstance;
class KAboutData;

class SysInfoConduitFactory : public KLibFactory
{
	Q_OBJECT

public:
	SysInfoConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~SysInfoConduitFactory();

	static KAboutData *about() { return fAbout; } ;

protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
	static KAboutData *fAbout;
public:
	// KConfig entry keys.
	static const char *fGroup,
		*fOutputFile, *fOutputType, *fTemplateFile,
		*fHardwareInfo, *fUserInfo,
		*fMemoryInfo, *fStorageInfo,
		*fDBList, *fRecordNumber,
		*fSyncInfo, *fKDEVersion,
		*fPalmOSVersion, *fDebugInfo;
} ;

extern "C"
{
	void *init_libsysinfoconduit();
}

#endif
