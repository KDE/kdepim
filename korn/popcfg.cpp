/*
* popcfg.cpp -- Implementation of class KPopCfg.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Mon Aug  3 14:58:11 EST 1998
*/

#include <assert.h>
#include <stdlib.h>

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>

//#include"typolayout.h"

#include "utils.h"
#include "pop.h"
#include "popcfg.h"

KPopCfg::KPopCfg( KPop3Drop *drop )
	: KMonitorCfg( drop ),
	_serverEdit( 0 ),
	_portEdit( 0 ),
	_userEdit( 0 ),
	_pwdEdit( 0 )
{
}

QString KPopCfg::name() const
{
	return i18n("P&OP 3");
}

QWidget *KPopCfg::makeWidget( QWidget *parent )
{
	KPop3Drop *d = dynamic_cast<KPop3Drop *>(drop());
	assert( d != 0 );

	QWidget *dlg = new QWidget( parent );
	QBoxLayout *layout = new QVBoxLayout( dlg, 10 );
	layout->addSpacing(10);

	QGroupBox *aGroup = new QGroupBox( i18n("Server"), dlg );
	layout->addWidget( aGroup );
	
	QGridLayout *slay = new QGridLayout( aGroup, 3, 2, 10 );
	slay->addRowSpacing(0, 10);

	slay->addWidget( new QLabel( i18n("Server:"), aGroup ), 1, 0 );

	_serverEdit = new QLineEdit( d->server(), aGroup );
	slay->addWidget( _serverEdit, 1, 1 );
	
	QString sport;
	sport.setNum( d->port() );
	slay->addWidget(new QLabel( i18n("Port:"), aGroup ), 2, 0);

	_portEdit = new QLineEdit( sport, aGroup );
	slay->addWidget( _portEdit, 2, 1);

	aGroup = new QGroupBox( i18n( "Identity" ), dlg );
	layout->addWidget( aGroup );

	slay = new QGridLayout ( aGroup, 4, 2, 10);
	slay->addRowSpacing (0, 10);

	slay->addWidget( new QLabel( i18n("Username:"), aGroup ), 1, 0);

	_userEdit = new QLineEdit( d->user(), aGroup );
	slay->addWidget( _userEdit, 1, 1);
	
	slay->addWidget( new QLabel( i18n("Password:"), aGroup ), 2, 0);

	_pwdEdit = new QLineEdit( d->password(), aGroup );
	slay->addWidget( _pwdEdit, 2, 1);
	_pwdEdit->setEchoMode( QLineEdit::Password );

	_savePass = new QCheckBox( i18n( "Save password"), aGroup );
	slay->addWidget( _savePass, 3, 1);
	_savePass->setChecked( d->password().isEmpty() ? false : true );

	_apopAuth = new QCheckBox( i18n( "Use APOP authentication"), aGroup );
	slay->addWidget( _apopAuth, 4, 1);
	_apopAuth->setChecked( d->apopAuth() );

	_pwdEdit->setEnabled(_savePass->isChecked());
	connect( _savePass, SIGNAL(toggled(bool)), _pwdEdit,
		SLOT(setEnabled(bool)) );

	return dlg;
}

void KPopCfg::updateConfig()
{
	assert( _serverEdit != 0 );
	assert( _portEdit != 0 );
	assert( _userEdit != 0 );
	assert( _pwdEdit != 0 );
	assert( _savePass != 0 );
	assert( _apopAuth != 0 );

	KPop3Drop *d = dynamic_cast<KPop3Drop *>(drop());
	assert( d != 0 );

	d->setPopServer( _serverEdit->text(), _portEdit->text().toInt(), _apopAuth->isChecked());
	d->setUser( _userEdit->text(), _pwdEdit->text(), 
		_savePass->isChecked() );
}
