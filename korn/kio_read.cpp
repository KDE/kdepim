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

#include "kio_read.h"

#include "kio.h"
#include "kio_proto.h"
#include "mailid.h"
#include "stringid.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kio/global.h>
#include <kio/jobclasses.h>
#include <kio/scheduler.h>

#include <qcstring.h>
#include <qstring.h>

KIO_Read::KIO_Read( QObject * parent, const char * name )
	: QObject( parent, name ),
	_job( 0 ),
	_message( 0 )
{
	_message = new QString;
}

KIO_Read::~KIO_Read()
{
	delete _message;
	delete _job;
}

void KIO_Read::readMail( const KornMailId *& mailid, KKioDrop* drop )
{
	_kio = drop;
	KURL kurl = *_kio->_kurl;
	KIO::MetaData metadata = *_kio->_metadata;
	
	kurl = dynamic_cast<const KornStringId*>(mailid)->getId( );
	
	_kio->_protocol->readMailKURL( kurl, metadata );
	
	_job = KIO::get( kurl, false, false );
	_job->addMetaData( metadata );
	
	connect( _job, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotResult( KIO::Job* ) ) );
	connect( _job, SIGNAL( data( KIO::Job*, const QByteArray& ) ), this, SLOT( slotData( KIO::Job*, const QByteArray & ) ) );
}

void KIO_Read::canceled( )
{
	if( _job )
		delete _job;
	_job = 0;
}

void KIO_Read::slotResult( KIO::Job* job )
{
	if( job != _job )
		kdWarning() << i18n( "Unknown job returned; I will try if this one will do... " ) << endl;

	if( job->error() )
		kdWarning() << i18n( "An error occurred when fetching the requested email: %1." ).arg( job->errorString() ) << endl;
		
	_kio->emitReadMailReady( _message );
	
	*_message = "";
	_job = 0;
}

void KIO_Read::slotData( KIO::Job* job, const QByteArray & data )
{
	if( job != _job )
		kdWarning() << i18n( "Unknown job returned; I will try if this one will do... " ) << endl;
	
	if( !data.isEmpty() )
		_message->append( data );
}

#include "kio_read.moc"
