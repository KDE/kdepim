/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MK_KIOCOUNT_H
#define MK_KIOCOUNT_H

//This class count the number of message available.

#include <qobject.h>

#include "kio.h" //Alsewise, no access to KKioDrop::FileInfo (needed in template)

#include <kio/global.h> //FIXME: without this, moc-compiler fails.

class KIO_Protocol;
class KIO_Single_Subject;
class KKioDrop; 

class KornMailSubject;
class SortedMailSubject;

template<class T> class QValueList;
namespace KIO { class Job; class ListJob; class MetaData; class Slave; }

class KURL;

class QString;

class KIO_Count : public QObject
{ Q_OBJECT
public:
	KIO_Count( QObject * parent = 0, const char * name = 0 );
	~KIO_Count();
	
	//This function starts counting
	void count( KKioDrop* );
	
	//This functions returns true of no error has occured.
	bool valid( ) { return _valid; }
	
	void stopActiveCount();
private:
	KKioDrop *_kio;
	KIO::ListJob *_job;
	KIO::Slave *_slave;
	KURL *_kurl;
	KIO::MetaData *_metadata;
	KIO_Protocol *_protocol;
	bool _valid;
	QValueList<KKioDrop::FileInfo> *_new_mailurls; //entries can come with more function calls.
	int _subjects_pending;
	int _total_new_messages;
	SortedMailSubject *_popup_subjects;
private:
	void showPassive( const QString& );
	void disconnectSlave();
	
private slots:
	void result( KIO::Job* );
	void entries( KIO::Job*, const KIO::UDSEntryList &list );
	
	void addtoPassivePopup( KornMailSubject* );
	void deleteSingleSubject( KIO_Single_Subject* );
};

#endif
