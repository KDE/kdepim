#include "kio_subjects.h"

#include "kio.h"
#include "kio_single_subject.h"
#include "kio_proto.h"
#include "mailsubject.h"

#include <kio/global.h>
#include <kio/scheduler.h>
#include <kdebug.h>

#include <qptrlist.h>
#include <qvaluelist.h>
#include <qstring.h>

KIO_Subjects::KIO_Subjects( QObject * parent, const char * name )
	: QObject( parent, name ),
	_protocol( 0 ),
	_slave( 0 ),
	_valid( true )
{
	_jobs = new QPtrList<KIO_Single_Subject>;
	_kurl = new KURL;
	_metadata = new KIO::MetaData;
	
	_jobs->setAutoDelete( true );
}

KIO_Subjects::~KIO_Subjects( )
{
	delete _jobs;
	delete _kurl;
	delete _metadata;
	delete _protocol;
	_protocol = 0;
}

void KIO_Subjects::doReadSubjects( KKioDrop *drop )
{
	QValueList<KKioDrop::FileInfo>::ConstIterator it;
	QValueList<KKioDrop::FileInfo>::ConstIterator end_it = drop->_mailurls->end();
	
	_kio = drop;
	delete _protocol;
	_protocol = _kio->_protocol->clone();
	*_kurl = *_kio->_kurl;
	*_metadata = *_kio->_metadata;

	if( _jobs->count() > 0 )
		kdWarning() << i18n( "Already a slave pending." ) << endl;
		
	_jobs->clear( );
	
	//Open connection
	getConnection( );
	
	//Open jobs for easy item in the list
	for( it = _kio->_mailurls->begin(); it != end_it; it++ )
		startJob( (*it).name, (*it).size );
	
	//close connection for trivial situations (empty list)
	disConnect( true );
	
	//passing number of subjects for progress bar.
	_kio->emitReadSubjectsTotalSteps( _jobs->count() );
}

void KIO_Subjects::getConnection( )
{
	KURL kurl = *_kurl;
	KIO::MetaData metadata = *_metadata;

	if( _slave )
		KIO::Scheduler::disconnectSlave( _slave );
	
	if( _protocol->connectionBased( ) )
	{
		_protocol->readSubjectConnectKURL( kurl, metadata );
		
		if( kurl.port() == 0 )
			kurl.setPort( _protocol->defaultPort() );
		
		if( ! ( _slave = KIO::Scheduler::getConnectedSlave( kurl, metadata ) ) )
		{
			kdWarning() << i18n( "Not able to open a kio-slave for %1." ).arg( _protocol->configName() );
			_valid = false;
			_kio->emitReadSubjectsReady( false );
			return;
		}
	}
}

void KIO_Subjects::startJob( const QString &name, const long size )
{
	KURL kurl = *_kurl;
	KIO::MetaData metadata = *_metadata;
	KIO_Single_Subject *subject;
	
	kurl = name;
	
	_protocol->readSubjectKURL( kurl, metadata );
	
	if( kurl.port() == 0 )
		kurl.setPort( _protocol->defaultPort() );
	
	subject = new KIO_Single_Subject( this, name.latin1(), kurl, metadata, _protocol, _slave, name, size );
	
	connect( subject, SIGNAL( readSubject( KornMailSubject* ) ), this, SLOT( slotReadSubject( KornMailSubject* ) ) );
	connect( subject, SIGNAL( finished( KIO_Single_Subject* ) ), this, SLOT( slotFinished( KIO_Single_Subject* ) ) );
	
	_jobs->append( subject );
}

void KIO_Subjects::disConnect( bool result )
{
	if( _jobs->isEmpty() )
	{
		KIO::Scheduler::disconnectSlave( _slave );
		_slave = 0;
		_kio->emitReadSubjectsReady( result );
	}
}

void KIO_Subjects::cancelled( )
{
	_jobs->clear();
	disConnect( false );
}

void KIO_Subjects::slotReadSubject( KornMailSubject* subject )
{
	_valid = true;
	_kio->emitReadSubjectsRead( subject );
}

void KIO_Subjects::slotFinished( KIO_Single_Subject* item )
{
	//Remove sender.... I didn't know of the computer gonna like me, but it seems he does :)
	_jobs->remove( item );
	
	_kio->emitReadSubjectsProgress( _jobs->count( ) );
	
	disConnect( true ); //Only works when all jobs are finished.
}

#include "kio_subjects.moc"
