/*
 * Copyright (C)       Sirtaj Singh Kang
 * Copyright (C)       Kurt Granroth
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

/*
 * kio.cpp -- Implementation of class KKIODrop.
 * It is copyied from imap.cpp, with was writed by:
 * Author:	Kurt Granroth
 * Version:	$Id$
 * Changed by:
 * Mart Kelder <mart.kde@hccnet.nl>, 2004
 */

#include "kio.h"
#include "kio_count.h"
#include "kio_subjects.h"
#include "kio_read.h"
#include "kio_delete.h"
#include "protocol.h"
#include "protocols.h"
#include"utils.h"
#include "mailsubject.h"

#include<kconfig.h>
#include<kconfigbase.h>
#include<kdebug.h>
#include<klocale.h>

#include<qlist.h>
#include<qregexp.h>
#include<qlist.h>
#include <QVariant>
#include<qvector.h>

#include<assert.h>
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>

//Headers of protocols
#include"kio_proto.h"
//#include"pop3_proto.h"
//#include"pop3s_proto.h"
//#include"imap_proto.h"
//#include"imaps_proto.h"
//#include"nntp_proto.h"
//#include"maildir_proto.h"
//#include"qmail_proto.h"
//#include"process_proto.h"
#include"mbox_proto.h"

/*
 * The 'process' maildrop is a lot different than the other protocols:
 * it haven't a kioslave and could not result in file list. To prevent
 * trowing it away, that functionality is hacked in this file.
 */

KKioDrop::KKioDrop()
	: KPollableDrop(),
	_kurl( 0 ),
	_metadata( 0 ),
	_valid(false),
	_protocol( 0 ),
	_ssl(false),
	_count( 0 ),
	_subjects( 0 ),
	_read( 0 ),
	_readSubjectsTotalSteps( 0 ),
	_deleteMailsTotalSteps( 0 ),
	_mailurls( 0 )
{
	_kurl = new KUrl;
	_metadata = new KIO::MetaData;

	//Initialising protocol; if no protocol is set before first use, it will use the first protocol
	_protocol = Protocols::firstProtocol()->getKIOProtocol(); //The first protocol is the default
	_kurl->setPort( _protocol->defaultPort( _ssl ) );

	//Creating children and connect them to the outside world; this class passes the messages for them...
	//This class handles all the counting.
	_count = new KIO_Count( this );

	//This class is responsible for providing the available subjects
	_subjects = new KIO_Subjects( this );

	//This class is used when a full message has to be read.
	_read = new KIO_Read( this );

	//This class can delete mails.
	_delete = new KIO_Delete( this );

	_mailurls = new QList<FileInfo>;
}

KKioDrop::KKioDrop( AccountSettings* )
	: KPollableDrop(),
	_kurl( 0 ),
	_metadata( 0 ),
	_valid(false),
	_protocol( 0 ),
	_ssl(false),
	_count( 0 ),
	_subjects( 0 ),
	_read( 0 ),
	_readSubjectsTotalSteps( 0 ),
	_deleteMailsTotalSteps( 0 ),
	_mailurls( 0 )
{
	_kurl = new KUrl;
	_metadata = new KIO::MetaData;

	//Initialising protocol; if no protocol is set before first use, it will use the first protocol
	_protocol = Protocols::firstProtocol()->getKIOProtocol(); //The first protocol is the default
	_kurl->setPort( _protocol->defaultPort( _ssl ) );

	//Creating children and connect them to the outside world; this class passes the messages for them...
	//This class handles all the counting.
	_count = new KIO_Count( this );

	//This class is responsible for providing the available subjects
	_subjects = new KIO_Subjects( this );

	//This class is used when a full message has to be read.
	_read = new KIO_Read( this );

	//This class can delete mails.
	_delete = new KIO_Delete( this );

	_mailurls = new QList<FileInfo>;

	//readConfigGroup( *config );
}

void KKioDrop::setKioServer( const QString & proto, const QString & server, int port )
{
	 //Settings default for last vars; could not inline because KIO::MetaData-object is not defined in header.
	setKioServer( proto, server, port, KIO::MetaData(), false, true );
}

void KKioDrop::setKioServer(const QString & proto, const QString & server, int port, const KIO::MetaData metadata, bool ssl,
		            bool setProtocol )
{
	QString auth;

	if( port == -1 )
		port = _protocol->defaultPort( ssl );

	if( setProtocol ) //false if _protocol already made
	{
		_protocol = Protocols::getProto( proto )->getKIOProtocol();

		if( ! _protocol )
			_protocol = Protocols::firstProtocol()->getKIOProtocol();
	}

	_kurl->setProtocol( _protocol->protocol( ssl ) );
	_kurl->setHost    ( server );
	_kurl->setPort    ( port );
	_ssl = ssl;

	//Checking for authentication-settings.
	//if( _metadata->contains("auth") )
	//{
	//	auth = (*_metadata)["auth"];
	//	*_metadata = metadata;
	//	if( ! _metadata->contains("auth") )
	//		(*_metadata)["auth"] = auth;
	//} else
	*_metadata = metadata;

	_count->stopActiveCount();
}

void KKioDrop::setUser(const QString & user, const QString & password,
	const QString & mailbox, const QString & auth )
{
	_kurl->setUser( user );
	_password = password ;
	_kurl->setPass( _password );
	_kurl->setPath( mailbox );
	if( ! auth.isEmpty() && auth != "Plain" )
		(*_metadata)["auth"] = auth;
	else if( _metadata->contains( "auth" ) )
		_metadata->remove( "auth" );

	_valid = _kurl->isValid();
	emit validChanged( valid() );

	if( ! _valid )
		kWarning() << i18n("url is not valid" );

	_count->stopActiveCount();
}

QString KKioDrop::protocol() const
{
	return _protocol->configName();
}

QString KKioDrop::server() const
{
	return _kurl->host();
}
int KKioDrop::port() const
{
	return _kurl->port();
}

QString KKioDrop::user() const
{
	return _kurl->user();
}
QString KKioDrop::password() const
{
	return _password ;
}
QString KKioDrop::mailbox() const
{
	return _kurl->path();
}
QString KKioDrop::auth() const
{
	return ( _metadata->contains("auth")?(*_metadata)["auth"]:"" );
}

void KKioDrop::recheck()
{
	_count->count( this, _settings );

	return;
}

void KKioDrop::forceRecheck()
{
	_count->stopActiveCount();
	_count->count( this, _settings );

	return;
}

bool KKioDrop::valid()
{
	return _valid && _count->valid() && _subjects->valid();
}

KKioDrop::~KKioDrop()
{
	delete _count;
	delete _subjects;
	delete _kurl;
	delete _metadata;
	delete _mailurls;
}

bool KKioDrop::canReadSubjects( )
{
	return (_protocol!=0?_protocol->canReadSubjects():false);
}

QVector<KornMailSubject> * KKioDrop::doReadSubjects(bool * )
{
	_subjects->doReadSubjects( this );

	/*
	 * A empty QValueVector is made here.
	 * After that, the size is expanded to the expected number of subjects.
	 * This way, reallocation of memmory is minimized, and thus more efficient.
	 */
	QVector<KornMailSubject> *vector = new QVector<KornMailSubject>( );
	vector->reserve( _mailurls->count() );
	return vector;
}

bool KKioDrop::canReadMail( ) const
{
	return (_protocol!=0?_protocol->canReadMail():false);
}

bool KKioDrop::deleteMails(QList<QVariant> * ids, bool * /*stop*/)
{
	_delete->deleteMails( ids, this );
	return _delete->valid();
}

bool KKioDrop::canDeleteMails ()
{
	return (_protocol!=0?_protocol->canDeleteMail():false);
}

QString KKioDrop::readMail(const QVariant item, bool * )
{
	_read->readMail( item, this );

	return "";
}

KMailDrop* KKioDrop::clone() const
{
	KKioDrop *clone = new KKioDrop;

	*clone = *this;

	return clone;
}

bool KKioDrop::readConfigGroup( const QMap< QString, QString > &map, const Protocol* protocol )
{
	QString val, val2;

	if( !map.contains( "server" ) || !map.contains( "port" ) || !map.contains( "ssl" ) || !map.contains( "username" ) ||
	    !map.contains( "mailbox" ) || !map.contains( "password" ) || !map.contains( "metadata" ) || !map.contains( "name" ) )
	{
		kWarning() <<"Bug: map niet compleet";
		return false;
	}

	this->setObjectName( *map.find( "name" ) );

	_protocol = protocol->getKIOProtocol();
	if( !_protocol )
		_protocol = Protocols::firstProtocol()->getKIOProtocol();

	val = *map.find( "server" );
	setKioServer( val2, val, (*map.find( "port" )).toInt(), KIO::MetaData(), *map.find( "ssl" ) == "true", false );

	_kurl->setUser( *map.find( "username" ) );
	_kurl->setPath( *map.find( "mailbox" ) );

	_kurl->setPass( *map.find( "password" ) );

	QStringList list = (*map.find( "metadata" )).split( ',', QString::SkipEmptyParts );
	QStringList::Iterator it;
	for( it = list.begin(); it != list.end(); ++it )
	{
		int split = (*it).indexOf( '=', 0, Qt::CaseSensitive );
		if( split > 0 )
			_metadata->insert( (*it).left( split ), (*it).right( (*it).length() - split - 1 ) );
	}

	_valid = true;
	emitValidChanged();

	return true;
}

bool KKioDrop::writeConfigGroup( AccountSettings *settings ) const
{
	return KPollableDrop::writeConfigGroup( settings );
}

KKioDrop& KKioDrop::operator = ( const KKioDrop& other )
{
	*_kurl=*other._kurl;

	if( other._protocol )
		_protocol = other._protocol->getKIOProtocol();
	_ssl = other._ssl;

	return *this;
}

//Public slots
void KKioDrop::readSubjectsCanceled()
{
	_subjects->canceled();
}

void KKioDrop::readMailCanceled()
{
	_read->canceled( );
}

void KKioDrop::deleteMailsCanceled()
{
	_delete->canceled( );
}

//Private slots for displaying connection errors
void KKioDrop::slotConnectionError( int number, const QString& arg )
{
	kError() << KIO::buildErrorString( number, arg );
//	if( passivePopup() )
		emitShowPassivePopup( KIO::buildErrorString( number, arg ) );
}

void KKioDrop::slotConnectionWarning( const QString& msg )
{
	kWarning() << msg;
}

void KKioDrop::slotConnectionInfoMessage( const QString& msg )
{
	kDebug() << msg; //Display only in debug modes
}

#include "kio.moc"
