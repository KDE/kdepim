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

#ifndef MK_KIO_SUBJECTS_H
#define MK_KIO_SUBJECTS_H

//This class calls other class to read all the subjects

#include <qobject.h>
class KKioDrop;
class KIO_Single_Subject;
class KIO_Protocol;
class KornMailSubject;

namespace KIO { class MetaData; class Slave; }
class KURL;

template<class T> class QPtrList;
class QString;
template<class T> class QValueList;

class KIO_Subjects : public QObject
{ Q_OBJECT
public:
	KIO_Subjects( QObject * parent, const char * name );
	~KIO_Subjects( );
	
	//This function let it start fetching headers.
	void doReadSubjects( KKioDrop* );
	
	//This function should return true then and only then of no error occurred.
	bool valid( ) { return _valid; }
	
private:
	KKioDrop *_kio;
	KURL *_kurl;
	KIO::MetaData *_metadata;
	const KIO_Protocol *_protocol;
	QPtrList<KIO_Single_Subject> *_jobs;
	KIO::Slave *_slave;
	bool _valid;
	
	//Opens a connection.
	void getConnection( );
	//Start a job; the job itself is executed in KIO_Single_Subject
	void startJob( const QString&, const long );
	//Disconnect the connection
	void disConnect( bool );
	
public slots:
	//This function called the fetching of headers.
	void cancelled( );
	
private slots:
	void slotReadSubject( KornMailSubject* );
	void slotFinished( KIO_Single_Subject* );
};

#endif
