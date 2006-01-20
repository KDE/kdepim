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

#ifndef MK_KIO_SINGEL_SUBJECT_H
#define MK_KIO_SINGEL_SUBJECT_H

/**
 * @file
 *
 * This file contains the class KIO_Singel_Subject
 */

#include <qobject.h>

class KornMailSubject;

class KURL;
namespace KIO { class MetaData; class Job; class TransferJob; class Slave; }
class KIO_Protocol;

class QString;
class QByteArray;

/**
 * The class KIO_Single_Subject is used to fetch one subject of an email.
 * If can access the private members of KKioDrop, because of friend classes.
 */
class KIO_Single_Subject : public QObject
{ Q_OBJECT
public:
	/**
	 * Constructor.
	 * 
	 * @param parent the parent of this object
	 * @param url the base KURL used to make a url
	 * @param metadata the metadata used for the job
	 * @param protocol the class containing information about the protocol
	 * @param slave the slave which is to be used (only if the protocol is connection based)
	 * @param name the name of the message to fetch
	 * @param size the size the size of the full message
	 */
	KIO_Single_Subject( QObject * parent, KURL &url, KIO::MetaData &metadata, const KIO_Protocol * protocol,
	                    KIO::Slave *& slave, const QString &name, const long size );
	/**
	 * Destructor
	 */
	~KIO_Single_Subject( );
	
	/**
	 * This functions try's te parse EMail; data, sender names and so on...
	 *
	 * @param message pointer to the message
	 * @param subject the subject information to put the data to
	 * @param fullMessage true if @p message contains the whole message; false if it only contains the header of the message
	 */
	static void parseMail( QString * message, KornMailSubject *subject, bool fullMessage );
	
private:
	QString *_message;
	QString *_name;
	KURL *_kurl;
	const KIO_Protocol *_protocol;
	KIO::MetaData *_metadata;
	KIO::TransferJob* _job;
	long _size;
	
	void init( KIO::Slave*& );
	
private slots:
	void slotResult( KIO::Job* );
	void slotData( KIO::Job*, const QByteArray& );
	
signals:
	/**
	 * This signal is emitted if the headers are read and put into a KornMailSubject*
	 *
	 * @param subject the new constructed subject
	 */
	void readSubject( KornMailSubject* subject );
	/**
	 * This signal is emitted if this class could be destroyed.
	 * 
	 * @param self a pointer to self to get itself destroyed
	 *
	 * @todo check if the parameter can be replaced by "this->deleteLater()" in the class itself
	 */
	void finished( KIO_Single_Subject* self );
};

#endif //MK_KIO_SINGEL_SUBJECT_H
