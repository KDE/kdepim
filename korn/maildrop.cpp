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

const char *KMailDrop::TypeConfigKey = "type";
const char *KMailDrop::CaptionConfigKey = "caption";
const char *KMailDrop::ClickConfigKey = "onclick";
const char *KMailDrop::NewMailConfigKey = "newcommand";
const char *KMailDrop::SoundFileConfigKey = "sound";
const char *KMailDrop::BgColourConfigKey = "bgcolour";
const char *KMailDrop::FgColourConfigKey = "fgcolour";
const char *KMailDrop::NBgColourConfigKey = "newmailbgcolour";
const char *KMailDrop::NFgColourConfigKey = "newmailfgcolour";
const char *KMailDrop::IconConfigKey = "icon";
const char *KMailDrop::NewMailIconConfigKey = "newmailicon";
const char *KMailDrop::DisplayStyleConfigKey = "displaystyle";
const char *KMailDrop::ResetCounterConfigKey = "resetcounter";
const char *KMailDrop::PassivePopupConfigKey = "passivepopup";
const char *KMailDrop::PassiveDateConfigKey = "passivedate";
const char *KMailDrop::UseBoxSettingsConfigKey = "boxsettings";
const char *KMailDrop::RealNameConfigKey = "name";

KMailDrop::KMailDrop()
  : _style(Plain),
    _lastCount(0)
{
  connect(this, SIGNAL(changed( int, KMailDrop* )), SLOT(setCount( int, KMailDrop* )));
  
  //Set default colours; this prevents black (QColor::invalid) boxes after creating a new box.
  _bgColour  = QApplication::palette().color( QPalette::Active, QPalette::Window );
  _fgColour  = QApplication::palette().color( QPalette::Active, QPalette::WindowText );
  _nbgColour = QApplication::palette().color( QPalette::Active, QPalette::Window );
  _nfgColour = QApplication::palette().color( QPalette::Active, QPalette::WindowText );
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

void KMailDrop::readGeneralConfigGroup( const KConfigBase& cfg )
{
  _passivePopup = cfg.readEntry(fu(PassivePopupConfigKey), false );
  _passiveDate = cfg.readEntry(fu(PassiveDateConfigKey), false );
  _soundFile = cfg.readEntry(fu(SoundFileConfigKey), QString());
  _nMailCmd = cfg.readEntry(fu(NewMailConfigKey), QString());

  emit(configChanged());
}

bool KMailDrop::readConfigGroup(const KConfigBase & c)
{
  _caption    = c.readEntry(fu(CaptionConfigKey), QString());
  _clickCmd   = c.readPathEntry(fu(ClickConfigKey));
  _style      = Style(c.readEntry(fu(DisplayStyleConfigKey), (unsigned int)Plain));
  _bgColour   = c.readEntry(fu(BgColourConfigKey), QApplication::palette().color(QPalette::Active, QPalette::Window));
  _fgColour   = c.readEntry(fu(FgColourConfigKey), QApplication::palette().color(QPalette::Active, QPalette::WindowText));
  _nbgColour  = c.readEntry(fu(NBgColourConfigKey), QApplication::palette().color(QPalette::Active, QPalette::Window));
  _nfgColour  = c.readEntry(fu(NFgColourConfigKey), QApplication::palette().color(QPalette::Active, QPalette::WindowText));
  _icon       = c.readEntry(fu(IconConfigKey), QString());
  _nIcon      = c.readEntry(fu(NewMailIconConfigKey), QString());
  _realName   = c.readEntry(fu(RealNameConfigKey), QString());

  if( !c.readEntry(fu(UseBoxSettingsConfigKey), true ) )
  	readGeneralConfigGroup( c );

  emit(configChanged());

  return true;
}

bool KMailDrop::writeConfigGroup(KConfigBase & c) const
{
  c.writeEntry(fu(TypeConfigKey),         type());
  c.writeEntry(fu(CaptionConfigKey),      caption());
  c.writePathEntry(fu(ClickConfigKey),        clickCmd());
  c.writePathEntry(fu(NewMailConfigKey),      newMailCmd());
  c.writePathEntry(fu(SoundFileConfigKey),    soundFile());
  c.writeEntry(fu(DisplayStyleConfigKey), (int)_style);
  c.writeEntry(fu(BgColourConfigKey),     _bgColour);
  c.writeEntry(fu(FgColourConfigKey),     _fgColour);
  c.writeEntry(fu(NBgColourConfigKey),    _nbgColour);
  c.writeEntry(fu(NFgColourConfigKey),    _nfgColour);
  c.writeEntry(fu(IconConfigKey),         _icon);
  c.writeEntry(fu(NewMailIconConfigKey),  _nIcon);
  c.writeEntry(fu(PassivePopupConfigKey), _passivePopup );
  c.writeEntry(fu(PassiveDateConfigKey),  _passiveDate );

  return true;
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

void KMailDrop::setCaption(QString s)
{
  _caption = s;
  emit(configChanged());
}

void KMailDrop::setClickCmd(QString s)
{
  _clickCmd = s;
  emit(configChanged());
}

void KMailDrop::setNewMailCmd(QString s)
{
  _nMailCmd = s;
  emit(configChanged());
}

void KMailDrop::setSoundFile(QString s)
{
  _soundFile = s;
  emit(configChanged());
}

void KMailDrop::setDisplayStyle(Style s)
{
  _style = s;
  emit(configChanged());
}

void KMailDrop::setBgColour(QColor c)
{
  _bgColour = c;
  emit(configChanged());
}

void KMailDrop::setFgColour(QColor c)
{
  _fgColour = c;
  emit(configChanged());
}

void KMailDrop::setNewBgColour(QColor c)
{
  _nbgColour = c;
  emit(configChanged());
}

void KMailDrop::setNewFgColour(QColor c)
{
  _nfgColour = c;
  emit(configChanged());
}

void KMailDrop::setIcon(QString s)
{
  _icon = s;
  emit(configChanged());
}

void KMailDrop::setNewIcon(QString s)
{
  _nIcon = s;
  emit(configChanged());
}

void KMailDrop::setPassivePopup( bool pp )
{
  _passivePopup = pp;
  emit(configChanged());
}

void KMailDrop::setPassiveDate( bool pd )
{
  _passiveDate = pd;
  emit(configChanged());
}

void KMailDrop::setRealName(QString str)
{
	_realName = str;
}

#include "maildrop.moc"
