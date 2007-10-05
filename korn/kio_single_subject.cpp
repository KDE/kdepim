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

#include "kio_single_subject.h"

#include "mailsubject.h"
#include "kio_proto.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kio/global.h>
#include <kio/scheduler.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kdatetime.h>

#include <QRegExp>
#include <QString>
#include <QVariant>
//Added by qt3to4:
#include <QTextStream>

KIO_Single_Subject::KIO_Single_Subject( QObject * parent,
		    KUrl &kurl, KIO::MetaData &metadata, const KIO_Protocol * protocol, KIO::Slave *& slave,
		    const QString &url, const long size )
		: QObject( parent )
{
	_kurl = new KUrl( kurl );
	_metadata = new KIO::MetaData( metadata );
	_protocol = protocol;
	_name = new QString( url );
	_size = size;
	_message = new QString;

	init( slave );
}

KIO_Single_Subject::~KIO_Single_Subject( )
{
	if( _job )
		KIO::Scheduler::cancelJob( _job );
	delete _kurl;
	delete _metadata;
	delete _name;
	delete _message;
}

void KIO_Single_Subject::init( KIO::Slave *& slave)
{
	_job = KIO::get( *_kurl, KIO::NoReload, KIO::HideProgressInfo );
	_job->addMetaData( *_metadata );

	connect( _job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
	connect( _job, SIGNAL( data         (KIO::Job *, const QByteArray &) ),
	         this, SLOT( slotData(KIO::Job *, const QByteArray &) ) );

	if( _protocol->connectionBased( ) && slave )
		KIO::Scheduler::assignJobToSlave( slave , _job );
	else
		KIO::Scheduler::scheduleJob( _job );

}

void KIO_Single_Subject::parseMail( QString * message, KornMailSubject *subject, bool fullMessage )
{
	QTextStream stream( message, QIODevice::ReadOnly );
	QString line;
	QRegExp rx_sender( "^[fF]rom: " ); //Ex: From: ...
	QRegExp rx_sender_has_name1( "^[fF]rom:\\s*(\\w+[\\w\\s]*)\\<" ); //Ex: From: A name<email@domein.country>
	QRegExp rx_sender_has_name2( "^[fF]rom:\\s*\\\"\\s*(\\w+[\\w\\s]*)\\\""); //Ex: From: "A name"<a@invalid>
	QRegExp rx_subject( "^[sS]ubject: " ); //Ex: Subject: ...
	QRegExp rx_date  ( "^[dD]ate: ");
	bool inheader = true;
	int fieldnumber = 0;

	while ( ! stream.atEnd() )
	{
		line = stream.readLine();
		if( line.isEmpty() && fieldnumber >= 2 )
			inheader = false;

		if( inheader )
		{
			if( rx_sender.indexIn( line ) == 0 )
			{
				if( rx_sender_has_name1.indexIn( line ) == 0 )
					subject->setSender( rx_sender_has_name1.cap( 1 )  );
				else if(rx_sender_has_name2.indexIn( line ) == 0)
					subject->setSender( rx_sender_has_name2.cap( 1 ) );
				else
					subject->setSender( line.remove( rx_sender ) );
				++fieldnumber;
			}
			else if( rx_subject.indexIn( line ) == 0 )
			{
				subject->setSubject( line.remove( rx_subject ) );
				++fieldnumber;
			}
			else if( rx_date.indexIn( line ) == 0 )
			{
				KDateTime dt = KDateTime::fromString(  line.right(  line.length() - 6 ),
									KDateTime::RFCDate );
	                 	subject->setDate( dt.toTime_t() );
				++fieldnumber;
			}
		}
	}

	subject->setHeader( *message, fullMessage );
}

void KIO_Single_Subject::slotData( KIO::Job* job, const QByteArray& data )
{
	if( job != _job )
		kWarning() << i18n("Got invalid job; something strange happened?" );
	if( !data.isEmpty() )
		_message->append( data );
}

//KIO::Scheduler::disconnectSlave missing  if connection stops
void KIO_Single_Subject::slotResult( KJob *job )
{
	if( job != _job )
		kWarning() << i18n("Got invalid job; something strange happened?" );

	if( job->error() )
	{
		kWarning() << i18n("Error when fetching %1: %2", *_name, job->errorString() );
	} else {
		KornMailSubject * mailSubject = new KornMailSubject( QVariant( *_name ), 0 );
		parseMail( _message, mailSubject, _protocol->fullMessage() );
		mailSubject->decodeHeaders();
		mailSubject->setSize( _size );
		emit readSubject( mailSubject );
	}

	_job = 0;

	emit finished( this );
}

#include "kio_single_subject.moc"
