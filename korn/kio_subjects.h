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

/**
 * This file defines the class KIO_Subjects
 */

#include <qobject.h>

class KKioDrop;
class KIO_Single_Subject;
class KIO_Protocol;
class KornMailSubject;

namespace KIO { class MetaData; class Slave; }
class KUrl;

template<class T> class QList;
class QString;

/**
 * This class calls other class to read all the subjects
 */
class KIO_Subjects : public QObject
{ Q_OBJECT
public:
	/**
	 * Constructor
	 */
	KIO_Subjects( QObject * parent );
	/**
	 * Destructor
	 */
	~KIO_Subjects( );
	
	/**
	 * This function let it start fetching headers.
	 *
	 * @param drop the drop to take of the list of message which have to be fetched
	 */
	void doReadSubjects( KKioDrop* drop );
	
	/**
	 * This function can be used to find out if this class is still valid.
	 * 
	 * @return false if an error occured; true otherwise
	 */
	bool valid( ) { return _valid; }
	
private:
	KKioDrop *_kio;
	KURL *_kurl;
	KIO::MetaData *_metadata;
	const KIO_Protocol *_protocol;
	QList<KIO_Single_Subject*> *_jobs;
	KIO::Slave *_slave;
	bool _valid;
	
	//Opens a connection.
	void getConnection( );
	//Start a job; the job itself is executed in KIO_Single_Subject
	void startJob( const QString&, const long );
	//Disconnect the connection
	void disConnect( bool );
	
public slots:
	/**
	 * This slot is called when the fetching of messages is canceled (by the user).
	 */
	void cancelled( );
	
private slots:
	void slotReadSubject( KornMailSubject* );
	void slotFinished( KIO_Single_Subject* );
};

#endif //MK_KIO_SUBJECTS_H
