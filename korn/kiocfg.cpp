/*
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
* imapcfg.cpp -- Implementation of class KImapCfg.
* Author:	Kurt Granroth (granroth@kde.org)
* Version:	$Id$
* kiocfg.cpp  -- Change of imapcfg.cpp for kio by Mart Kelder
*/

#include <assert.h>
#include <stdlib.h> 

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qptrlist.h>

#include <klocale.h>

//#include "typolayout.h"

#include "kio.h"
#include "kiocfg.h"
#include "kio_proto.h"

KKioCfg::KKioCfg( KKioDrop *drop )
  : KMonitorCfg( drop ),
    _protoCombo( 0 ),
    _serverEdit( 0 ),
    _portEdit( 0 ),
    _mailboxEdit( 0 ),
    _userEdit( 0 ),
    _pwdEdit( 0 )
{
	_protocols = new QPtrList<KIO_Protocol>;
	_protocols->setAutoDelete( true );
}

KKioCfg::~KKioCfg( )
{
	delete _protocols;
}

QString KKioCfg::name() const
{
	return i18n("&KIO"); //Original name, isn't it?
}

QWidget *KKioCfg::makeWidget( QWidget *parent )
{
	KKioDrop *d = dynamic_cast<KKioDrop *>(drop());
	assert( d != 0 );

	QWidget *dlg = new QWidget( parent );
	QBoxLayout *layout = new QVBoxLayout( dlg, 10 );
	layout->addSpacing(10);

	// set up a box for the server settings

	QGroupBox *aGroup = new QGroupBox( i18n("Server"), dlg);
	layout->addWidget(aGroup);

	QGridLayout *slay = new QGridLayout( aGroup, 4, 3, 10 );
	slay->addRowSpacing(0, 10);

	QLabel *aLabel = new QLabel( i18n("Protocol:"), aGroup );
	slay->addWidget(aLabel, 1, 0);

	_protoCombo = new QComboBox ( aGroup );
	for( _this_protocol = _protocols->first(); _this_protocol; _this_protocol = _protocols->next() )
		_protoCombo->insertItem( _this_protocol->configName() );
	for( _this_protocol = _protocols->first(); _this_protocol && _this_protocol->configName() != d->protocol(); _this_protocol = _protocols->next() )
		if( ! _this_protocol ) 
			_this_protocol = _protocols->first();
	setComboItem( _this_protocol->configName() );

	connect( _protoCombo, SIGNAL(activated(int)), this, SLOT(protoChange(int)));
	slay->addWidget(_protoCombo, 1, 1);

        _serverLabel = new QLabel( _this_protocol->serverName(), aGroup );
        slay->addWidget(_serverLabel, 2, 0);

	_serverEdit = new QLineEdit( d->server(), aGroup );
	slay->addWidget(_serverEdit, 2, 1);
	if( ! _this_protocol->fields() & KIO_Protocol::server )
	{
		_serverLabel->hide();
		_serverEdit->hide();
	}
	
	QString sport;
	sport.setNum( d->port() );

	_portLabel = new QLabel( _this_protocol->portName(), aGroup );
	slay->addWidget(_portLabel, 3, 0);

	_portEdit = new QLineEdit( sport, aGroup );
	slay->addWidget(_portEdit, 3, 1);
	if( ! _this_protocol->fields() & KIO_Protocol::port )
	{
		_portLabel->hide();
		_portEdit->hide();
	}

	// set up a box for the user settings

	aGroup = new QGroupBox( i18n( "Identity" ), dlg );
	layout->addWidget(aGroup);

	slay = new QGridLayout(aGroup, 6, 2, 10);
	slay->addRowSpacing(0,10);

	_userLabel = new QLabel( _this_protocol->usernameName(), aGroup);
	slay->addWidget(_userLabel, 1, 0);

	_userEdit = new QLineEdit( d->user(), aGroup );
	slay->addWidget(_userEdit, 1, 1);
	if( ! _this_protocol->fields() & KIO_Protocol::username )
	{
		_userLabel->hide();
		_userEdit->hide();
	}

	_mailboxLabel = new QLabel( _this_protocol->mailboxName(), aGroup );
	slay->addWidget(_mailboxLabel, 2, 0);

	_mailboxEdit = new QLineEdit( d->mailbox(), aGroup );
	slay->addWidget(_mailboxEdit, 2, 1);
	if( ! _this_protocol->fields() & KIO_Protocol::mailbox )
	{
		_mailboxLabel->hide();
		_mailboxEdit->hide();
	}

	_pwdLabel = new QLabel( _this_protocol->passwordName(), aGroup );
	slay->addWidget(_pwdLabel, 3, 0);

	_pwdEdit = new QLineEdit( d->password(), aGroup );
	slay->addWidget(_pwdEdit, 3, 1);
	if( ! _this_protocol->fields() & KIO_Protocol::password )
	{
		_pwdLabel->hide();
		_pwdEdit->hide();
	}

	_pwdEdit->setEchoMode( QLineEdit::Password );

	_savePass = new QCheckBox( _this_protocol->savePasswordName(), aGroup );
	if( ! d->password().isEmpty() )
		_savePass->setChecked(true); //This line was missing in imap.cpp
	slay->addWidget(_savePass, 4, 1);

	_pwdEdit->setEnabled( _savePass->isChecked() );
	if( ! _this_protocol->fields() & KIO_Protocol::password )
	{
		_pwdLabel->hide();
		_pwdEdit->hide();
		_savePass->hide();
	}
	connect( _savePass, SIGNAL(toggled(bool)), _pwdEdit,
		SLOT(setEnabled(bool)) );

	_authLabel = new QLabel( _this_protocol->authName(), aGroup );
	slay->addWidget( _authLabel, 5, 0);

	_authCombo = new QComboBox( aGroup );
	_authCombo->insertStringList( _this_protocol->authList() );
	slay->addWidget( _authCombo, 5, 1);
	if( ! _this_protocol->fields() & KIO_Protocol::auth )
	{
		_authLabel->hide();
		_authCombo->hide();
	} else {
		_authCombo->setCurrentItem( 0 );
		for( int xx=0; xx < _authCombo->count(); xx++ )
			if( _authCombo->text( xx ) == d->auth() )
				_authCombo->setCurrentItem( xx );
	}

	return dlg;
}

void KKioCfg::updateConfig()
{
	assert( _protoCombo != 0 );
	assert( _serverEdit != 0 );
	assert( _portEdit != 0 );
	assert( _mailboxEdit != 0 );
	assert( _userEdit != 0 );
	assert( _pwdEdit != 0 );
	assert( _savePass != 0 );

	KKioDrop *d = dynamic_cast<KKioDrop *>(drop());
	assert( d != 0 );

	d->setKioServer( _protoCombo->currentText(),
	                 _this_protocol->fields() & KIO_Protocol::server ? _serverEdit->text() : "",
			 _this_protocol->fields() & KIO_Protocol::port ? _portEdit->text().toInt() : 0 );
	d->setUser( _this_protocol->fields() & KIO_Protocol::username ? _userEdit->text() : "",
	            _this_protocol->fields() & KIO_Protocol::password && _savePass->isChecked() ? _pwdEdit->text() : "", 
		    _this_protocol->fields() & KIO_Protocol::mailbox ? _mailboxEdit->text() : "",
		    _this_protocol->fields() & KIO_Protocol::auth ? _authCombo->currentText() : "" );
}

void KKioCfg::addProtocol( KIO_Protocol * protocol )
{
	_protocols->append( protocol );
}

bool KKioCfg::setComboItem( const QString & item )
{
	for ( int xx = 0; xx < _protoCombo->count(); xx++)
	{
		if ( _protoCombo->text( xx ) == item )
		{
			_protoCombo->setCurrentItem( xx );
			return true;
		}
	}
	return false;
}
//private slots
void KKioCfg::protoChange(int index)
{
	_this_protocol = _protocols->at( index );
	if( _this_protocol->fields() & KIO_Protocol::server )
	{
		_serverLabel->show();
		_serverEdit->show();
	} else {
		_serverLabel->hide();
		_serverEdit->hide();
	}
	if( _this_protocol->fields() & KIO_Protocol::port )
	{
		_portLabel->show();
		_portEdit->show();
		_portEdit->setText( QString::number( _this_protocol->defaultPort() ) );
	} else {
		_portLabel->hide();
		_portEdit->hide();
	}
	if( _this_protocol->fields() & KIO_Protocol::username )
	{
		_userLabel->show();
		_userEdit->show();
	} else {
		_userLabel->hide();
		_userEdit->hide();
	}
	if( _this_protocol->fields() & KIO_Protocol::mailbox )
	{
		_mailboxLabel->show();
		_mailboxEdit->show();
	} else {
		_mailboxLabel->hide();
		_mailboxEdit->hide();
	}
	if( _this_protocol->fields() & KIO_Protocol::password )
	{
		_savePass->show();
		_pwdLabel->show();
		_pwdEdit->show();
		_pwdEdit->setEnabled( _savePass->isChecked() );
	} else {
		_savePass->hide();
		_pwdLabel->hide();
		_pwdEdit->hide();
	}
	if( _this_protocol->fields() & KIO_Protocol::auth )
	{
		_authLabel->show();
		_authCombo->show();
		_authCombo->clear();
		_authCombo->insertStringList( _this_protocol->authList() );
	} else {
		_authLabel->hide();
		_authCombo->hide();
	}

	_serverLabel->setText( _this_protocol->serverName() );
	_portLabel->setText( _this_protocol->portName() );
	_userLabel->setText( _this_protocol->usernameName() );
	_mailboxLabel->setText( _this_protocol->mailboxName() );
	_pwdLabel->setText( _this_protocol->passwordName() );
	_savePass->setText( _this_protocol->savePasswordName() );
}


#include "kiocfg.moc"
