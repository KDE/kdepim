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

#include "keditlistboxman.h"

#include <kconfig.h>
#include <kdebug.h>

#include <qmap.h>
#include <qstring.h>

KEditListBoxManager::KEditListBoxManager(	QWidget *parent, const char *name,
						bool checkAtEntering, int buttons )
	: KEditListBox( parent, name, checkAtEntering, buttons ),
	_config( 0 ),
	_groupName( 0 ),
	_subGroupName( 0 )
{
	init();
}

KEditListBoxManager::KEditListBoxManager(	const QString& title, QWidget *parent,
						const char *name, bool checkAtEntering,
						int buttons)
	: KEditListBox( title, parent, name, checkAtEntering, buttons ),
	_config( 0 ),
	_groupName( 0 ),
	_subGroupName( 0 )
{
	init();
}

KEditListBoxManager::KEditListBoxManager(	const QString& title,
						const KEditListBox::CustomEditor &customEditor,
						QWidget *parent, const char *name,
						bool checkAtEntering, int buttons )
	: KEditListBox( title, customEditor, parent, name, checkAtEntering, buttons ),
	_config( 0 ),
	_groupName( 0 ),
	_subGroupName( 0 )
{
	init();
}

KEditListBoxManager::~KEditListBoxManager()
{
	delete _groupName;
}

void KEditListBoxManager::setConfig( KConfig* config )
{
	_config = config;
	if( _groupName )
		readNames();
}

void KEditListBoxManager::setGroupName( const QString& name )
{
	if( _groupName )
		*_groupName = name;
	else
		_groupName = new QString( name );
	
	if( _config )
		readNames();
}

void KEditListBoxManager::setSubGroupName( const QString& name )
{
	if( _subGroupName )
		*_subGroupName = name;
	else
		_subGroupName = new QString( name );
		
	if( _config && _groupName )
		readNames();
}

void KEditListBoxManager::init()
{
	connect( this, SIGNAL( changed() ), this, SLOT( slotChanged() ) );
	connect( this, SIGNAL( added( const QString& ) ), this, SLOT( slotAdded( const QString& ) ) );
	connect( this, SIGNAL( removed( const QString& ) ), this, SLOT( slotRemoved( const QString& ) ) );
	
	connect( this->listBox(), SIGNAL( doubleClicked( QListBoxItem * ) ), this, SLOT( slotActivated( QListBoxItem * ) ) );
	connect( this->listBox(), SIGNAL( returnPressed( QListBoxItem * ) ), this, SLOT( slotActivated( QListBoxItem * ) ) );
}

void KEditListBoxManager::readNames()
{
	int number = 0;
	
	this->clear();
	while( _config->hasGroup( _groupName->arg( number ) ) )
	{
		_config->setGroup( _groupName->arg( number ) );
		this->insertItem( _config->readEntry( "name", QString::null ) );
		++number;
	}
}

void KEditListBoxManager::slotChanged()
{
	/* Three thing could be hapened:
	 * 1. the text is changed;
	 * 2. the item has moved up;
	 * 3. the item has moved down.
	 */
	
	 if( !_config || !_groupName )
	 	return;
	
	//First check if the item was moved up
	
	_config->setGroup( _groupName->arg( this->currentItem() ) );
	
	if( this->currentItem() > 0 && this->text( this->currentItem() - 1 ) == _config->readEntry( "name", QString::null ) )
		changeItem( this->currentItem() - 1, this->currentItem() ); //moved down
	else if( this->currentItem() < this->count() - 1 &&
		 this->text( this->currentItem() + 1 ) == _config->readEntry( "name", QString::null ) )
		changeItem( this->currentItem(), this->currentItem() + 1 );  //moved up
	else if( this->currentText() != _config->readEntry( "name", QString::null ) )
		changedText(); //changed
}

void KEditListBoxManager::slotAdded( const QString& name )
{
	if( !_config || !_groupName )
	 	return;
		
	int number = 0;
	while( _config->hasGroup( _groupName->arg( number ) ) )
		++number;
		
	_config->setGroup( _groupName->arg( number ) );
	_config->writeEntry( "name", name );
	
	emit setDefaults( name, number, _config );
}

void KEditListBoxManager::slotRemoved( const QString& name )
{
	if( !_config || !_groupName )
	 	return;
		
	//First: search the item number.
	int number = 0;
	int subnumber = 0;
	while( true )
	{
		if( !_config->hasGroup( _groupName->arg( number ) ) )
		{
			number = -1; //not found
			break;
		}
		_config->setGroup( _groupName->arg( number ) );
		if( name == _config->readEntry( "name", QString::null ) )
			break; //found
		
		++number; //Try next group
	}
	
	if( number < 0 ) //failure
		return; //do nothing
	
	_config->deleteGroup( _groupName->arg( number ), true, false );
	while( _subGroupName && _config->hasGroup( _subGroupName->arg( number ).arg( subnumber ) ) )
	{
		_config->deleteGroup( _subGroupName->arg( number ).arg( subnumber ) );
		++subnumber;
	}
	
	//rotate groups
	while( _config->hasGroup( _groupName->arg( number + 1 ) ) )
	{
		moveItem( number + 1, number );	
	
		++number;
	}
}

void KEditListBoxManager::slotActivated( QListBoxItem* item )
{
	if( item )
		emit activated( item->text() );
}

void KEditListBoxManager::moveItem( int src, int dest )
{
	QMap<QString, QString> *srcList = new QMap<QString, QString >;
	QMap<QString, QString>::iterator it;
	int subnumber = 0;

	*srcList = _config->entryMap( _groupName->arg( src ) );
	_config->deleteGroup( _groupName->arg( src ) );
	
	_config->setGroup( _groupName->arg( dest ) );
	for( it = srcList->begin(); it != srcList->end(); ++it )
		_config->writeEntry( it.key(), it.data() );
		
	while( _subGroupName && _config->hasGroup( _subGroupName->arg( src ).arg( subnumber ) ) )
	{
		_config->deleteGroup( _subGroupName->arg( dest ).arg( subnumber ) );
		_config->setGroup( _subGroupName->arg( dest ).arg( subnumber ) );
		for( it = srcList->begin(); it != srcList->end(); ++it )
			_config->writeEntry( it.key(), it.data() );
			
		++subnumber;
	}
		
	delete srcList;
}

void KEditListBoxManager::changeItem( int first, int last )
{
	QMap<QString, QString> *firstList = new QMap<QString, QString >;
	QMap<QString, QString> *lastList = new QMap<QString, QString >;
	QMap<QString, QString>::iterator it;
	int subnumber = 0;

	*firstList = _config->entryMap( _groupName->arg( first ) );
	*lastList = _config->entryMap( _groupName->arg( last ) );
	_config->deleteGroup( _groupName->arg( first ) );
	_config->deleteGroup( _groupName->arg( last ) );
	
	_config->setGroup( _groupName->arg( last ) );
	for( it = firstList->begin(); it != firstList->end(); ++it )
		_config->writeEntry( it.key(), it.data() );
		
	_config->setGroup( _groupName->arg( first ) );
	for( it = lastList->begin(); it != lastList->end(); ++it )
		_config->writeEntry( it.key(), it.data() );
	
	while( _subGroupName && (
		_config->hasGroup( _subGroupName->arg( first ).arg( subnumber ) ) ||
		_config->hasGroup( _subGroupName->arg( last ).arg( subnumber ) ) ) )
	{
		*firstList = _config->entryMap( _subGroupName->arg( first ).arg( subnumber ) );
		*lastList = _config->entryMap( _subGroupName->arg( last ).arg( subnumber ) );
		_config->deleteGroup( _subGroupName->arg( first ).arg( subnumber ) );
		_config->deleteGroup( _subGroupName->arg( last ).arg( subnumber ) );
		
		_config->setGroup( _subGroupName->arg( last ).arg( subnumber ) );
		for( it = firstList->begin(); it != firstList->end(); ++it )
			_config->writeEntry( it.key(), it.data() );
		
		_config->setGroup( _subGroupName->arg( first ).arg( subnumber ) );
		for( it = lastList->begin(); it != lastList->end(); ++it )
			_config->writeEntry( it.key(), it.data() );
	
		++subnumber;
	}
		
	delete firstList;
	delete lastList;
}

void KEditListBoxManager::changedText()
{
	_config->setGroup( _groupName->arg( this->currentItem() ) );
	_config->writeEntry( "name", this->currentText() );
}

#include "keditlistboxman.moc"
