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

#include "accountmanager.h"

#include "dcopdrop.h"
#include "kio.h"
#include "maildrop.h"
#include "password.h"
#include "protocol.h"
#include "protocols.h"
#include "subjectsdlg.h"

#include <kaudioplayer.h>
#include <kconfig.h>
#include <kdebug.h>

#include <tqptrlist.h>

KornSubjectsDlg* AccountManager::_subjectsDlg = 0;

AccountManager::AccountManager( TQObject * parent, const char * name )
	: TQObject( parent, name ),
	_kioList( new TQPtrList< KMailDrop > ),
	_dcopList( new TQPtrList< DCOPDrop > ),
	_dropInfo( new TQMap< KMailDrop*, Dropinfo* > )
{
	_kioList->setAutoDelete( true );
	_dcopList->setAutoDelete( true );
}

AccountManager::~AccountManager()
{
	delete _kioList;
	delete _dcopList;
	delete _dropInfo;
}
	
void AccountManager::readConfig( KConfig* config, const int index )
{
	KConfigGroup *masterGroup = new KConfigGroup( config, TQString( "korn-%1" ).arg( index ) );
	TQStringList dcop = masterGroup->readListEntry( "dcop", ',' );
	KConfigGroup *accountGroup;
	int counter = 0;
		
	while( config->hasGroup( TQString( "korn-%1-%2" ).arg( index ).arg( counter ) ) )
	{
		accountGroup = new KConfigGroup( config, TQString( "korn-%1-%2" ).arg( index ).arg( counter ) );
		
		const Protocol *proto = Protocols::getProto( accountGroup->readEntry( "protocol" ) );
		if( !proto )
		{
			kdWarning() << "Protocol werd niet gevonden" << endl;
			++counter;
			continue;
		}
		TQMap< TQString, TQString > *configmap = proto->createConfig( accountGroup,
		                                                   KOrnPassword::readKOrnPassword( index, counter, *accountGroup ) );
		KMailDrop *kiodrop = proto->createMaildrop( accountGroup );
		const Protocol *nproto = proto->getProtocol( accountGroup );
		Dropinfo *info = new Dropinfo;

		if( !kiodrop || !configmap || !nproto )
		{
			//Error occured when reading for config
			++counter;
			delete info;
			continue;
		}
		
		//TODO: connect some stuff
		connect( kiodrop, TQT_SIGNAL( changed( int, KMailDrop* ) ), this, TQT_SLOT( slotChanged( int, KMailDrop* ) ) );
		connect( kiodrop, TQT_SIGNAL( showPassivePopup( TQPtrList< KornMailSubject >*, int, bool, const TQString& ) ),
			 this, TQT_SLOT( slotShowPassivePopup( TQPtrList< KornMailSubject >*, int, bool, const TQString& ) ) );
		connect( kiodrop, TQT_SIGNAL( showPassivePopup( const TQString&, const TQString& ) ),
			 this, TQT_SLOT( slotShowPassivePopup( const TQString&, const TQString& ) ) );
		connect( kiodrop, TQT_SIGNAL( validChanged( bool ) ), this, TQT_SLOT( slotValidChanged( bool ) ) );
		
		kiodrop->readGeneralConfigGroup( *masterGroup );
		if( !kiodrop->readConfigGroup( *accountGroup ) || !kiodrop->readConfigGroup( *configmap, nproto ) )
		{
			++counter;
			delete info;
			continue;
		}
		
		kiodrop->startMonitor();
		
		_kioList->append( kiodrop );
		
		info->index = counter;
		info->reset = accountGroup->readNumEntry( "reset", 0 );
		info->msgnr = info->reset;
		info->newMessages = false;
		
		_dropInfo->insert( kiodrop, info );
		
		++counter;
	}
	
	TQStringList::Iterator it;
	for( it = dcop.begin(); it != dcop.end(); ++it )
	{
		DCOPDrop *dcopdrop = new DCOPDrop;
		Dropinfo *info = new Dropinfo;
		
		connect( dcopdrop, TQT_SIGNAL( changed( int, KMailDrop* ) ), this, TQT_SLOT( slotChanged( int, KMailDrop* ) ) );
		connect( dcopdrop, TQT_SIGNAL( showPassivePopup( TQPtrList< KornMailSubject >*, int, bool, const TQString& ) ),
			 this, TQT_SLOT( slotShowPassivePopup( TQPtrList< KornMailSubject >*, int, bool, const TQString& ) ) );
		
		dcopdrop->readConfigGroup( *masterGroup );
		dcopdrop->setDCOPName( *it );
		
		_dcopList->append( dcopdrop );
		
		info->index = 0;
		info->reset = 0;
		info->msgnr = 0;
		info->newMessages = false;
		
		_dropInfo->insert( dcopdrop, info );
	}
	
	setCount( totalMessages(), hasNewMessages() );
}

void AccountManager::writeConfig( KConfig* config, const int index )
{
	TQMap< KMailDrop*, Dropinfo* >::Iterator it;
	for( it = _dropInfo->begin(); it != _dropInfo->end(); ++it )
	{
		config->setGroup( TQString( "korn-%1-%2" ).arg( index ).arg( it.data()->index ) );
		config->writeEntry( "reset", it.data()->reset );
	}
}

TQString AccountManager::getTooltip() const
{
	TQStringList result;
	TQMap< KMailDrop*, Dropinfo* >::Iterator it;
	for( it = _dropInfo->begin(); it != _dropInfo->end(); ++it )
		if( it.key()->valid() )
			result.append( TQString( "%1: %2" ).arg( it.key()->realName() ).arg( it.data()->msgnr - it.data()->reset ));
		else
			result.append( TQString( "%1: invalid" ).arg( it.key()->realName() ) );
	result.sort();
	return result.join( TQChar( '\n' ) );
}
	
void AccountManager::doRecheck()
{
	KMailDrop *item;
	for( item = _kioList->first(); item; item = _kioList->next() )
		item->forceRecheck();
}

void AccountManager::doReset()
{
	TQMap< KMailDrop*, Dropinfo* >::Iterator it;
	for( it = _dropInfo->begin(); it != _dropInfo->end(); ++it )
	{
		it.data()->reset = it.data()->msgnr;
		it.data()->newMessages = false;
	}
	
	setCount( 0, false );
}

void AccountManager::doView()
{
	TQMap< KMailDrop*, Dropinfo* >::Iterator it;
	
	if( !_subjectsDlg )
		_subjectsDlg = new KornSubjectsDlg();
	
	_subjectsDlg->clear();
	
	for( it = _dropInfo->begin(); it != _dropInfo->end(); ++it )
		_subjectsDlg->addMailBox( it.key() );
	
	_subjectsDlg->loadMessages();
}

void AccountManager::doStartTimer()
{
	KMailDrop *item;
	
	for( item = _kioList->first(); item; item = _kioList->next() )
		item->startMonitor();
}

void AccountManager::doStopTimer()
{
	KMailDrop *item;

	for( item = _kioList->first(); item; item = _kioList->next() )
		item->stopMonitor();
}

int AccountManager::totalMessages()
{
	int result = 0;
	
	TQMap< KMailDrop*, Dropinfo* >::Iterator it;
	for( it = _dropInfo->begin(); it != _dropInfo->end(); ++it )
		//if( it.date()->msgnr - it.date()->reset > 0 )
			result += it.data()->msgnr - it.data()->reset;
	
	return result;
}

bool AccountManager::hasNewMessages()
{
	TQMap< KMailDrop*, Dropinfo* >::Iterator it;
	for( it = _dropInfo->begin(); it != _dropInfo->end(); ++it )
		if( it.data()->newMessages )
			return true;
	
	return false;
}

void AccountManager::playSound( const TQString& file )
{
	KAudioPlayer::play( file );
}

void AccountManager::slotChanged( int count, KMailDrop* mailDrop )
{
	Dropinfo *info = _dropInfo->find( mailDrop ).data();
	info->newMessages = count > info->msgnr || ( count == info->msgnr && info->newMessages );
	
	if( count > info->msgnr )
	{
		if( !mailDrop->soundFile().isEmpty() )
			playSound( mailDrop->soundFile() );
		if( !mailDrop->newMailCmd().isEmpty() )
			runCommand( mailDrop->newMailCmd() );
	}
	
	info->msgnr = count;	
	if( info->msgnr - info->reset < 0 )
		info->reset = 0;

	setCount( totalMessages(), hasNewMessages() && totalMessages() > 0 );
	setTooltip( getTooltip() );
}

void AccountManager::slotValidChanged( bool )
{
	setTooltip( getTooltip() );
}

#include "accountmanager.moc"
