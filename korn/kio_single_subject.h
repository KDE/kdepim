#ifndef MK_KIO_SINGEL_SUBJECT_H
#define MK_KIO_SINGEL_SUBJECT_H

/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

//This function reads a single subject

#include <qobject.h>

class KornMailSubject;

class KURL;
namespace KIO { class MetaData; class Job; class TransferJob; class Slave; }
class KIO_Protocol;

class QString;
template<class T> class QMemArray;
typedef QMemArray<char> QByteArray;

class KIO_Single_Subject : public QObject
{ Q_OBJECT
public:
	KIO_Single_Subject( QObject * parent, const char * name, KURL &, KIO::MetaData &, KIO_Protocol *,
	                    KIO::Slave *&, const QString &, const long );
	~KIO_Single_Subject( );
	
	//This functions try's te parse EMail; data, sender names and so on...
	static void parseMail( QString * message, KornMailSubject *subject, bool );
	
private:
	QString *_message;
	QString *_name;
	KURL *_kurl;
	KIO_Protocol *_protocol;
	KIO::MetaData *_metadata;
	KIO::TransferJob* _job;
	long _size;
	
	void init( KIO::Slave*& );
	
private slots:
	void slotResult( KIO::Job* );
	void slotData( KIO::Job*, const QByteArray& );
	
signals:
	//This signal is emitted if the headers are read and put into a KornMailSubject*
	void readSubject( KornMailSubject* );
	//This signal is emitted if this class could be destroyed.
	void finished( KIO_Single_Subject* );
};

#endif
