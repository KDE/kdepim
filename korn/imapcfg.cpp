/*
* imapcfg.cpp -- Implementation of class KImapCfg.
* Author:	Kurt Granroth (granroth@kde.org)
* Version:	$Id$
*/

#include <assert.h>
#include <stdlib.h> 

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <klocale.h>

//#include "typolayout.h"

#include "imap.h"
#include "imapcfg.h"

KImapCfg::KImapCfg( KImap4Drop *drop )
  : KMonitorCfg( drop ),
    _serverEdit( 0 ),
    _portEdit( 0 ),
    _mailboxEdit( 0 ),
    _userEdit( 0 ),
    _pwdEdit( 0 )
{
}

QString KImapCfg::name() const
{
	return i18n("&IMAP 4");
}

QWidget *KImapCfg::makeWidget( QWidget *parent )
{
	KImap4Drop *d = dynamic_cast<KImap4Drop *>(drop());
	assert( d != 0 );

	QWidget *dlg = new QWidget( parent );
	QBoxLayout *layout = new QVBoxLayout( dlg, 10 );
	layout->addSpacing(10);


	// set up a box for the server settings

	QGroupBox *aGroup = new QGroupBox( i18n("Server"), dlg);
	layout->addWidget(aGroup);

	QGridLayout *slay = new QGridLayout( aGroup, 3, 3, 10 );
	slay->addRowSpacing(0, 10);

	QLabel *aLabel = new QLabel( i18n("Server:"), aGroup );
	slay->addWidget(aLabel, 1, 0);

	_serverEdit = new QLineEdit( d->server(), aGroup );
	slay->addWidget(_serverEdit, 1, 1);
	
	QString sport;
	sport.setNum( d->port() );

	aLabel = new QLabel( i18n("Port:"), aGroup );
	slay->addWidget(aLabel, 2, 0);

	_portEdit = new QLineEdit( sport, aGroup );
	slay->addWidget(_portEdit, 2, 1);


	// set up a box for the user settings

	aGroup = new QGroupBox( i18n( "Identity" ), dlg );
	layout->addWidget(aGroup);

	slay = new QGridLayout(aGroup, 5, 2, 10);
	slay->addRowSpacing(0,10);

	aLabel = new QLabel( i18n("Username:"), aGroup);
	slay->addWidget(aLabel, 1, 0);

	_userEdit = new QLineEdit( d->user(), aGroup );
	slay->addWidget(_userEdit, 1, 1);

	aLabel = new QLabel( i18n("Mailbox:"), aGroup );
	slay->addWidget(aLabel, 2, 0);

	_mailboxEdit = new QLineEdit( d->mailbox(), aGroup );
	slay->addWidget(_mailboxEdit, 2, 1);

	aLabel = new QLabel( i18n("Password:"), aGroup );
	slay->addWidget(aLabel, 3, 0);

	_pwdEdit = new QLineEdit( d->password(), aGroup );
	slay->addWidget(_pwdEdit, 3, 1);

	_pwdEdit->setEchoMode( QLineEdit::Password );

	_savePass = new QCheckBox( i18n( "Save password"), aGroup );
	slay->addWidget(_savePass, 4, 1);
	_savePass->setChecked( ! d->password().isEmpty() );

	_pwdEdit->setEnabled(_savePass->isChecked());
	connect( _savePass, SIGNAL(toggled(bool)), _pwdEdit,
		SLOT(setEnabled(bool)) );

	return dlg;
}

void KImapCfg::updateConfig()
{
	assert( _serverEdit != 0 );
	assert( _portEdit != 0 );
	assert( _mailboxEdit != 0 );
	assert( _userEdit != 0 );
	assert( _pwdEdit != 0 );
	assert( _savePass != 0 );

	KImap4Drop *d = dynamic_cast<KImap4Drop *>(drop());
	assert( d != 0 );

	d->setImapServer( _serverEdit->text(), _portEdit->text().toInt());
	d->setUser( _userEdit->text(), _pwdEdit->text(), 
			_mailboxEdit->text(), _savePass->isChecked() );
}
