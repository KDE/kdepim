/*
* dropman.cpp -- Implementation of class KDropManager.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Fri May  1 19:33:10 EST 1998
*/

#include<assert.h>
#include<stdlib.h>

#include<klocale.h>
#include<kconfigbase.h>
#include<kmessagebox.h>

#include"utils.h"
#include"dropman.h"
#include"unixdrop.h"
#include"qmail.h"
#include"pop.h"
#include"news.h"
#include"imap.h"
#include"edrop.h"
#include"kio.h"
#include"dcopdrop.h"

KDropManager::KDropManager()
{
	_prototypes.setAutoDelete( true );
	_monitors.setAutoDelete( true );

	addPrototype( new KUnixDrop );
	addPrototype( new KQMailDrop );
	addPrototype( new KPop3Drop );
	addPrototype( new KImap4Drop );
	addPrototype( new KNewsDrop );
	addPrototype( new KExternDrop );
	addPrototype( new KKioDrop );
	addPrototype( new DCOPDrop );
}

KDropManager::~KDropManager()
{
}

bool KDropManager::readOldConfig( KConfigBase& config, const QString & group )
{
  KConfigGroupSaver save( &config, group );

  // split box names up
  QStringList boxes = config.readListEntry(fu(BoxConfigKey), ',');

  if (boxes.isEmpty())
    return false;

  // read and create boxes;
  QString type;
  QString grp;
  QString defAudio = config.readEntry(fu("audio"));
  QString defCmd = config.readPathEntry(fu("command"));

  int defPoll = KPollableDrop::DefaultPoll;

  if (config.hasKey(fu("polltime")))
    defPoll = config.readNumEntry(fu("polltime"), KPollableDrop::DefaultPoll);

  for (QStringList::ConstIterator it = boxes.begin(); it != boxes.end(); ++it) {

    // new monitor
    KUnixDrop * drop = dynamic_cast<KUnixDrop *>(newMonitor(fu("mbox")));

    if( drop == 0 ) {
      qWarning("KDropManager::readOldConfig: creation of unix box failed.");
      return false;
    }

    // configure
    config.setGroup(*it);

    drop->setFile(config.readEntry(fu("box")));

    drop->setCaption(config.readEntry(fu("Name"), i18n("MBOX Monitor")));

    drop->setNewMailCmd(config.readEntry(fu("audio"), defAudio));
    drop->setClickCmd(config.readPathEntry(fu("command"), defCmd));
    drop->setFreq(config.readNumEntry(fu("polltime"), defPoll));
  }

  return true;
}

bool KDropManager::readConfig( KConfigBase& config, const QString & group )
{
  KConfigGroupSaver save( &config, group );

  // check if new config system works
  if( !config.hasKey(fu(BoxNumConfigKey)) ) {
    return readOldConfig( config, group );
  }

  // split box names up
  int boxcount = config.readNumEntry(fu(BoxNumConfigKey));

  if( boxcount <= 0 ) {
    return false;
  }

  // read and create boxes;
  QString type;
  QString grp;
  QString c;

  for( int count = 0; count < boxcount; count++ ) {

    // get type
    grp = "box-";
    c.setNum( count );
    grp += c;

    config.setGroup( grp );
    type = config.readEntry(fu("type"));

    if ( type.isEmpty() ) {
      qWarning("KDropManager::readConfig: box %d has no type.", count);	
      continue;
    }

    // new monitor
    KMailDrop * drop = newMonitor(type);

    if (drop == 0) {
      qWarning("KDropManager::readConfig: creation of %s box failed.", type.ascii());
      continue;
    }

    drop->readConfigGroup( config );
  }

  return true;
}

  bool
KDropManager::writeConfig(KConfigBase & config, const QString & group) const
{
  KConfigGroupSaver ( &config, group );

  QPtrListIterator<KMailDrop> iter(_monitors);

  QString grp;
  QString boxes;

  int count = 0;
  QString c;

  for (; iter.current(); ++iter) {

    boxes += iter.current()->caption();
    boxes += BoxConfigSep;

    grp = "box-";
    c.setNum( count++ );
    grp += c;

    config.setGroup( grp );

    iter.current()->writeConfigGroup( config );

  }

  config.setGroup( group );
  config.writeEntry(fu(BoxNumConfigKey), count );
  config.sync();
  return true;
}

bool KDropManager::createBasicConfig()
{
	KUnixDrop *drop = dynamic_cast<KUnixDrop *>(newMonitor(fu("mbox")));

	if ( !drop ) return false;

	drop->setCaption(i18n("Inbox"));

	// find mailbox
	QString s = fu(getenv("MAIL"));

	if (s.isEmpty()) {
		KMessageBox::sorry( 0,
			i18n("Mail Monitor was unable to automatically detect\n"
				"your email settings. You need to configure mail\n"
				"monitor for your email accounts. To do that,\n"
				"right click on the applet in the panel tray and\n"
				"select 'Configure Korn...'."),
			i18n("Mail Monitor"));
	}
	else {
		drop->setFile( s );
	}

	return true;
}

KMailDrop *KDropManager::newMonitor(const QString & type)
{
	KMailDrop *proto = _prototypes.find( type );

	if( !proto ) {
		return 0;
	}

	KMailDrop *newdrop = proto->clone();

	if( newdrop != 0 ) {
		_monitors.append( newdrop );
		connect( newdrop, SIGNAL(configChanged()),
			this, SLOT(raiseConfig()) );
	}

	emit monitorCreated();

	return newdrop;
}

KMailDrop *KDropManager::cloneMonitor( const KMailDrop *monitor )
{
	assert( monitor );

	// clone monitor

	KMailDrop *newdrop = monitor->clone();

	if( newdrop == 0 ) {
		qWarning("cloneMonitor: couldn't clone %s.", monitor->caption().ascii() );
		return 0;
	}

	// register 

	_monitors.append( newdrop );
	connect( newdrop, SIGNAL(configChanged()),
			this, SLOT(raiseConfig()) );

	// append " (clone)" to the caption to keep it
	// legal.

	QString caption = newdrop->caption();
	caption += i18n( " (clone)" );
	newdrop->setCaption( caption );

	emit monitorCreated();

	return newdrop;
}

bool KDropManager::deleteMonitor( KMailDrop *monitor )
{
	// disconnect clients
	monitor->notifyClients();

	// delete monitor
	if( _monitors.removeRef( monitor ) == false ) {
		return false;
	}

	emit monitorDeleted();

	return true;
}

void KDropManager::addPrototype( const KMailDrop *p )
{
	_prototypes.insert( p->type(), p );
	_types.append( p->type() );
}

void KDropManager::raiseConfig()
{
	emit configChanged();
}

const char *KDropManager::BoxConfigKey = "boxes";
const char *KDropManager::BoxNumConfigKey = "numboxes";
const char KDropManager::BoxConfigSep = '|';
#include "dropman.moc"
