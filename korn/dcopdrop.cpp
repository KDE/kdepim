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

#include "dcopdrop.h"

#include "dcopdropif.h"
#include "dropdlg.h"
#include "intid.h"
#include "mailid.h"
#include "mailsubject.h"

#include <kconfigbase.h>
#include <kdebug.h>

#include <qdatetime.h>
#include <qmap.h>
#include <qstring.h>
#include <qtimer.h>

DCOPDrop::DCOPDrop()
	: KMailDrop(),
	_isRunning( false ),
	_msgList( new QMap< int, KornMailSubject* > ),
	_name( new QString( "" ) ),
	_counter( 1 ),
	_interface( 0 )
{
}

DCOPDrop::~DCOPDrop()
{
	eraseList();
	delete _interface;
	delete _msgList;
	delete _name;
}

void DCOPDrop::recheck()
{
	emit changed( _msgList->count(), this );
	emit rechecked();
}

bool DCOPDrop::startMonitor()
{
	_isRunning  = true;
	return true;
}

bool DCOPDrop::stopMonitor()
{
	_isRunning = false;
	return true;
}

bool DCOPDrop::readConfigGroup( const KConfigBase& cfg )
{
	return KMailDrop::readConfigGroup( cfg );
}

bool DCOPDrop::writeConfigGroup( KConfigBase& cfg ) const
{
	KMailDrop::writeConfigGroup( cfg );

	KMailDrop::writeConfigGroup( cfg );
	
	cfg.writeEntry( "dcopname", *_name );
	return true;
}

QString DCOPDrop::type() const
{
	return QString( "dcop" );
}

QValueVector< KornMailSubject >* DCOPDrop::doReadSubjects( bool * )
{
	emit readSubjectsTotalSteps( 1 );
	
	/*
	 * This way, the function is really asynchrone.
	 * So, the return value arraves before any data arrives.
	 */
	QTimer::singleShot( 1, this, SLOT( doReadSubjectsASync( void ) ) );
	
	/*
         * A empty QValueVector is made here.
         * After that, the size is expanded to the expected number of subjects.
         * This way, reallocation of memmory is minimized, and thus more efficient.
         */
	QValueVector<KornMailSubject> *vector = new QValueVector<KornMailSubject>( );
        vector->reserve( _msgList->count() );
        return vector;
}

bool DCOPDrop::deleteMails( QPtrList<const KornMailId> * ids, bool * )
{
	emit deleteMailsTotalSteps( 1 );
	
	for( const KornMailId *it = ids->first(); it; it = ids->next() )
	{
		const KornIntId* id = dynamic_cast< const KornIntId* >( it );
		if( _msgList->contains( id->getId() ) )
			_msgList->erase( id->getId() );
	}
	
	emit deleteMailsProgress( 1 );
	emit deleteMailsReady( true );
	
	return true;
}

void DCOPDrop::eraseList( void )
{
	QMap<int, KornMailSubject* >::iterator it;
	for( it = _msgList->begin(); it != _msgList->end(); ++it )
		delete it.data();
	_msgList->clear();
}

void DCOPDrop::doReadSubjectsASync( void )
{
	QMap<int, KornMailSubject* >::iterator it;
	for( it = _msgList->begin(); it != _msgList->end(); ++it )
		emit readSubject( new KornMailSubject( *it.data() ) );
	emit readSubjectsProgress( 1 );
	emit readSubjectsReady( true );
}

int DCOPDrop::addMessage( const QString& subject, const QString& message )
{
	KornIntId *id = new KornIntId( _counter );
	KornMailSubject *mailsubject = new KornMailSubject( id, this );
	++_counter;
	
	mailsubject->setSubject( subject );
	mailsubject->setSender( QString( "DCOP: %1" ).arg( *_name ) );
	mailsubject->setHeader( message, true );
	mailsubject->setSize( message.length() );
	mailsubject->setDate( QDateTime::currentDateTime().toTime_t() );
	
	_msgList->insert( id->getId(), mailsubject );
	
	emit changed( _msgList->count(), this );
	
	return _counter - 1;
}

bool DCOPDrop::removeMessage( int id )
{
	if( ! _msgList->contains( id ) )
		return false;
	
	delete (*_msgList)[ id ];
	_msgList->erase( id );
	
	emit changed( _msgList->count(), this );
	
	return true;
}

QString DCOPDrop::DCOPName() const
{
	return *_name;
}

void DCOPDrop::setDCOPName( const QString& name)
{
	*_name = name;
	if( _interface )
		_interface->changeName( name );
	else
		_interface = new DCOPDropInterface( this, name.utf8() );
}

#include "dcopdrop.moc"
