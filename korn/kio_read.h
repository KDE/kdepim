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

#ifndef MK_KIO_READ_H
#define MK_KIO_READ_H

/*
 * @file
 *
 * This file contains the class KIO_Read
 * This class should be used if someone wants to read the Full Message
 */

#include <qobject.h>

class KKioDrop;

class KUrl;
namespace KIO { class MetaData; class Job; }
class KIO_Protocol;

class QString;
class QVariant;

/**
 * This class is used to read full messages.
 * This class can access the private members of KKioDrop, because KKioDrop is friend of this class.
 */
class KIO_Read : public QObject
{ Q_OBJECT
public:
	/**
	 * Constructor
	 * 
	 * @param parent the parent of this object
	 */
	KIO_Read( QObject * parent = 0 );
	/**
	 * Destructor
	 */
	~KIO_Read();

public slots:
	/**
	 * This function makes this class start downloading the messages.
	 * This is the function which makes the nessesairy slaves for reading a message.
	 *
	 * @param id the message to be downloaded
	 * @param drop the maildrop of the account of the message
	 */
	void readMail( const QVariant, KKioDrop* drop );
	/**
	 * This function cancels a pending download.
	 * Normally, this slot is connected to a "Cancel"-button.
	 */
	void canceled();
private:
	KKioDrop *_kio;
	KIO::Job *_job;
	QString *_message;
	
signals:
	/**
	 * This signal is emitted when the whole message is read; the message got passed as QString*
	 *
	 * @param msg a pointer to the full message
	 *
	 * @todo QString* -> const QString&
	 */
	void ready( QString *msg );
	
private slots:
	void slotResult( KIO::Job* );
	void slotData( KIO::Job*, const QByteArray& );
};

#endif //MK_KIO_READ_H
