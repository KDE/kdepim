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

#include "kio_count.h"

#include "kio.h"
#include "kio_proto.h"
#include "kio_single_subject.h"
#include "mailsubject.h"
#include "sortedmailsubject.h"

#include <kdebug.h>
#include <kio/jobclasses.h>
#include <kio/scheduler.h>
#include <kio/global.h>
#include <kurl.h>
#include <qvaluelist.h>

#include <qstring.h>

KIO_Count::KIO_Count( QObject * parent, const char * name )
	: QObject ( parent, name ),
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
	delete _protocol;
}

void KIO_Count::count( KKioDrop *drop )
{
	if( _new_mailurls )
		return; //A counting is pending, so no new one is started.
		
	delete _kurl;
	delete _metadata;
	delete _protocol;
	_kio = drop;
	
	/*
	 * Saving current settings: all actions are asynchroon, so if someone
	 * use slow servers, settings could been changed before this class is
	 * finished with counten. To be able to track back te staring values;
	 * these are saved in the class.
	 */
	_kurl = new KURL( *_kio->_kurl );
	_metadata = new KIO::MetaData( *_kio->_metadata );
	_protocol = _kio->_protocol->clone();
	
	KURL kurl = *_kurl;
	KIO::MetaData metadata = *_metadata;
	
	// Serup a connection
	if( _protocol->connectionBased( ) )
	{
		_protocol->recheckConnectKURL( kurl, metadata );
		
		if( kurl.port() == 0 )
			kurl.setPort( _protocol->defaultPort() );
			
		if( ! ( _slave = KIO::Scheduler::getConnectedSlave( kurl, metadata ) ) ) //Forcing reload
		{
			kdWarning() << i18n( "Not able to open a kio slave for %1." ).arg( _protocol->configName() ) << endl;
			_valid = false;
			_slave = 0;
			//delete _new_mailurls; _new_mailurls = 0; //No connection pending
			return;
		}
		
		/*
		 * _protocol->recheckConnectKURL could have change kurl and metadata in order to have the right
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
	_new_mailurls = new QValueList< KKioDrop::FileInfo >;
	
	_protocol->recheckKURL( kurl, metadata );
	
	if( kurl.port() == 0 )
		kurl.setPort( _protocol->defaultPort() );
		
	//Making job to fetch file-list
	_job = KIO::listDir( kurl, false );
	_job->addMetaData( metadata );
	
	connect( _job, SIGNAL( result( KIO::Job* ) ), this, SLOT( result( KIO::Job* ) ) );
	connect( _job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
	         this, SLOT( entries( KIO::Job*, const KIO::UDSEntryList& ) ) );
		 
	if( _protocol->connectionBased( ) )
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
	delete _protocol; _protocol = 0;
	
	delete _new_mailurls; _new_mailurls = 0;
}

void KIO_Count::showPassive( const QString& id )
{
	KURL kurl = *_kio->_kurl;
	KIO::MetaData metadata = *_kio->_metadata;
	kurl = id;
	KIO::Slave *slave = 0;
	
	_kio->_protocol->readSubjectKURL( kurl, metadata );
	if( kurl.port() == 0 )
		kurl.setPort( _kio->_protocol->defaultPort() );
	
	KIO_Single_Subject *subject = new KIO_Single_Subject( this, id.latin1(), kurl, metadata, _kio->_protocol, slave, id, 0 );
	
	_subjects_pending++;
	
	connect( subject, SIGNAL( readSubject( KornMailSubject* ) ), this, SLOT( addtoPassivePopup( KornMailSubject* ) ) );
        connect( subject, SIGNAL( finished( KIO_Single_Subject* ) ), this, SLOT( deleteSingleSubject( KIO_Single_Subject* ) ) );
}

//This function is called when fetching is over
void KIO_Count::result( KIO::Job* job )
{
	//job should be the latest job; elsewise: print an error.
	if( job != _job )
		kdError() << i18n( "Got unknown job; something must be wrong..." ) << endl;
	
	//look of an error occured. If there is, print the error.
	//This could be very useful by resolving bugs.
	if( job->error() )
		kdError() << i18n( "The next KIO-error occurred by counting: %1" ).arg( job->errorText() ) << endl;
		
	disconnect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( result( KIO::Job* ) ) );
	disconnect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
	            this, SLOT( entries( KIO::Job*, const KIO::UDSEntryList& ) ) );
		    
	if( _protocol->connectionBased( ) && _slave )
	{
		KIO::Scheduler::disconnectSlave( _slave );
		_slave = 0;
	}
	
	//Deletings settings
	delete _kurl; _kurl = 0;
	delete _metadata; _metadata = 0;
	delete _protocol; _protocol = 0;
	
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
	
	_kio->emitRechecked();
}

//An file list is ready; now save it in _kio->_mailurls.
void KIO_Count::entries( KIO::Job* job, const KIO::UDSEntryList &list )
{
	QStringList old_list;
	KIO::UDSEntryListConstIterator  it1 ;
	KIO::UDSEntry::ConstIterator it2 ;
	KIO::MetaData metadata;
	KURL kurl;
	bool isFile;
		
	//job should be the latest job
	if( job != _job )
		kdError() << i18n( "Got unknown job; something must be wrong..." ) << endl;
		
	for( QValueListConstIterator<KKioDrop::FileInfo> it = _kio->_mailurls->begin(); it != _kio->_mailurls->end(); ++it )
		old_list.append( (*it).name );
	
	for ( it1 = list.begin() ; it1 != list.end() ; it1++ )
	{
		/*
		 * The list contains multiple objects. Each object could be a file.
		 * Settings about it are saved in this scope until it is added to the list.
		 */
		isFile=false;
		KKioDrop::FileInfo fileinfo;
		fileinfo.name = QString::null;
		fileinfo.size = 0;
		
		for ( it2 = (*it1).begin() ; it2 != (*it1).end() ; it2++ )
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
				_protocol->recheckKURL( kurl, metadata );
				kurl.setPath ( kurl.path() + '/' + (*it2).m_str );
				fileinfo.name = kurl.url();
			}
			else if( (*it2).m_uds == KIO::UDS_SIZE )
			{
				fileinfo.size = (*it2).m_long;
			}
		}

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
	{
		_popup_subjects = new SortedMailSubject;
		_popup_subjects->setAutoDelete( true );
	}
	
	_popup_subjects->inSort( subject );
	if( _popup_subjects->count() > 5 )
		_popup_subjects->removeFirst(); //Overhead: subject is downloaded
	
	_subjects_pending--;
	_total_new_messages++;
	if( _subjects_pending == 0 )
	{
		_kio->emitShowPassivePopup( dynamic_cast< QPtrList<KornMailSubject>* >( _popup_subjects ), _total_new_messages );
		delete _popup_subjects; _popup_subjects = 0;
		_total_new_messages = 0;
	}
}

void KIO_Count::deleteSingleSubject( KIO_Single_Subject* single_subject )
{
	delete single_subject;
}

#include "kio_count.moc"
