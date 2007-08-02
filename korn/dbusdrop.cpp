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

#include "dbusdrop.h"
#include "maildropadaptor.h"

#include "mailsubject.h"
#include "settings.h"

#include <kconfigbase.h>
#include <kdebug.h>


#include <QDateTime>
#include <QList>
#include <QMap>
#include <QString>
#include <QTimer>

DBUSDrop::DBUSDrop()
	: KMailDrop(),
	_isRunning( false ),
	_msgList( new QMap< int, KornMailSubject* > ),
	_name( new QString( "" ) ),
	_counter( 1 )
{
	new MailDropAdaptor( this );
}

DBUSDrop::~DBUSDrop()
{
	eraseList();
	delete _msgList;
	delete _name;
}

void DBUSDrop::recheck()
{
	emit changed( _msgList->count(), this );
	emit rechecked();
}

bool DBUSDrop::startMonitor()
{
	_isRunning  = true;
	return true;
}

bool DBUSDrop::stopMonitor()
{
	_isRunning = false;
	return true;
}

bool DBUSDrop::readConfig( AccountSettings *cfg )
{
	return KMailDrop::readConfig( cfg );
}

bool DBUSDrop::readConfigGroup( const QMap< QString, QString > &map, const Protocol * protocol )
{
	if( !map.contains( "dbusname" ) )
		//The mapping MUST contain dbusname.
		kDebug() <<"mapping is niet compleet";

	this->setDBUSName( *map.find( "dbusname" ) );
	
	return true;
}

//bool DBUSDrop::writeConfigGroup( KConfigBase& cfg ) const
//{
//	KMailDrop::writeConfigGroup( cfg );
//	
//	cfg.writeEntry( "dbusname", *_name );
//	return true;
//}

QString DBUSDrop::type() const
{
	return QString( "dbus" );
}

QVector< KornMailSubject >* DBUSDrop::doReadSubjects( bool * )
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
	QVector<KornMailSubject> *vector = new QVector<KornMailSubject>( );
        vector->reserve( _msgList->count() );
        return vector;
}

bool DBUSDrop::deleteMails( QList<QVariant> * ids, bool * )
{
	emit deleteMailsTotalSteps( 1 );
	
	for( int xx = 0; xx < ids->size(); ++xx )
	{
		if( ids->at( xx ).type() == QVariant::Int )
		{
			if( _msgList->contains( ids->at( xx ).toInt() ) )
				_msgList->remove( ids->at( xx ).toInt() );
		} else {
			kDebug() <<"Got a non-int in DBUSDrop::deleteMails()!";
		}
		
	}
	
	emit deleteMailsProgress( 1 );
	emit deleteMailsReady( true );
	
	return true;
}

void DBUSDrop::eraseList( void )
{
	QMap<int, KornMailSubject* >::iterator it;
	for( it = _msgList->begin(); it != _msgList->end(); ++it )
		delete it.value();
	_msgList->clear();
}

void DBUSDrop::doReadSubjectsASync( void )
{
	QMap<int, KornMailSubject* >::iterator it;
	for( it = _msgList->begin(); it != _msgList->end(); ++it )
		emit readSubject( new KornMailSubject( *it.value() ) );
	emit readSubjectsProgress( 1 );
	emit readSubjectsReady( true );
}

int DBUSDrop::addMessage( const QString& subject, const QString& message )
{
	int id = _counter++;

	KornMailSubject *mailsubject = new KornMailSubject( QVariant( id ), this );
	mailsubject->setSubject( subject );
	mailsubject->setSender( QString( "DBUS: %1" ).arg( *_name ) );
	mailsubject->setHeader( message, true );
	mailsubject->setSize( message.length() );
	mailsubject->setDate( QDateTime::currentDateTime().toTime_t() );
	
	_msgList->insert( id, mailsubject );
	
	emit changed( _msgList->count(), this );

	if( _settings->passivePopup() )
	{
		QList< KornMailSubject > list;
		list.append( *mailsubject );
		emit showPassivePopup( &list, 1, _settings->passiveDate(), this->realName() );
	}
	
	return id;
}

bool DBUSDrop::removeMessage( int id )
{
	if( ! _msgList->contains( id ) )
		return false;
	
	delete (*_msgList)[ id ];
	_msgList->remove( id );
	
	emit changed( _msgList->count(), this );
	
	return true;
}

QString DBUSDrop::DBUSName() const
{
	return *_name;
}

void DBUSDrop::setDBUSName( const QString& name)
{
#ifdef __GNUC__
#warning Port me (using DBus object path?)
#endif
	*_name = name;
//	if( _interface )
//		_interface->changeName( name );
//	else
//		_interface = new DBUSDropInterface( this, name.toUtf8() );
	QDBusConnection::sessionBus().registerObject( '/' + name, this, QDBusConnection::ExportAdaptors );
}

#include "dbusdrop.moc"
