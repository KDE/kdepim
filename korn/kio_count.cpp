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

#include "kio_count.h"

#include "kio.h"
#include "kio_conn.h"
#include "kio_proto.h"
#include "kio_single_subject.h"
#include "mailsubject.h"

#include <kdebug.h>
#include <kio/jobclasses.h>
#include <kio/scheduler.h>
#include <kio/slave.h>
#include <kio/global.h>
#include <kurl.h>

#include <qstring.h>

KIO_Count::KIO_Count( QObject * parent )
	: QObject ( parent ),
	_kurl( 0 ),
	_metadata( 0 ),
	_protocol( 0 ),
	_valid( true ),
	_new_mailurls( 0 ),
	_subjects_pending( 0 ),
	_total_new_messages( 0 ),
	_popup_subjects( 0 )
{
}

KIO_Count::~KIO_Count()
{
	// Delete copies of urls.
	delete _kurl;
	delete _metadata;
}

void KIO_Count::count( KKioDrop *drop )
{
	if( _new_mailurls )
		return; //A counting is pending, so no new one is started.

	delete _kurl;
	delete _metadata;
	_kio = drop;

	/*
	 * Saving current settings: all actions are asynchroon, so if someone
	 * use slow servers, settings could been changed before this class is
	 * finished with counten. To be able to track back te staring values;
	 * these are saved in the class.
	 */
	_kurl = new KUrl( *_kio->_kurl );
	_metadata = new KIO::MetaData( *_kio->_metadata );
	_protocol = _kio->_protocol;

	KUrl kurl = *_kurl;
	KIO::MetaData metadata = *_metadata;

	// Serup a connection
	if( _protocol->connectionBased( ) )
	{
		_protocol->recheckConnectKUrl( kurl, metadata );

		if( kurl.port() == 0 )
			kurl.setPort( _protocol->defaultPort( _kio->_ssl ) );

		//if( ! ( _slave = KIO::Scheduler::getConnectedSlave( kurl, metadata ) ) ) //Forcing reload
		if( ! ( _slave = KIO_Connection::getSlave( kurl, metadata, _protocol ) ) ) //Reusing connection
		{
			kWarning() << i18n( "Not able to open a kio slave for %1." ).arg( _protocol->configName() ) << endl;
			_kio->emitShowPassivePopup( i18n( "Not able to open a kio slave for %1." ).arg( _protocol->configName() ) );
			_valid = false;
			_kio->emitValidChanged();
			_slave = 0;
			//delete _new_mailurls; _new_mailurls = 0; //No connection pending
			return;
		}

		connect( _slave, SIGNAL( error( int, const QString& ) ), _kio, SLOT( slotConnectionError( int, const QString& ) ) );
		connect( _slave, SIGNAL( warning( const QString& ) ), _kio, SLOT( slotConnectionWarning( const QString& ) ) );
		connect( _slave, SIGNAL( infoMessage( const QString& ) ), _kio, SLOT( slotConnectionInfoMessage( const QString& ) ) );

		/*
		 * _protocol->recheckConnectKUrl could have change kurl and metadata in order to have the right
		 * settings to connect. But some other functions assumed unmodified settings,
		 * so the settings are set back to his originals.
		 */
		kurl = *_kurl;
		metadata = *_metadata;
	}
	else
	{
		_slave = 0; //Prevent disconnecting not-existing slave
	}

	/* Blocking this function: no new counts can be started from now */
	_new_mailurls = new QList< KKioDrop::FileInfo >;

	_protocol->recheckKUrl( kurl, metadata );

	if( kurl.port() == 0 )
		kurl.setPort( _protocol->defaultPort( _kio->_ssl ) );

	//Making job to fetch file-list
	
	_job = KIO::listDir( kurl, false );
	_job->addMetaData( metadata );

	connect( _job, SIGNAL( result( KIO::Job* ) ), this, SLOT( result( KIO::Job* ) ) );
	connect( _job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
	         this, SLOT( entries( KIO::Job*, const KIO::UDSEntryList& ) ) );

	if( _protocol->connectionBased() )
		KIO::Scheduler::assignJobToSlave( _slave, _job );
	else
		KIO::Scheduler::scheduleJob( _job );
}

void KIO_Count::stopActiveCount()
{
	if( !_new_mailurls )
		return;

	disconnect( _job, SIGNAL( result( KIO::Job* ) ), this, SLOT( result( KIO::Job* ) ) );
	disconnect( _job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
	            this, SLOT( entries( KIO::Job*, const KIO::UDSEntryList& ) ) );

	KIO::Scheduler::cancelJob( _job );

	if( _slave )
	{
		//Slave seems to be disconnected by canceling the last job of the slave
		//KIO::Scheduler::disconnectSlave( _slave );
		_slave = 0;
	}

	//Deletings settings
	delete _kurl; _kurl = 0;
	delete _metadata; _metadata = 0;

	delete _new_mailurls; _new_mailurls = 0;
}

void KIO_Count::showPassive( const QString& id )
{
	KUrl kurl = *_kio->_kurl;
	KIO::MetaData metadata = *_kio->_metadata;
	kurl = id;
	//KIO::Slave *slave = 0;

	_kio->_protocol->readSubjectKUrl( kurl, metadata );
	if( kurl.port() == 0 )
		kurl.setPort( _kio->_protocol->defaultPort( _kio->_ssl ) );

	KIO_Single_Subject *subject = new KIO_Single_Subject( this, kurl, metadata, _kio->_protocol, _slave, id, 0 );

	_subjects_pending++;

	connect( subject, SIGNAL( readSubject( KornMailSubject* ) ), this, SLOT( addtoPassivePopup( KornMailSubject* ) ) );
        connect( subject, SIGNAL( finished( KIO_Single_Subject* ) ), this, SLOT( deleteSingleSubject( KIO_Single_Subject* ) ) );
}

void KIO_Count::disconnectSlave()
{
	if( _subjects_pending > 0 )
		return; //Still getting data

	if( !_protocol->connectionBased() )
		return; //Protocol doesn't have a connection

	if( !_slave )
		return; //Slave doens't exist

	//Disconnect slave
	//KIO::Scheduler::disconnectSlave( _slave );
	KIO_Connection::removeSlave( _slave );
	_slave = 0;
	_protocol = 0;
}

//This function is called when fetching is over
void KIO_Count::result( KIO::Job* job )
{
	//job should be the latest job; elsewise: print an error.
	if( job != _job )
		kError() << i18n( "Got unknown job; something must be wrong..." ) << endl;

	//look of an error occurred. If there is, print the error.
	//This could be very useful by resolving bugs.
	if( job->error() )
	{
		kError() << i18n( "The next KIO-error occurred by counting: %1" ).arg( job->errorString() ) << endl;
		_kio->emitShowPassivePopup( i18n( "The next KIO-error occurred by counting: %1" ).arg( job->errorString() ) );
		_valid = false;
		_kio->emitValidChanged();
	}

	disconnect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( result( KIO::Job* ) ) );
	disconnect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
	            this, SLOT( entries( KIO::Job*, const KIO::UDSEntryList& ) ) );

	disconnectSlave();

	//Deletings settings
	delete _kurl; _kurl = 0;
	delete _metadata; _metadata = 0;

	if( _kio->_mailurls->count() != _new_mailurls->count() )
	{
		*_kio->_mailurls = *_new_mailurls;
		_kio->emitChanged(_kio->_mailurls->count());
	}
	else
	{
		*_kio->_mailurls = *_new_mailurls;
	}
	delete _new_mailurls; _new_mailurls = 0;

	_valid = true;
	_kio->emitValidChanged();
	_kio->emitRechecked();
}

//An file list is ready; now save it in _kio->_mailurls.
void KIO_Count::entries( KIO::Job* job, const KIO::UDSEntryList &list )
{
	QStringList old_list;
	KIO::UDSEntryList::ConstIterator  it1 ;
	//KIO::UDSEntry::ConstIterator it2 ;
	KIO::MetaData metadata;
	KUrl kurl;
	bool isFile;

	//job should be the latest job
	if( job != _job )
		kError() << i18n( "Got unknown job; something must be wrong..." ) << endl;

	for( QList<KKioDrop::FileInfo>::ConstIterator it = _kio->_mailurls->begin(); it != _kio->_mailurls->end(); ++it )
		old_list.append( (*it).name );

	for ( it1 = list.begin() ; it1 != list.end() ; it1++ )
	{
		/*
		 * The list contains multiple objects. Each object could be a file.
		 * Settings about it are saved in this scope until it is added to the list.
		 */
		isFile=false;
		KKioDrop::FileInfo fileinfo;
		fileinfo.name = (*it1).stringValue( KIO::UDS_URL );
		fileinfo.size = (*it1).numberValue( KIO::UDS_SIZE, 0 );

		if( (*it1).contains( KIO::UDS_NAME ) && fileinfo.name.isNull() )
		{ //The file kioslave doesn't return UDS_URL, so use UDS_NAME.
			kurl = *_kurl;
			metadata = *_metadata;
			_protocol->recheckKUrl( kurl, metadata );
			kurl.addPath( (*it1).stringValue( KIO::UDS_NAME ) );
			fileinfo.name = kurl.url();
		}

		isFile = (*it1).numberValue( KIO::UDS_FILE_TYPE, 0 ) & S_IFREG;
		/*for ( it2 = (*it1).begin() ; it2 != (*it1).end() ; it2++ )
		{
			if( (*it2).m_uds == KIO::UDS_FILE_TYPE &&
			   ((long)(*it2).m_long & S_IFREG ) )
				isFile=true;
			else if( (*it2).m_uds == KIO::UDS_URL )
				fileinfo.name = (*it2).m_str;
			else if( (*it2).m_uds == KIO::UDS_NAME )
			{ //The file kioslave doesn't return UDS_URL.
				kurl = *_kurl;
				metadata = *_metadata;
				_protocol->recheckKUrl( kurl, metadata );
				kurl.addPath ( (*it2).m_str );
				fileinfo.name = kurl.url();
			}
			else if( (*it2).m_uds == KIO::UDS_SIZE )
			{
				fileinfo.size = (*it2).m_long;
			}
		}*/

		//Add the entry.
		if( ! fileinfo.name.isNull() && isFile )
		{
			_new_mailurls->append( fileinfo );
			if( ! old_list.contains( fileinfo.name ) && _kio->passivePopup() )
				showPassive( fileinfo.name );
		}
	}
}

void KIO_Count::addtoPassivePopup( KornMailSubject* subject )
{
	if( ! _popup_subjects )
		_popup_subjects = new QList< KornMailSubject >;

	_popup_subjects->append( *subject );

	_subjects_pending--;
	_total_new_messages++;
	if( _subjects_pending == 0 )
	{
		qSort( _popup_subjects->begin(), _popup_subjects->end() );
		_kio->emitShowPassivePopup( _popup_subjects, _total_new_messages );
		delete _popup_subjects; _popup_subjects = 0;
		_total_new_messages = 0;

		disconnectSlave();
	}
}

void KIO_Count::deleteSingleSubject( KIO_Single_Subject* single_subject )
{
	delete single_subject;
}

#include "kio_count.moc"
