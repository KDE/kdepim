/* This file is part of the KDE libraries
    Copyright (C) 2003 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include <qstring.h>
#include <qstringlist.h>

#include <klibloader.h>
#include <kstandarddirs.h>
#include <kmobiledevice.h>
#include <kmessagebox.h>

#include <kio/global.h>
#include <kdebug.h>
#include <klocale.h>

#define KMOBILEDEVICE_DEBUG_AREA 5730
#define PRINT_DEBUG kdDebug(KMOBILEDEVICE_DEBUG_AREA) << "KMobileDevice: "

/**
 *  The base plugin class of all mobile device drivers. 
 */

KMobileDevice::KMobileDevice(QObject *obj, const char *name, const QStringList &args)
	: KLibFactory(obj,name),
           m_config(0L), d(0L)
{
  connect( this, SIGNAL( connectionChanged(bool) ),
	this, SLOT( slotConnectionChanged(bool) ) );
  connect( this, SIGNAL( message(int,const QString &) ),
	this, SLOT( slotMessage(int, const QString &) ) ); 

  setClassType(Unclassified);
  setCapabilities(hasNothing);
  m_deviceClassName = defaultClassName(Unclassified);
  m_deviceName = i18n("Unknown Device");
  m_deviceRevision = i18n("n/a");  /* not available */
  m_connectionName = i18n("Unknown Connection");

  // set the config file name
  m_configFileName = args[0];
  Q_ASSERT(!m_configFileName.isEmpty());
  m_config = new KConfig(m_configFileName);
  PRINT_DEBUG << QString("New configfile is %1\n").arg(m_configFileName);
}

KMobileDevice::~KMobileDevice()
{
  delete m_config;
}


bool KMobileDevice::connected()
{
  return m_connected;
}

// returns e.g. "Nokia mobile phone", "MP3 Player", "Handspring Organizer"
QString KMobileDevice::deviceClassName() const
{
  return m_deviceClassName;
}

// returns real device name, e.g. "Nokia 6310" or "Rio MP3 Player"
QString KMobileDevice::deviceName() const
{
  return m_deviceName;
}

QString KMobileDevice::revision() const
{
  return m_deviceRevision;
}

bool KMobileDevice::isSlowDevice() const
{
  return false;
}

bool KMobileDevice::isReadOnly() const
{
  return false;
}

bool KMobileDevice::configDialog( QWidget *parent )
{
  KMessageBox::information( parent, 
		i18n("This device does not need any configuration."),
		deviceName() );		 
  return true;
}

void KMobileDevice::setClassType( enum ClassType ct )
{
  m_classType = ct; 
};

enum KMobileDevice::ClassType KMobileDevice::classType() const
{
  return m_classType;
};

QString KMobileDevice::iconFileName() const
{
  return defaultIconFileName( classType() );
}

QString KMobileDevice::defaultIconFileName( ClassType ct )
{
  QString name;
  switch (ct) {
    case Phone:		name = "mobile_phone";		break;
    case Organizer:	name = "mobile_organizer";	break;
    case Camera:	name = "mobile_camera";		break;
    case MusicPlayer:	name = "mobile_mp3player";	break;
    case Unclassified:
    default:		name = KMOBILE_ICON_UNKNOWN;	break;
  }
  return name;
}

QString KMobileDevice::defaultClassName( ClassType ct )
{
  QString name;
  switch (ct) {
    case Phone:		name = i18n("Cellular Mobile Phone");	break;
    case Organizer:	name = i18n("Organizer");		break;
    case Camera:	name = i18n("Digital Camera");		break;
    case MusicPlayer:	name = i18n("Music/MP3 Player");	break;
    case Unclassified:
    default:		name = i18n("Unclassified");		break;
  }
  return name;
}

void KMobileDevice::setCapabilities( int caps )
{
  m_caps = caps;
};

int KMobileDevice::capabilities() const
{
  return m_caps;
};

const QString KMobileDevice::nameForCap(int cap) const
{
  switch (cap) {
    case hasAddressBook: return i18n("Contacts");
    case hasCalendar:	 return i18n("Calendar");
    case hasNotes:	 return i18n("Notes");
    case hasFileStorage: return i18n("Files");
    default:		 return i18n("Unknown");
  }
}

// returns an error string for the given error code
QString KMobileDevice::buildErrorString(KIO::Error err, const QString &errorText) const
{
  return KIO::buildErrorString( err, errorText);
}

/*
 * Addressbook / Phonebook support
 */
int KMobileDevice::numAddresses()
{
  return 0;
}

int KMobileDevice::readAddress( int, KABC::Addressee & )
{
  return KIO::ERR_UNSUPPORTED_ACTION;
}

int KMobileDevice::storeAddress( int, const KABC::Addressee &, bool )
{
  return KIO::ERR_UNSUPPORTED_ACTION;
}

/*
 * Calendar support
 */
int KMobileDevice::numCalendarEntries()
{
  return 0;
}

// TODO: TBD
//    virtual int readCalendarEntry( int index, <type> &entry );
//    virtual int storeCalendarEntry( int index, <type> &entry );

/*
 * Notes support
 */
int KMobileDevice::numNotes()
{
  return 0;
}

int KMobileDevice::readNote( int, QString & )
{
  return KIO::ERR_UNSUPPORTED_ACTION;
}

int KMobileDevice::storeNote( int, const QString & )
{
  return KIO::ERR_UNSUPPORTED_ACTION;
}

/*
 * File storage support
 * @param fileName  path and name of a file in the mobile device, e.g. "/MYFILE.TXT", "/mp3/song1.mp3"
 */

static
void addAtom(KIO::UDSEntry& entry, unsigned int ID, long l, const QString& s = QString::null)
{
	KIO::UDSAtom atom;
	atom.m_uds = ID;
	atom.m_long = l;
	atom.m_str = s;
	entry.append(atom);
}

void KMobileDevice::createDirEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime) const
{
	entry.clear();
	addAtom(entry, KIO::UDS_NAME, 0, name);
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDS_ACCESS, 0500);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
	addAtom(entry, KIO::UDS_URL, 0, url);
	PRINT_DEBUG << QString("createDirEntry: File: %1  MIME: %2  URL: %3\n").arg(name).arg(mime).arg(url);
//	addAtom(entry, KIO::UDS_SIZE, 0);
	addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, mime);
}

void KMobileDevice::createFileEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime, 
		const unsigned long size) const
{
	entry.clear();
	addAtom(entry, KIO::UDS_NAME, 0, name);
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFREG);
	addAtom(entry, KIO::UDS_URL, 0, url);
	addAtom(entry, KIO::UDS_ACCESS, 0400);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
	if (size) addAtom(entry, KIO::UDS_SIZE, size);
	addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, mime);
	PRINT_DEBUG << QString("createFileEntry: File: %1, Size: %2,  MIME: %3\n").arg(name).arg(size).arg(mime);
}


void KMobileDevice::listDir( const QString & )
{
  emit error(KIO::ERR_CANNOT_ENTER_DIRECTORY,QString::null);
}

void KMobileDevice::mkdir( const QString &, int )
{
  emit error(KIO::ERR_COULD_NOT_MKDIR, QString::null);
}

void KMobileDevice::rename( const QString &, const QString &, bool )
{
  emit error(KIO::ERR_UNSUPPORTED_ACTION, QString::null);
}

void KMobileDevice::symlink( const QString &, const QString &, bool )
{
  emit error(KIO::ERR_UNSUPPORTED_ACTION, QString::null);
}

void KMobileDevice::del( const QString &, bool )
{
  emit error(KIO::ERR_UNSUPPORTED_ACTION, QString::null);
}

void KMobileDevice::stat( const QString & )
{
  emit error(KIO::ERR_UNSUPPORTED_ACTION, QString::null);
}

void KMobileDevice::chmod( const QString &, int )
{
  emit error(KIO::ERR_UNSUPPORTED_ACTION, QString::null);
}

void KMobileDevice::get( const QString & )
{
  emit error(KIO::ERR_UNSUPPORTED_ACTION, QString::null);
}

void KMobileDevice::put( const QString &, int, bool, bool )
{
  emit error(KIO::ERR_UNSUPPORTED_ACTION, QString::null);
}

void KMobileDevice::mimetype( const QString & )
{
  emit error(KIO::ERR_UNSUPPORTED_ACTION, QString::null);
}

void KMobileDevice::special( const QByteArray & )
{
  emit error(KIO::ERR_UNSUPPORTED_ACTION, QString::null);
}





void KMobileDevice::slotConnectionChanged( bool conn_established )
{
  PRINT_DEBUG << QString("KMobile: Connection to %1 via %2 %3.\n")
		.arg(deviceName())     // e.g. "Nokia 6310", "Opie"
		.arg(m_connectionName) // e.g. "IRDA", "USB", "Cable", "gnokii", "gammu", ...
		.arg(conn_established ? "ESTABLISHED":"LOST");
}

void KMobileDevice::slotMessage( int msgLevel, const QString &msg )
{
  PRINT_DEBUG << QString("KMobile: <%1>: %2.\n")
		.arg(msgLevel).arg(msg);
}

#include "kmobiledevice.moc"
