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
 
#include "korncfgimpl.h"

#include "keditlistboxman.h"
#include "kornboxcfgimpl.h"
#include "password.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>

#include <qcolor.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qstring.h>
#include <qtimer.h>


/*
 * parent should be of type KDialogBase
 */
KornCfgImpl::KornCfgImpl( QWidget * parent, const char * name )
	: KornCfgWidget( parent, name ),	
	_config( new KConfig( "kornrc" ) ),
	_base( 0 )
{
	elbBoxes->setSubGroupName( "korn-%1-%2" );
	elbBoxes->setGroupName( "korn-%1" );
	elbBoxes->setConfig( _config );
	elbBoxes->setTitle( i18n( "Boxes" ) );
	
	connect( parent, SIGNAL( okClicked() ), this, SLOT( slotOK() ) );
	connect( parent, SIGNAL( cancelClicked() ), this, SLOT( slotCancel() ) );
	connect( parent, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );

	connect( elbBoxes, SIGNAL( elementsSwapped( int, int ) ), this, SLOT( slotElementsSwapped( int, int ) ) );
	connect( elbBoxes, SIGNAL( elementDeleted( int ) ), this, SLOT( slotElementDeleted( int ) ) );
	
	readConfig();
}

KornCfgImpl::~KornCfgImpl()
{
	_config->sync();
}

void KornCfgImpl::slotEditBox()
{
	if( _base )
		return; //Already a dialog open
	if( elbBoxes->listBox()->currentItem() < 0 )
		return; //No item selected
	elbBoxes->setEnabled( false );
	
	_base = new KDialogBase( this, "Box Dialog", false, "Box Configuration",
					     KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );
	KornBoxCfgImpl *widget = new KornBoxCfgImpl( _base, "Box Widget" );
	
	connect( _base, SIGNAL( finished() ), this, SLOT( slotDialogDestroyed() ) );
	
	_base->setMainWidget( widget );
	widget->readConfig( _config, elbBoxes->listBox()->currentItem() );
	
	_base->show();
}

void KornCfgImpl::slotDialogDestroyed()
{
	_base->deleteLater(); _base = 0;
	elbBoxes->setEnabled( true );
}

void KornCfgImpl::slotElementsSwapped( int box1, int box2 )
{
	int accountnumber1 = 0, accountnumber2 = 0;
	while( _config->hasGroup( QString( "korn-%1-%2" ).arg( box1 ).arg( accountnumber1 ) ) )
		++accountnumber1;
	
	while( _config->hasGroup( QString( "korn-%1-%2" ).arg( box2 ).arg( accountnumber2 ) ) )
		++accountnumber2;
	
	KOrnPassword::swapKOrnWalletPasswords( box1, accountnumber1, box2, accountnumber2 );
}

void KornCfgImpl::slotElementDeleted( int box )
{
	int accountnumber = 0;
	while( _config->hasGroup( QString( "korn-%1-%2" ).arg( box ).arg( accountnumber ) ) )
	{
		KOrnPassword::deleteKOrnPassword( box, accountnumber );
		++accountnumber;
	}
}

void KornCfgImpl::slotActivated( const QString& )
{
	slotEditBox();
}

void KornCfgImpl::slotActivated( const int )
{
	slotEditBox();
}

void KornCfgImpl::slotSetDefaults( const QString& name, const int index, KConfig* config )
{
	config->writeEntry( "name", name );
	config->writeEntry( "hasnormalfgcolour", true );
	config->writeEntry( "hasnewfgcolour", true );
	config->writeEntry( "hasnormalbgcolour", false );
	config->writeEntry( "hasnewbgcolour", false );
	config->writeEntry( "hasnormalicon", false );
	config->writeEntry( "hasnewicon", false );
	config->writeEntry( "hasnormalanim", false );
	config->writeEntry( "hasnewanim", false );
	config->writeEntry( "normalfgcolour", Qt::black );
	config->writeEntry( "newfgcolour", Qt::black );
	config->writeEntry( "normalbgcolour", QString::null );
	config->writeEntry( "newbgcolour", QString::null );
	config->writeEntry( "normalicon", QString::null );
	config->writeEntry( "newicon", QString::null );
	config->writeEntry( "normalanim", QString::null );
	config->writeEntry( "newanim", QString::null );
	config->writeEntry( "leftrecheck", true );
	config->writeEntry( "middlerecheck", false );
	config->writeEntry( "rightrecheck", false );
	config->writeEntry( "leftreset", false );
	config->writeEntry( "middlereset", false );
	config->writeEntry( "rightreset", false );
	config->writeEntry( "leftview", false );
	config->writeEntry( "middleview", false );
	config->writeEntry( "rightview", false );
	config->writeEntry( "leftcommand", false );
	config->writeEntry( "middlecommand", false );
	config->writeEntry( "rightcommand", false );
	config->writeEntry( "leftpopup", false );
	config->writeEntry( "middlepopupk", false );
	config->writeEntry( "rightpopup", true );
	config->writeEntry( "command", "" );
	config->writeEntry( "newcommand", "" );
	config->writeEntry( "sound", "" );
	config->writeEntry( "passivepopup", false );
	config->writeEntry( "passivedate", false );
	config->writeEntry( "numaccounts", 1 );
	config->writeEntry( "dcop", QStringList() );
	config->setGroup( QString( "korn-%1-0" ).arg( index ) );
	config->writeEntry( "name", name );
	config->writeEntry( "protocol", "mbox" );
	config->writeEntry( "server", QString::null );
	config->writeEntry( "port", QString::null );
	config->writeEntry( "username", QString::null );
	config->writeEntry( "mailbox", "/var/spool/mail/" );
	config->writeEntry( "savepassword", 0 );
	config->writeEntry( "password", QString::null );
	config->writeEntry( "auth", QString::null );
	config->writeEntry( "interval", 300 );
	config->writeEntry( "boxsettings", true );
	config->writeEntry( "command", "" );
	config->writeEntry( "sound", "" );
	config->writeEntry( "passivepopup", false );
	config->writeEntry( "passivedate", false );
}

void KornCfgImpl::slotOK()
{
	writeConfig();
}

void KornCfgImpl::slotCancel()
{
	_config->rollback();
}

void KornCfgImpl::slotApply()
{
	writeConfig();
}

void KornCfgImpl::readConfig()
{
	_config->setGroup( "korn" );
	
	QChar layout = _config->readEntry( "layout" ).stripWhiteSpace()[0].upper();
	if( layout == QChar( 'H' ) )
		rbHorizontal->setChecked( true );
	else if( layout == QChar( 'V' ) )
		rbVertical->setChecked( true );
	else
		rbDocked->setChecked( true );

	chUseWallet->setChecked( _config->readBoolEntry( "usewallet", true ) );
}

void KornCfgImpl::writeConfig()
{
	_config->setGroup( "korn" );
	
	if( rbHorizontal->isChecked() )
		_config->writeEntry( "layout", "Horizontal" );
	if( rbVertical->isChecked() )
		_config->writeEntry( "layout", "Vertical" );
	if( rbDocked->isChecked() )
		_config->writeEntry( "layout", "Docked" );

	//Default is 'false' here, because if no option is set, KWallet isn't used.
	if( _config->readBoolEntry( "usewallet", false ) != chUseWallet->isChecked() )
		//Configuration box changed => setting over configuration
		rewritePasswords();

	_config->writeEntry( "usewallet", chUseWallet->isChecked() );
	
	_config->sync();
}

void KornCfgImpl::rewritePasswords()
{
	int box = 0 - 1;
	int account = 0 - 1;
	KConfigGroup *group;

	while( _config->hasGroup( QString( "korn-%1" ).arg( ++box ) ) )
	{
		account = 0 - 1;
		while( _config->hasGroup( QString( "korn-%1-%2" ).arg( box ).arg( ++account ) ) )
		{
			group = new KConfigGroup( _config, QString( "korn-%1-%2" ).arg( box ).arg( account ) );
			KOrnPassword::rewritePassword( box, account, *group, chUseWallet->isChecked() );
			delete group;
		}
	}
	
	_config->setGroup( "korn" );
}

#include "korncfgimpl.moc"
