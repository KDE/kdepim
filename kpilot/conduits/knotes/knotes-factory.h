#ifndef _KPILOT_NULL_FACTORY_H
#define _KPILOT_NULL_FACTORY_H
/* null-factory.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the null-conduit plugin.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <klibloader.h>

class KInstance;
class KAboutData;

class KNotesConduitFactory : public KLibFactory
{
Q_OBJECT

public:
	KNotesConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~KNotesConduitFactory();

	static KAboutData *about() { return fAbout; } ;

	// The KNotes instance, unlike previous conduits (alphabetically)
	// has const char * const members. The extra const prevents people
	// from assigning to this variable, so you have to work hard to
	// break its value. We store group and entry keys in here.
	//
	// Typical usage is KConfig::setGroup(KNotesConduitFactory::group).
	//
	//
	static const char * const group ;
	static const char * const matchDeletes ; // delete note when memo is deleted

protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
	static KAboutData *fAbout;
} ;

extern "C"
{

void *init_libknotesconduit();

} ;

// $Log$
// Revision 1.4  2001/10/31 23:46:51  adridg
// CVS_SILENT: Ongoing conduits ports
//
// Revision 1.3  2001/10/16 21:44:53  adridg
// Split up some files, added behavior
//
// Revision 1.2  2001/10/10 21:42:09  adridg
// Actually do part of a sync now
//
// Revision 1.1  2001/10/08 22:27:41  adridg
// New ui, moved to lib-based conduit
//
// Revision 1.1  2001/10/04 23:51:55  adridg
// Nope. One more really final commit to get the alpha to build. Dirk, otherwise just remove the conduits/ subdir from kdepim/kpilot/Makefile.am
//

#endif
