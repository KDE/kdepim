/*
 * imap.cpp -- Implementation of class KImap4Drop.
 * Author:	Kurt Granroth
 * Version:	$Id$
 */

#include<stdio.h>

#include<kconfigbase.h>

#include"utils.h"
#include"kbiffimap.h"
#include"imap.h"
#include"imapcfg.h"
#include"dropdlg.h"

KImap4Drop::KImap4Drop()
	: KPollableDrop(),
	_port( DefaultPort ),
	_savePassword(false),
	_valid(false),
	_imap(0)
{
}

void KImap4Drop::setImapServer(const QString & server, int port)
{
	_server = server;
	_port	= port;
}

void KImap4Drop::setUser(const QString & user, const QString & password,
	const QString & mailbox, bool savepass )
{
	_user = user;
	_password = password;
	_mailbox = mailbox;
	_savePassword = savepass;

	_valid = true;
}

void KImap4Drop::recheck()
{
	QString command;
	int seq = 1000;

	if( _imap == 0 ) {
		_imap = new KBiffImap;
	}

	if ( _imap->connect( _server, _port ) == false ) {
		_valid = false;
		return;
	}
	
	// some systems allow spaces in usernames -- must be quoted, though
	_user = _imap->mungeUser( _user );

	// if user is null, assume PREAUTH
	if ( ! _user.isEmpty() )
	{
		command = QString::number(seq) + fu(" LOGIN ") + _user + fu(" ") + _password + fu("\r\n");
		if ( _imap->command( command, seq ) == false ) {
			_valid = false;
			_imap->close();
			return;
		}
		seq++;
	}

	command = QString::number(seq) + fu(" STATUS ") + _mailbox + fu(" (unseen)\r\n");

	if ( _imap->command( command, seq ) == false )
		return;
	seq++;

	command = QString::number(seq) + fu(" LOGOUT\r\n");
	_imap->command( command, seq );
	_imap->close();

	int newcount = _imap->messages();

	if( newcount != count() ) {
		emit changed( newcount );
	}

	_valid = true;
	return;
}

bool KImap4Drop::valid()
{
	return _valid;
}

KImap4Drop::~KImap4Drop()
{
	delete _imap;
}

KMailDrop* KImap4Drop::clone() const 
{
	KImap4Drop *clone = new KImap4Drop;

	*clone = *this;

	return clone;
}

bool KImap4Drop::readConfigGroup( const KConfigBase& cfg )
{
	QString val;
	KPollableDrop::readConfigGroup( cfg );

	val = cfg.readEntry(fu(HostConfigKey));
	if( val.isEmpty() ) { _valid = false; return false; }

	setImapServer( val, cfg.readNumEntry(fu(PortConfigKey), DefaultPort ) );

	_user = cfg.readEntry(fu(UserConfigKey));
	if( _user.isEmpty() ) { _valid = false; return false; }

	_mailbox = cfg.readEntry(fu(MailboxConfigKey));
	if( _mailbox.isEmpty() ) { _valid = false; return false; }

	_password = cfg.readEntry(fu(PassConfigKey));

	if( _password.isEmpty() ) {
		_savePassword = false;
	}
	else {
		_savePassword = true;
		decrypt( _password );
	}

	return true;
}

bool KImap4Drop::writeConfigGroup( KConfigBase& cfg ) const
{
	KPollableDrop::writeConfigGroup( cfg );
	QString p;

	if( _savePassword == true ) {
		p = _password;
		encrypt( p );
	}

	cfg.writeEntry(fu(HostConfigKey), _server );
	cfg.writeEntry(fu(PortConfigKey), _port );
	cfg.writeEntry(fu(UserConfigKey), _user );
	cfg.writeEntry(fu(MailboxConfigKey), _mailbox );
	cfg.writeEntry(fu(PassConfigKey), p );

	return true;
}

KImap4Drop& KImap4Drop::operator = ( const KImap4Drop& other )
{
	setImapServer( other._server, other._port );
	setUser( other._user,other._password,other._mailbox );
	setFreq( other.freq() );

	return *this;
}

void KImap4Drop::addConfigPage( KDropCfgDialog *dlg )
{
	dlg->addConfigPage( new KImapCfg( this ) );

	KPollableDrop::addConfigPage( dlg );
}

void KImap4Drop::encrypt( QString& str )
{
	unsigned int i, val;
	unsigned int len = str.length();
	QString result;

	for ( i=0; i < len; i++ )
	{
		val = str[i].latin1() - ' ';
		val = (255-' ') - val;
		result[i] = (char)(val + ' ');
	}
	result[i] = '\0';
}

void KImap4Drop::decrypt( QString& str )
{
	encrypt( str );
}

const char *KImap4Drop::HostConfigKey = "host";
const char *KImap4Drop::PortConfigKey = "port";
const char *KImap4Drop::UserConfigKey = "user";
const char *KImap4Drop::PassConfigKey = "pass";
const char *KImap4Drop::MailboxConfigKey = "mailbox";
const char *KImap4Drop::SavePassConfigKey = "savepass";
const int  KImap4Drop::DefaultPort	= 143;
