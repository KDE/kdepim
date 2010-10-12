#ifndef _DOC_FACTORY_H
#define _DOC_FACTORY_H

/* doc-factory.h                       KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the doc-conduit plugin.
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
	
#include <klibloader.h>
	
class KInstance;
class KAboutData;


class DOCConduitFactory:public KLibFactory 
{
 
Q_OBJECT 
 
public:
	DOCConduitFactory(QObject * = 0L, const char * = 0L);
	virtual ~ DOCConduitFactory();
	
	static KAboutData *about() { 
		return fAbout;
	};

	static const char 
		*fDBListSynced;
	static const char *dbDOCtype;
	static const char *dbDOCcreator;
 

protected:
	virtual QObject * createObject(QObject * parent = 0, 
		const char *name = 0, 
		const char *classname = "QObject", 
		const QStringList & args = QStringList());
 
private:
	KInstance * fInstance;
	static KAboutData *fAbout;
};


extern "C" {
	void *init_libdocconduit();
}


#endif
