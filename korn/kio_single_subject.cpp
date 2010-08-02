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
#include "stringid.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kio/global.h>
#include <kio/scheduler.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <krfcdate.h>

#include <tqregexp.h>
#include <tqcstring.h>
#include <tqstring.h>

KIO_Single_Subject::KIO_Single_Subject( TQObject * parent, const char * name,
		    KURL &kurl, KIO::MetaData &metadata, const KIO_Protocol * protocol, KIO::Slave *& slave,
		    const TQString &url, const long size ) 
		: TQObject( parent, name )
{
	_kurl = new KURL( kurl );
	_metadata = new KIO::MetaData( metadata );
	_protocol = protocol;
	_name = new TQString( url );
	_size = size;
	_message = new TQString;
	
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
	_job = KIO::get( *_kurl, false, false );
	_job->addMetaData( *_metadata );
	
	connect( _job, TQT_SIGNAL( result( KIO::Job* ) ), this, TQT_SLOT( slotResult( KIO::Job* ) ) );
	connect( _job, TQT_SIGNAL( data         (KIO::Job *, const TQByteArray &) ),
	         this, TQT_SLOT( slotData(KIO::Job *, const TQByteArray &) ) );
	
	if( _protocol->connectionBased( ) && slave )
		KIO::Scheduler::assignJobToSlave( slave , _job );
	else
		KIO::Scheduler::scheduleJob( _job );
		 
}

void KIO_Single_Subject::parseMail( TQString * message, KornMailSubject *subject, bool fullMessage )
{
	TQTextStream stream( message, IO_ReadOnly );
	TQString line;
	TQRegExp rx_sender( "^[fF]rom: " ); //Ex: From: ...
	TQRegExp rx_sender_has_name1( "^[fF]rom:\\s*(\\w+[\\w\\s]*)\\<" ); //Ex: From: A name<email@domein.country>
	TQRegExp rx_sender_has_name2( "^[fF]rom:\\s*\\\"\\s*(\\w+[\\w\\s]*)\\\""); //Ex: From: "A name"<a@invalid>
	TQRegExp rx_subject( "^[sS]ubject: " ); //Ex: Subject: ...
	TQRegExp rx_date  ( "^[dD]ate: ");
	bool inheader = true;
	int fieldnumber = 0;

	while ( ! stream.atEnd() )
	{
		line = stream.readLine();
		if( line.isEmpty() && fieldnumber >= 2 )
			inheader = false;
		
		if( inheader )
		{
			if( rx_sender.search( line ) == 0 )
			{
				if( rx_sender_has_name1.search( line ) == 0 )
					subject->setSender( rx_sender_has_name1.cap( 1 )  );
				else if(rx_sender_has_name2.search( line ) == 0)
					subject->setSender( rx_sender_has_name2.cap( 1 ) );
				else
					subject->setSender( line.remove( rx_sender ) );
				++fieldnumber;
			}
			else if( rx_subject.search( line ) == 0 )
			{
				subject->setSubject( line.remove( rx_subject ) );
				++fieldnumber;
			}
			else if( rx_date.search( line ) == 0 )
			{
				subject->setDate( KRFCDate::parseDate( line.right( line.length() - 6 ) ) );
				++fieldnumber;
			}
		}
	}

	subject->setHeader( *message, fullMessage );
}

void KIO_Single_Subject::slotData( KIO::Job* job, const TQByteArray& data )
{
	if( job != _job )
		kdWarning() << i18n( "Got invalid job; something strange happened?" ) << endl;
	if( !data.isEmpty() )
		_message->append( data );
}

//KIO::Scheduler::disconnectSlave missing  if connection stops
void KIO_Single_Subject::slotResult( KIO::Job *job )
{
	if( job != _job )
		kdWarning() << i18n( "Got invalid job; something strange happened?" ) << endl;
		
	if( job->error() )
	{
		kdWarning() << i18n( "Error when fetching %1: %2" ).arg( *_name ).arg( job->errorString() ) << endl;
	} else {
		KornMailSubject * mailSubject = new KornMailSubject( new KornStringId( *_name ), 0 );
		parseMail( _message, mailSubject, _protocol->fullMessage() );
		mailSubject->decodeHeaders();
		mailSubject->setSize( _size );
		emit readSubject( mailSubject );
	}
	
	_job = 0;
	
	emit finished( this );
}

#include "kio_single_subject.moc"
