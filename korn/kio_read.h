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

#ifndef MK_KIO_READ_H
#define MK_KIO_READ_H

//This class should be used if someone wants to read the Full Message

#include <qobject.h>

class KKioDrop;
class KornMailId;

class KURL;
namespace KIO { class MetaData; class Job; }
class KIO_Protocol;

class QString;

class KIO_Read : public QObject
{ Q_OBJECT
public:
	KIO_Read( QObject * parent = 0, const char * name = 0 );
	~KIO_Read();

public slots:
	//This is the function which makes the nessesairy slaves for reading a message
	void readMail( const KornMailId *&, KKioDrop* );
	//This function should be called if the user presses canceled.
	void canceled();
private:
	KKioDrop *_kio;
	KIO::Job *_job;
	QString *_message;
	
signals:
	//This signal is emitted when the whole message is read; the message got passed as QString*
	void ready( QString* );
	
private slots:
	void slotResult( KIO::Job* );
	void slotData( KIO::Job*, const QByteArray& );
};

#endif //MK_KIO_READ_H
