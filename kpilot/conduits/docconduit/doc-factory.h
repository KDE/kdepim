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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
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
	

	static const char *fGroup;
	static const char *fDOCDir, *fPDBDir, *fKeepPDBLocally, 
		*fConflictResolution, 
		*fConvertBookmarks, *fBookmarksBmk, *fBookmarksInline, *fBookmarksEndtags, 
		*fCompress, 
		*fSyncDirection, *fDOCList, *fDBListSynced, *fIgnoreBmkChanges, *fLocalSync, *fAlwaysUseResolution;
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
};



// $Log$
// Revision 1.2  2002/12/31 00:22:10  kainhofe
// Currently restructuring everything. Not yet finished.
//
// Revision 1.1  2002/12/13 16:29:53  kainhofe
// New PalmDOC conduit to syncronize text files with doc databases (AportisDoc, TealReader, etc) on the handheld
//
//
#endif
