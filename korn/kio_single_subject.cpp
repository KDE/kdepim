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

#include <qregexp.h>
#include <qcstring.h>
#include <qstring.h>

KIO_Single_Subject::KIO_Single_Subject( QObject * parent, const char * name,
		    KURL &kurl, KIO::MetaData &metadata, KIO_Protocol * protocol, KIO::Slave *& slave,
		    const QString &url, const long size ) 
		: QObject( parent, name )
{
	_kurl = new KURL( kurl );
	_metadata = new KIO::MetaData( metadata );
	_protocol = protocol->clone( );
	_name = new QString( url );
	_size = size;
	_message = new QString;
	
	init( slave );
}

KIO_Single_Subject::~KIO_Single_Subject( )
{
	delete _kurl;
	delete _metadata;
	delete _protocol;
	delete _name;
	delete _message;
}

void KIO_Single_Subject::init( KIO::Slave *& slave)
{
	_job = KIO::get( *_kurl, false, false );
	_job->addMetaData( *_metadata );
	
	connect( _job, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotResult( KIO::Job* ) ) );
	connect( _job, SIGNAL( data         (KIO::Job *, const QByteArray &) ),
	         this, SLOT( slotData(KIO::Job *, const QByteArray &) ) );
		 
	if( _protocol->connectionBased( ) && slave )
		KIO::Scheduler::assignJobToSlave( slave , _job );
	else
		KIO::Scheduler::scheduleJob( _job );
		 
}

void KIO_Single_Subject::parseMail( QString * message, KornMailSubject *subject, bool fullMessage )
{
	QTextStream stream( message, IO_ReadOnly );
	QString line;
	QRegExp rx_sender( "^[fF]rom: " ); //Ex: From: ...
	QRegExp rx_sender_has_name1( "^[fF]rom:\\s*(\\w+[\\w\\s]*)\\<" ); //Ex: From: A name<email@domein.country>
	QRegExp rx_sender_has_name2( "^[fF]rom:\\s*\\\"\\s*(\\w+[\\w\\s]*)\\\""); //Ex: From: "A name"<a@invalid>
	QRegExp rx_subject( "^[sS]ubject: " ); //Ex: Subject: ...
	QRegExp rx_date  ( "^[dD]ate:\\s*(\\w{3},\\s*)?(\\d+)\\s*(\\w{3})\\s*(\\d+)\\s*(\\d+):(\\d+):(\\d+)\\s*([+-])(\\d+)\\s*$");
	bool inheader = true;
	bool firstLine = true;
	while ( ! stream.atEnd() )
	{
		line = stream.readLine();
		if( line.isEmpty() && ! firstLine )
			inheader = false;
		
		if( firstLine && !line.isEmpty() )
			firstLine = false;
	
		if( inheader )
		{
			if( rx_sender.search( line ) == 0 )
				if( rx_sender_has_name1.search( line ) == 0 )
					subject->setSender( rx_sender_has_name1.cap( 1 )  );
				else if(rx_sender_has_name2.search( line ) == 0)
					subject->setSender( rx_sender_has_name2.cap( 1 ) );
				else
					subject->setSender( line.remove( rx_sender ) );
			else if( rx_subject.search( line ) == 0 )
				subject->setSubject( line.remove( rx_subject ) );
			else if( rx_date.search( line ) == 0 )
			{
				subject->setDate( KRFCDate::parseDate( line.right( line.length() - 6 )  ) +
				                  KRFCDate::localUTCOffset() * 60 );
			}
		}
	}

	subject->setHeader( *message, fullMessage );
}

void KIO_Single_Subject::slotData( KIO::Job* job, const QByteArray& data )
{
	if( job != _job )
		kdWarning() << i18n( "Got invalid job; something strange happened?" ) << endl;
	if( !data.isEmpty() )
		_message->append( data );
}

void KIO_Single_Subject::slotResult( KIO::Job *job )
{
	if( job != _job )
		kdWarning() << i18n( "Got invalid job; something strange happened?" ) << endl;
		
	if( job->error() )
	{
		kdWarning() << i18n( "Error when fetching %1: %2" ).arg( *_name ).arg( job->errorString() ) << endl;
	} else {
		KornMailSubject * mailSubject = new KornMailSubject( new KornStringId( *_name ) );
		parseMail( _message, mailSubject, _protocol->fullMessage() );
		mailSubject->setSize( _size );
		emit readSubject( mailSubject );
	}
	
	emit finished( this );
}
