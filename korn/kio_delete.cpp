#include "kio_delete.h"

#include "mailid.h"
#include "stringid.h"
#include "kio.h"
#include "kio_proto.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kio/global.h>
#include <kio/jobclasses.h>
#include <kio/scheduler.h>

#include <qptrlist.h>

KIO_Delete::KIO_Delete( QObject * parent, const char * name ) : QObject( parent, name ),
	_kio( 0 ),
	_total( 0 ),
	_jobs( 0 ),
	_slave( 0 ),
	_valid( true )
{
	_jobs = new QPtrList< KIO::Job >;
}

KIO_Delete::~KIO_Delete( )
{
	disConnect( );
	delete _jobs;
}

bool KIO_Delete::deleteMails( QPtrList< const KornMailId > * ids, KKioDrop *drop )
{
	KURL kurl = *drop->_kurl;
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
	
	for( const KornMailId * item = ids->first(); item; item = ids->next() )
		deleteItem( item, kurl, metadata, _kio->_protocol );
	
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
	_jobs->clear( );

	if( _slave )
	{
		KIO::Scheduler::disconnectSlave( _slave );
		_slave = 0;
	}
}

bool KIO_Delete::setupSlave( KURL kurl, KIO::MetaData metadata, KIO_Protocol *& protocol )
{
	protocol->deleteMailConnectKURL( kurl, metadata );
	
	if( kurl.port() == 0 )
		kurl.setPort( protocol->defaultPort() );
		
	if( ! ( _slave = KIO::Scheduler::getConnectedSlave( kurl, metadata ) ) )
	{
		kdWarning() << i18n( "Could not get a connected slave; I cannot delete this way..." ) << endl;
		_valid = false;
		return false;
	}

	return true;
}

void KIO_Delete::deleteItem( const KornMailId *item, KURL kurl, KIO::MetaData metadata, KIO_Protocol *& protocol )
{
	KIO::Job* job = 0;

	kurl = dynamic_cast<const KornStringId*>( item )->getId();
	
	protocol->deleteMailKURL( kurl, metadata );
	
	if( kurl.port() == 0 )
		kurl.setPort( protocol->defaultPort() );
		
	if( protocol->deleteFunction() == KIO_Protocol::get )
	{
		kdDebug() << "KIO::get( " << kurl << ", true, false )" << endl;
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
		
	connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotResult( KIO::Job* ) ) );
		
	job->addMetaData( metadata );
	
	_jobs->append( dynamic_cast< KIO::Job* >( job ) );
}

/*
 * Some protocols needs to a command to commit protocols.
 */
void KIO_Delete::commitDelete( KURL kurl, KIO::MetaData metadata, KIO_Protocol *& protocol )
{
	protocol->deleteCommitKURL( kurl, metadata );
	
	if( kurl.port() == 0 )
		kurl.setPort( protocol->defaultPort() );
	
	kdDebug() << "KIO::get( " << kurl << ", true, false )" << endl;
	KIO::TransferJob *job = KIO::get( kurl, true, false );
	job->addMetaData( metadata );
	connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotResult( KIO::Job* ) ) );
	
	_jobs->append( dynamic_cast< KIO::Job* >( job ) );
	
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

void KIO_Delete::slotResult( KIO::Job* job )
{
	if( job->error() )
	{
		kdWarning() << i18n( "An error occurred when deleting email: %1." ).arg( job->errorString() ) << endl;
		_valid = false;
	}
	
	_jobs->remove( job );
	
	_kio->emitDeleteMailsProgress( _total - _jobs->count() );
	
	if( _jobs->isEmpty() )
	{
		_kio->emitDeleteMailsReady( _valid );
		disConnect();
	}
}

