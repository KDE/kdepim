/*
* qmail.cpp -- Implementation of class KQMailDrop.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Mon Dec  1 00:45:12 EST 1997
*/

#include<qdir.h>

#include<kconfigbase.h>

#include"utils.h"
#include"dropdlg.h"

#include"qmail.h"
#include"qmailcfg.h"

KQMailDrop::KQMailDrop()
	: KPollableDrop()
{
	_lastSize = 0;

	_valid = false;
}

KQMailDrop::~KQMailDrop()
{
	stopMonitor();
}

void KQMailDrop::recheck()
{
	int messages = count();

	if( valid() && touched() ) {
		if( !_info.exists() ){
			messages = 0;
		}
		else {
			QDir dir( _info.filePath() );
			messages = (dir.count()-2);
		}
	}

	if( messages != count() ) {
		emit changed(messages);
	}
}

bool KQMailDrop::valid()
{
	return _valid;
}

bool KQMailDrop::touched()
{
	_info.refresh();

	if ( !_info.exists() ) {
		if( _lastSize == 0  ) return false;
		_lastSize = 0;
		return true;
	}

	if(_info.size() != _lastSize
			|| (_info.lastModified() > _lastMod) ) {

		_lastSize = _info.size();
		_lastMod = _info.lastModified();

		return true;
	}

	return false;
}

void KQMailDrop::setMaildir(const QString & dir)
{
  bool run = running();

  if( run ) {
    stopMonitor();
  }

  _maildir = dir;

  QString real = dir;
  real += fu("/new");

  _valid = true;

  // update monitors
  _info.setFile( real );
  _lastMod.setTime_t( 0 );

  if( run ) {
    startMonitor();
  }  	
}

KQMailDrop& KQMailDrop::operator=( const KQMailDrop& other )
{
	setMaildir( other.maildir() );
	setFreq( other.freq() );

	return *this;
}

QWidget* KQMailDrop::typeSpecificConfig()
{
	return 0;
}

KMailDrop* KQMailDrop::clone() const
{
	KQMailDrop *clone = new KQMailDrop;

	*clone = *this;

	return clone;
}

bool KQMailDrop::readConfigGroup( const KConfigBase& cfg )
{
	KPollableDrop::readConfigGroup( cfg );

	QString dir = cfg.readEntry(fu(MaildirConfigKey));

	if( dir.isEmpty() ) {
		qWarning( "KQMailDrop::readConfigGroup: no dir for '%s'.",
		      caption().ascii() );

		_valid = false;

		return false;
	}

	setMaildir( dir );

	return true;
}

bool KQMailDrop::writeConfigGroup( KConfigBase& cfg ) const
{
	KPollableDrop::writeConfigGroup( cfg );
	cfg.writeEntry(fu(MaildirConfigKey), maildir() );

	return true;
}

void KQMailDrop::addConfigPage( KDropCfgDialog *dlg )
{
	dlg->addConfigPage( new KQMailCfg ( this ) );
	KPollableDrop::addConfigPage( dlg );
}

const char *KQMailDrop::MaildirConfigKey = "maildir";
