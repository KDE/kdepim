#ifndef _ABBROWSER_FACTORY_H
#define _ABBROWSER_FACTORY_H
/* abbrowser-factory.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the abbrowser-conduit plugin.
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

class AbbrowserConduitFactory : public KLibFactory
{
Q_OBJECT

public:
	AbbrowserConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~AbbrowserConduitFactory();

	static KAboutData *about() { return fAbout; } ;
	static const char *group() { return fGroup; } ;
	static const char *smartMerge() { return fSmartMerge; } ;
	static const char *conflictResolution() { return fResolution; } ;
	static const char *archiveDeletedRecs() { return fArchive; };
	static const char *streetType() { return fStreetType; } ;
	static const char *faxType() { return fFaxType; } ;
	static const char *syncMode() { return fSyncMode;};
	static const char *firstSync() { return fFirstSync; } ;
	static const char *fullSyncOnPCChange() {return fFullSyncOnPCChange; } ;
	static const char *otherField() { return fOtherField; } ;
	
protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
	static KAboutData *fAbout;
	static const char *fGroup;

	// KConfig entry keys.
	//
	//
	static const char *fSmartMerge,
		*fResolution,
		*fArchive,
		*fStreetType,
		*fFaxType,
		*fSyncMode, 
		*fFirstSync,
		*fOtherField,
		*fFullSyncOnPCChange;
} ;

extern "C"
{

void *init_libaddressconduit();

} ;

// $Log$
// Revision 1.5  2002/06/30 16:23:23  kainhofe
// Started rewriting the addressbook conduit to use libkabc instead of direct dcop communication with abbrowser. Palm->PC is enabled (but still creates duplicate addresses), the rest is completely untested and thus disabled for now
//
// Revision 1.4  2002/04/16 18:22:12  adridg
// Wishlist fix from David B: handle formatted names when syncing
//
// Revision 1.3  2001/12/20 22:55:21  adridg
// Making conduits save their configuration and doing syncs
//
// Revision 1.2  2001/12/10 22:10:17  adridg
// Make the conduit compile, for Danimo, but it may not work
//
// Revision 1.1  2001/10/31 23:54:45  adridg
// CVS_SILENT: Ongoing conduits ports
//

#endif
