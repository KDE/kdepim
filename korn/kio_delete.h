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

#ifndef MK_KIO_DELETE_H
#define MK_KIO_DELETE_H

/*
 * This class handles removing of selected messages.
 * This class starts working when deleteMails() is called.
 */

#include <qobject.h>
class KKioDrop;
class KIO_Protocol;
class KornMailId;

class KURL;
namespace KIO { class MetaData; class Job; class Slave; }

template<class T> class QPtrList;

class KIO_Delete : public QObject
{ Q_OBJECT
public:
	//constructors
	KIO_Delete( QObject * parent = 0, const char * name = 0 );
	~KIO_Delete( );
	
	//This function should be called if there are messages to be deleted.
	bool deleteMails( QPtrList< const KornMailId > *, KKioDrop* );
	
	//This function should return false then and only then if an error occured.
	bool valid( ) { return _valid; }
	
public slots:
	//If this slot is called, the whole deletion is canceled.
	void canceled( );
private slots:
	void slotResult( KIO::Job* );
	
private:
	void disConnect( );
	bool setupSlave( KURL kurl, KIO::MetaData metadata, KIO_Protocol *& protocol );
	void deleteItem( const KornMailId *item, KURL, KIO::MetaData, KIO_Protocol *&);
	void commitDelete( KURL, KIO::MetaData, KIO_Protocol *& );

	KKioDrop *_kio;
	unsigned int _total;
	QPtrList< KIO::Job > *_jobs;
	KIO::Slave *_slave;
	bool _valid;
};

#endif //MK_KIO_DELETE_H
