#ifndef _KPILOT_VCAL_FACTORYBASE_H
#define _KPILOT_VCAL_FACTORYBASE_H
/* vcal-factory.h                       KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the vcal-conduit plugin.
** It also defines the class for the behavior of the setup dialog.
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

#define RES_PALMOVERRIDES 0
#define RES_PCOVERRIDES 1
#define RES_ASK 2

#define SYNC_FIRST 0
#define SYNC_FAST 1
#define SYNC_FULL 2
#define SYNC_MAX SYNC_FULL

class KAboutData;
class VCalConduitSettings;

class VCalConduitFactoryBase : public KLibFactory
{
  Q_OBJECT

public:
	VCalConduitFactoryBase(QObject * p= 0L,const char * n= 0L):KLibFactory(p,n){};
	virtual ~VCalConduitFactoryBase();
	static KAboutData *about() { return fAbout; };

protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() )=0;
	static KAboutData *fAbout;
};

#endif
