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

#ifndef MK_KIO_DELETE_H
#define MK_KIO_DELETE_H

/**
 * @file
 *
 * This file contains the class KIO_Delete
 */

#include <QObject>

class KKioDrop;
class KIO_Protocol;
class KJob;
class KUrl;
namespace KIO { class MetaData; class Job; class Slave; }

template<class T> class QList;
class QVariant;

/**
 * This class handles removing of selected messages.
 * This class starts working when deleteMails() is called.
 */
class KIO_Delete : public QObject
{ Q_OBJECT
public:
	/**
	 * Constructor
	 *
	 * @param parent the parent of this object
	 */
	KIO_Delete( QObject * parent = 0 );
	/**
	 * Destructor
	 */
	~KIO_Delete( );
	
	/**
	 * This function should be called if there are messages to be deleted.
	 *
	 * @param list a list of messages which should be deleted
	 * @param drop the maildrop of the account
	 * @return true if succesfull, false otherwise
	 */
	bool deleteMails( QList< QVariant > *list, KKioDrop* drop );
	
	/**
	 * This function should return false then and only then if an error occurred.
	 *
	 * @return false if an error occured; true otherwise
	 */
	bool valid( ) { return _valid; }
	
public slots:
	/**
	 * If this slot is called, the whole deletion is canceled.
	 */
	void canceled( );
private slots:
	void slotResult( KJob* );
	
private:
	void disConnect( );
	bool setupSlave( KUrl kurl, KIO::MetaData metadata, const KIO_Protocol *& protocol );
	void deleteItem( const QVariant item, KUrl, KIO::MetaData, const KIO_Protocol *&);
	void commitDelete( KUrl, KIO::MetaData, const KIO_Protocol *& ) const;

	KKioDrop *_kio;
	mutable unsigned int _total;
	QList< KIO::Job* > *_jobs;
	KIO::Slave *_slave;
	bool _valid;
};

#endif //MK_KIO_DELETE_H
