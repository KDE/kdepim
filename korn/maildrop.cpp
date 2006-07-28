/*
* maildrop.cpp -- Implementation of class KMailDrop.
* Author:  Sirtaj Singh Kang
* Version:  $Id$
* Generated:  Sat Nov 29 20:07:45 EST 1997
*/

#include<assert.h>
#include<qapplication.h>
#include <QList>

#include<kconfigbase.h>
#include<kdebug.h>

#include"utils.h"
#include"maildrop.h"
#include"mailsubject.h"
#include"settings.h"

KMailDrop::KMailDrop()
  : _lastCount(0)
{
  connect(this, SIGNAL(changed( int, KMailDrop* )), SLOT(setCount( int, KMailDrop* )));
}

KMailDrop::~KMailDrop()
{
  // Empty.
}

void KMailDrop::setCount(int count, KMailDrop*)
{
  _lastCount = count;
}

void KMailDrop::notifyClients()
{
  emit(notifyDisconnect());
}

//void KMailDrop::addConfigPage(KDropCfgDialog * dlg)
//{
//  dlg->addConfigPage(new KGeneralCfg(this));
//  dlg->addConfigPage(new KCommandsCfg(this));
//}

void KMailDrop::forceCountZero()
{
  emit changed( 0, this );
}

bool KMailDrop::readConfig( AccountSettings *settings )
{
  _settings = settings;
  emit(configChanged());

  return true;
}

bool KMailDrop::writeConfigGroup(AccountSettings *) const
{
  return true;
}

QString KMailDrop::realName() const
{
	return _settings->accountName();
}

QString KMailDrop::soundFile() const
{
	return _settings->sound();
}

QString KMailDrop::newMailCmd() const
{
	return _settings->command();
}

QVector<KornMailSubject> * KMailDrop::doReadSubjects(bool * /*stop*/)
{
	return new QVector<KornMailSubject>(); // empty vector
}

QVector<KornMailSubject> * KMailDrop::readSubjects(bool * stop)
{
	// remember timer status
	bool timerWasRunning = running();

	// stop timer to avoid conflicts with reading mesage count
	if (timerWasRunning)
		stopMonitor();

	// read the subjects
	QVector<KornMailSubject> * result = doReadSubjects(stop);
	int newcount = result->size();

	// if the mail count has changed: notify the button!
	if( newcount != count() && (!stop || !*stop) && synchrone() )
	{ //asynchrone connections don't have a list at this time
		emit changed( newcount, this );
	}

	// if the timer was previously running, start it again
	if (timerWasRunning)
		startMonitor();
	return result;
}


bool KMailDrop::deleteMails(QList<QVariant> * /*ids*/, bool * /*stop*/)
{
	return false;
}

QString KMailDrop::readMail(const QVariant /*id*/, bool * /*stop*/)
{
	return "";
}

void KMailDrop::setResetCounter(int val)
{
	_settings->setReset( val );
}

#include "maildrop.moc"
