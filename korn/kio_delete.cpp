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

#include "kio_delete.h"

#include "kio.h"
#include "kio_proto.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kio/global.h>
#include <kio/jobclasses.h>
#include <kio/deletejob.h>
#include <kio/scheduler.h>

#include <QList>
#include <QVariant>

KIO_Delete::KIO_Delete( QObject * parent ) : QObject( parent ),
	_kio( 0 ),
	_total( 0 ),
	_jobs( 0 ),
	_slave( 0 ),
	_valid( true )
{
	_jobs = new QList< KIO::Job* >;
}

KIO_Delete::~KIO_Delete( )
{
	disConnect( );
	delete _jobs;
}

bool KIO_Delete::deleteMails( QList< QVariant > * ids, KKioDrop *drop )
{
	KUrl kurl = *drop->_kurl;
	KIO::MetaData metadata = *drop->_metadata;
		
	_kio = drop;
	_valid = true;
	
	//disConnect earlier operations
	disConnect( );
	if( _kio->_protocol->connectionBased( ) )
	{
		if( ! setupSlave( kurl, metadata, _kio->_protocol ) )
		{
			_valid = false;
			return false;
		}
	}
	
	_total = ids->count( );
	
	for( int xx = 0; xx < ids->size(); ++xx )
		deleteItem( ids->at( xx ) , kurl, metadata, _kio->_protocol );
	
	if( _jobs->count() == 0 )
	{
		_kio->emitDeleteMailsReady( true );
		disConnect( );
		return true;
	}
	
	if( _kio->_protocol->commitDelete() )
		commitDelete( kurl, metadata, _kio->_protocol );
			
	_kio->emitDeleteMailsTotalSteps( _total );
		
	return true;
}

void KIO_Delete::disConnect( )
{
	_jobs->clear();

	if( _slave )
	{
		KIO::Scheduler::disconnectSlave( _slave );
		_slave = 0;
	}
}

bool KIO_Delete::setupSlave( KUrl kurl, KIO::MetaData metadata, const KIO_Protocol *& protocol )
{
	protocol->deleteMailConnectKUrl( kurl, metadata );
	
	if( kurl.port() == 0 )
		kurl.setPort( protocol->defaultPort( _kio->_ssl ) );
		
	if( ! ( _slave = KIO::Scheduler::getConnectedSlave( kurl, metadata ) ) )
	{
		kWarning() << i18n("Could not get a connected slave; I cannot delete this way..." );
		_valid = false;
		return false;
	}

	return true;
}

void KIO_Delete::deleteItem( const QVariant item, KUrl kurl, KIO::MetaData metadata, const KIO_Protocol *& protocol )
{
	KIO::Job* job = 0;

	if( item.type() != QVariant::String )
	{
		kDebug() <<"Got wrong id type in KIO_Delete::deleteItem!";
		return;
	}
	kurl = item.toString();
	
	protocol->deleteMailKUrl( kurl, metadata );
	
	if( kurl.port() == 0 )
		kurl.setPort( protocol->defaultPort( _kio->_ssl ) );
		
	if( protocol->deleteFunction() == KIO_Protocol::get )
	{
		job = KIO::get( kurl, true, false );
		
		if( protocol->connectionBased() )
			KIO::Scheduler::assignJobToSlave( _slave, dynamic_cast< KIO::SimpleJob* >( job ) );
		else
			KIO::Scheduler::scheduleJob( dynamic_cast< KIO::SimpleJob* >( job ) );
	}
	else if( protocol->deleteFunction() == KIO_Protocol::del )
	{
		job = KIO::del( kurl, false, false );
	}
	else
		return; //Unknown deleteFunction
		
	connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
		
	job->addMetaData( metadata );
	
	_jobs->append( job );
}

/*
 * Some protocols needs to a command to commit protocols.
 */
void KIO_Delete::commitDelete( KUrl kurl, KIO::MetaData metadata, const KIO_Protocol *& protocol ) const
{
	protocol->deleteCommitKUrl( kurl, metadata );
	
	if( kurl.port() == 0 )
		kurl.setPort( protocol->defaultPort( _kio->_ssl ) );
	
	KIO::TransferJob *job = KIO::get( kurl, true, false );
	job->addMetaData( metadata );
	connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
	
	_jobs->append( job );
	
	if( protocol->connectionBased() )
		KIO::Scheduler::assignJobToSlave( _slave, job );
	else
		KIO::Scheduler::scheduleJob( job );

	_total++;
}

void KIO_Delete::canceled( )
{
	disConnect( );
}

void KIO_Delete::slotResult( KJob* job )
{
	if( job->error() )
	{
		kWarning() << i18n("An error occurred when deleting email: %1.", job->errorString() );
		_valid = false;
	}
	
	_jobs->removeAll( static_cast<KIO::Job*>(job) );
	
	_kio->emitDeleteMailsProgress( _total - _jobs->count() );
	
	if( _jobs->isEmpty() )
	{
		_kio->emitDeleteMailsReady( _valid );
		disConnect();
	}
}


#include "kio_delete.moc"
