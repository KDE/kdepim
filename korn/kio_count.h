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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MK_KIOCOUNT_H
#define MK_KIOCOUNT_H

/**
 * @file
 *
 * This file contains the class KIO_Count
 */

#include <qobject.h>

#include "kio.h" //Alsewise, no access to KKioDrop::FileInfo (needed in template)

#include <kio/global.h> //FIXME: without this, moc-compiler fails.

class KIO_Protocol;
class KIO_Single_Subject;
class KKioDrop; 

class KornMailSubject;

template<class T> class QList;
namespace KIO { class Job; class ListJob; class MetaData; class Slave; }

class KUrl;

class QString;

/**
 * This class count the number of message available in a mailbox.
 * This class have access to the KKioDrop drop throught friend classes.
 */
class KIO_Count : public QObject
{ Q_OBJECT
public:
	/**
	 * Constructor.
	 *
	 * @param parent the parent of this object
	 */
	KIO_Count( QObject * parent = 0 );
	/**
	 * Destructor
	 */
	~KIO_Count();
	
	/**
	 * This function start counting.
	 * This infomration needed for counting are retrieved through the drop parameter.
	 *
	 * @param drop the KKioDrop of the account to check
	 */
	void count( KKioDrop* drop );
	
	/**
	 * This function returns true if no errors occured.
	 *
	 * @return false if an error occured; true otherwise
	 */
	bool valid( ) { return _valid; }
	
	/**
	 * This function stops the pending counting.
	 */
	void stopActiveCount();
private:
	KKioDrop *_kio;
	KIO::ListJob *_job;
	KIO::Slave *_slave;
	KUrl *_kurl;
	KIO::MetaData *_metadata;
	const KIO_Protocol *_protocol;
	bool _valid;
	QList<KKioDrop::FileInfo> *_new_mailurls; //entries can come with more function calls.
	int _subjects_pending;
	int _total_new_messages;
	QList< KornMailSubject > *_popup_subjects;
private:
	void showPassive( const QString& );
	void disconnectSlave();
	
private slots:
	void result( KIO::Job* );
	void entries( KIO::Job*, const KIO::UDSEntryList &list );
	
	void addtoPassivePopup( KornMailSubject* );
	void deleteSingleSubject( KIO_Single_Subject* );
};

#endif //MK_KIOCOUNT_H
