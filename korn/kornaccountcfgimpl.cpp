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

#include "kornaccountcfgimpl.h"

#include "kio_proto.h"
#include "protocols.h"

#include <kconfigbase.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qwidget.h>

KornAccountCfgImpl::KornAccountCfgImpl( QWidget * parent, const char * name )
	: KornAccountCfg( parent, name ),
	_config( 0 ),
	_fields( 0 ),
	_urlfields( 0 )
{
	connect( parent, SIGNAL( okClicked() ), this, SLOT( slotOK() ) );
	connect( parent, SIGNAL( cancelClicked() ), this, SLOT( slotCancel() ) );
	
	this->cbProtocol->insertStringList( Protocols::getProtocols() );
}
	
KornAccountCfgImpl::~KornAccountCfgImpl()
{
}

void KornAccountCfgImpl::readConfig( KConfigGroup *config )
{
	_config = config;
	
	this->cbProtocol->setCurrentText( _config->readEntry( "protocol", "mbox" ) );
	slotProtocolChanged( this->cbProtocol->currentText() );
	
	if( _fields & KIO_Protocol::server )
		this->edServer->setText( _config->readEntry( "server", "localhost" ) );
	if( _urlfields & KIO_Protocol::server )
		this->urlServer->setURL( _config->readEntry( "server", "localhost" ) );
	if( _fields & KIO_Protocol::port )
		this->edPort->setText( _config->readEntry( "port", this->edPort->text() ) );
	if( _urlfields & KIO_Protocol::port )
		this->urlPort->setURL( _config->readEntry( "port", this->urlPort->url() ) );
	
	if( _fields & KIO_Protocol::username )
		this->edUsername->setText( _config->readEntry( "username", "" ) );
	if( _urlfields & KIO_Protocol::username )
		this->urlUsername->setURL( _config->readEntry( "username", "" ) );
	if( _fields & KIO_Protocol::mailbox )
		this->edMailbox->setText( _config->readEntry( "mailbox", "" ) );
	if( _urlfields & KIO_Protocol::mailbox )
		this->urlMailbox->setURL( _config->readEntry( "mailbox", "" ) );
	if( _fields & KIO_Protocol::password )
		this->chPassword->setChecked( _config->readBoolEntry( "savepassword", false ) );
	if( _fields & KIO_Protocol::password )
	{
		this->edPassword->setEnabled( _config->readBoolEntry( "savepassword", false ) );
		this->edPassword->setText( _config->readEntry( "password", "" ) );
	}
	if( _fields & KIO_Protocol::auth )
	{
		this->cbAuth->setCurrentItem( -1 );
		for( int index = 0; index < this->cbAuth->count(); ++index )
			if( this->cbAuth->text( index ) == _config->readEntry( "auth", "Plain" ) )
				this->cbAuth->setCurrentItem( index );
	}
	
	this->edInterval->setText( _config->readEntry( "interval", "300" ) );
	
	this->chUseBox->setChecked( _config->readBoolEntry( "boxsettings", true ) );
	this->edRunCommand->setURL( _config->readEntry( "command", "" ) );
	this->edPlaySound->setURL( _config->readEntry( "sound", "" ) );
	this->chPassivePopup->setChecked( _config->readBoolEntry( "passivepopup", false ) );
	this->chPassiveDate->setChecked( _config->readBoolEntry( "passivedate", false ) );
}

void KornAccountCfgImpl::writeConfig()
{
	_config->writeEntry( "protocol", this->cbProtocol->currentText() );
		
	if( _fields & KIO_Protocol::server )
		_config->writeEntry( "server", this->edServer->text() );
	else if( _urlfields & KIO_Protocol::server )
		_config->writeEntry( "server", this->urlServer->url() );
	else
		_config->writeEntry( "server", "" );
	if( _fields & KIO_Protocol::port )
		_config->writeEntry( "port", this->edPort->text() );
	else if( _urlfields & KIO_Protocol::port )
		_config->writeEntry( "port", this->urlPort->url() );
	else
		_config->writeEntry( "port", "" );
	
	if( _fields & KIO_Protocol::username )
		_config->writeEntry( "username", this->edUsername->text() );
	else if( _urlfields & KIO_Protocol::username )
		_config->writeEntry( "username", this->urlUsername->url() );
	else
		_config->writeEntry( "username", "" );
	if( _fields & KIO_Protocol::mailbox )
		_config->writeEntry( "mailbox", this->edMailbox->text() );
	else if( _urlfields & KIO_Protocol::mailbox )
		_config->writeEntry( "mailbox", this->urlMailbox->url() );
	else
		_config->writeEntry( "mailbox", "" );
	if( _fields & KIO_Protocol::password )
		_config->writeEntry( "savepassword", this->chPassword->isChecked() );
	
	if( ( _fields & KIO_Protocol::password ) && this->edPassword->isEnabled() )
		_config->writeEntry( "password", this->edPassword->text() );
	else
		_config->writeEntry( "password", "" );
	if( _fields & KIO_Protocol::auth )
		_config->writeEntry( "auth", this->cbAuth->currentText() );
	else
		_config->writeEntry( "auth", "" );
	
	_config->writeEntry( "interval", this->edInterval->text().toInt() );

	_config->writeEntry( "boxsettings", this->chUseBox->isChecked() );
	_config->writeEntry( "command", this->edRunCommand->url() );
	_config->writeEntry( "sound", this->edPlaySound->url() );
	_config->writeEntry( "passivepopup", this->chPassivePopup->isChecked() );
	_config->writeEntry( "passivedate", this->chPassiveDate->isChecked() );
}
	
void KornAccountCfgImpl::slotOK()
{
	writeConfig();
}

void KornAccountCfgImpl::slotCancel()
{
}

void KornAccountCfgImpl::slotProtocolChanged( const QString& proto )
{
	KIO_Protocol *protocol = Protocols::getProto( proto );
	if( protocol == 0 )
		return; //ERROR: protocol not found
	
	_fields = protocol->fields();
	_urlfields = protocol->urlFields();
	
	//Hide show the different boxes
	showHide( KIO_Protocol::server, lbServer, (QWidget*)edServer, urlServer, protocol->serverName() );
	showHide( KIO_Protocol::port, lbPort, (QWidget*)edPort, urlPort, protocol->portName() );
	showHide( KIO_Protocol::username, lbUsername, (QWidget*)edUsername, urlUsername, protocol->usernameName() );
	showHide( KIO_Protocol::mailbox, lbMailbox, (QWidget*)edMailbox, urlMailbox, protocol->mailboxName() );
	showHide( KIO_Protocol::password, 0, chPassword, 0, QString::null );
	showHide( KIO_Protocol::password, lbPassword, (QWidget*)edPassword, 0, protocol->passwordName() );
	showHide( KIO_Protocol::auth, lbAuth, (QWidget*)cbAuth, 0, protocol->authName() );
	chPassword->setText( protocol->savePasswordName() );
	
	//Filling authlist
	if( _fields & KIO_Protocol::auth )
	{
		cbAuth->clear();
		cbAuth->insertStringList( protocol->authList() );
	}
	
	edPort->setText( QString::number( protocol->defaultPort() ) );
	urlPort->setURL( QString::number( protocol->defaultPort() ) );
}

void KornAccountCfgImpl::showHide( int fieldvalue, QLabel *label, QWidget* edit, KURLRequester* url,
                                   const QString& labelText )
{
	if( _fields & fieldvalue )
	{
		if( label )
		{
			label->show();
			label->setText( labelText );
		}
		if( edit )
			edit->show();
		if( url )
			url->hide();
	} else if( _urlfields & fieldvalue )
	{
		if( label )
		{
			label->show();
			label->setText( labelText );
		}
		if( edit )
			edit->hide();
		if( url )
			url->show();
	} else {
		if( label )
		{
			label->hide();
			label->setText( "" );
		}
		if( edit )
			edit->hide();
		if( url )
			url->hide();
	}
}

#include "kornaccountcfgimpl.moc"
