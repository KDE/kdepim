/*  This file is part of the KDE KMobile library
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

#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>
#include <kinstance.h>
#include <kabc/vcardconverter.h>

#include "kio_mobile.h"

#include <kdepimmacros.h>

using namespace KIO;

#define KIO_MOBILE_DEBUG_AREA 7126
#define PRINT_DEBUG kdDebug(KIO_MOBILE_DEBUG_AREA) << "kio_mobile: "

extern "C" { KDE_EXPORT int kdemain(int argc, char **argv); }

/**
 * The main program.
 */
int kdemain(int argc, char **argv)
{
  KInstance instance( "kio_mobile" );

  PRINT_DEBUG << "Starting " << getpid() << endl;

  if (argc != 4) {
	fprintf(stderr, "Usage kio_mobile protocol pool app\n");
	return -1;
  }
  // let the protocol class do its work
  KMobileProtocol slave(argv[2], argv[3]);

  slave.dispatchLoop();

  PRINT_DEBUG << "Done" << endl;
  return 0;
}


/**
 * Initialize the mobile slave
 */
KMobileProtocol::KMobileProtocol(const QCString &pool, const QCString &app)
  : SlaveBase( "mobile", pool, app)
{
}

KMobileProtocol::~KMobileProtocol()
{
}

/*
 * getDeviceAndRessource("mobile:/<devicename>/<resource>/...") - split
 */
int KMobileProtocol::getDeviceAndRessource(const QString &_path,
	QString &devName, QString &resource, QString &devPath, 
	KMobileDevice::Capabilities &devCaps)
{
//  PRINT_DEBUG << QString("###getDeviceAndRessource### %1\n").arg(_path);
  QStringList path = QStringList::split('/', _path, false);

  devName = resource = devPath = QString::null;
  devCaps = KMobileDevice::hasNothing;

  if (path.count() >= 1)  { devName = path[0];  path.pop_front(); };
  if (path.count() >= 1)  { resource = path[0]; path.pop_front(); };
  if (path.count() >= 1)  devPath = path.join("/");

  if (devName.isEmpty())
	return 0;

  int _caps = m_dev.capabilities(devName);

  if (resource.isEmpty()) {
	devCaps = (KMobileDevice::Capabilities) _caps;
	return 0;
  }

  for (int i=0; i<31; i++) {
	int cap = 1UL << i;
	if ((_caps & cap) == 0)
		continue;
	QString capname = m_dev.nameForCap(devName,cap);
	if (capname != resource)
		continue;
	devCaps = (KMobileDevice::Capabilities) cap;
	return 0;
  }

  return KIO::ERR_DOES_NOT_EXIST;
}


static
void addAtom(KIO::UDSEntry& entry, unsigned int ID, long l, const QString& s = QString::null)
{
	KIO::UDSAtom	atom;
	atom.m_uds = ID;
	atom.m_long = l;
	atom.m_str = s;
	entry.append(atom);
}

static
void createDirEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime)
{
	entry.clear();
	addAtom(entry, KIO::UDS_NAME, 0, name);
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDS_ACCESS, 0500);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
	addAtom(entry, KIO::UDS_URL, 0, url);
	addAtom(entry, KIO::UDS_USER, 0, getenv("USER"));
	addAtom(entry, KIO::UDS_GROUP, 0, getenv("USER"));
	PRINT_DEBUG << QString("createDirEntry: File: %1  MIME: %2  URL: %3\n").arg(name).arg(mime).arg(url);
//	addAtom(entry, KIO::UDS_SIZE, 0);
	addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, mime);
}

static
void createFileEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime, 
		const unsigned long size = 0)
{
	entry.clear();
	addAtom(entry, KIO::UDS_NAME, 0, name);
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFREG);
	addAtom(entry, KIO::UDS_URL, 0, url);
	addAtom(entry, KIO::UDS_ACCESS, 0400);
	addAtom(entry, KIO::UDS_USER, 0, getenv("USER"));
	addAtom(entry, KIO::UDS_GROUP, 0, getenv("USER"));
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
	if (size) addAtom(entry, KIO::UDS_SIZE, size);
	addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, mime);
	PRINT_DEBUG << QString("createFileEntry: File: %1, Size: %2,  MIME: %3\n").arg(name).arg(size).arg(mime);
}


/**
 * Get the information contained in the URL.
 */
void KMobileProtocol::get(const KURL &url)
{
  PRINT_DEBUG << "###############################\n";
  PRINT_DEBUG << QString("get(%1)\n").arg(url.path());

  KMobileDevice::Capabilities devCaps;
  QString devName, resource, devPath;

  int err = getDeviceAndRessource(url.path(), devName, resource, devPath, devCaps);
  if (err) {
	error(err, url.path());
	return;
  }

  if (devName.isEmpty() || resource.isEmpty()) {
	error(KIO::ERR_DOES_NOT_EXIST, url.path());
	return;
  }
  
  // collect the result
  QCString result;
  QString mime;
  switch (devCaps) {
    case KMobileDevice::hasAddressBook:	err = getVCard(devName, result, mime, devPath);
					break;
    case KMobileDevice::hasCalendar:	err = getCalendar(devName, result, mime, devPath);
					break;
    case KMobileDevice::hasNotes:	err = getNote(devName, result, mime, devPath);
					break;
    case KMobileDevice::hasFileStorage: err = getFileStorage(devName, result, mime, devPath);
					break;
    default:				err = KIO::ERR_CANNOT_ENTER_DIRECTORY; /* TODO */
  }

  if (err) {
	error(err, url.path());
	return;
  }

  // tell the mimetype
  mimeType(mime);

  // tell the length
  KIO::filesize_t processed_size = result.length();
  totalSize(processed_size);

  // tell the contents of the URL
  QByteArray array;
  array.setRawData( result.data(), result.length() );
  data(array);
  array.resetRawData( result.data(), result.length() );
  processedSize( processed_size );
  // tell we are finished
  data(QByteArray());

  // tell we are finished
  finished();
}


/*
 * listRoot() - gives listing of all devices
 */
void KMobileProtocol::listRoot(const KURL& url)
{
  PRINT_DEBUG << QString("########## listRoot(%1) for %2:/\n").arg(url.path()).arg(url.protocol());

  KIO::UDSEntry entry;

  QStringList deviceNames = m_dev.deviceNames();
  unsigned int dirs = deviceNames.count();
  totalSize(dirs);

  int classMask = KMobileDevice::Unclassified;
  /* handle all possible protocols here and just add a <protocol>.protocol file */
  if (url.protocol() == "cellphone")		// cellphone:/
	classMask = KMobileDevice::Phone;
  if (url.protocol() == "organizer" ||		// organizer:/
      url.protocol() == "pda")			// pda:/
	classMask = KMobileDevice::Organizer;
  if (url.protocol() == "camera")		// camera:/
	classMask = KMobileDevice::Camera;

  for (unsigned int i=0; i<dirs; i++) {

	QString devName = deviceNames[i];

	if (classMask != KMobileDevice::Unclassified && 
	    m_dev.classType(devName) != classMask)
		continue;	

        createDirEntry(entry, devName, "mobile:/"+devName, 
			KMOBILE_MIMETYPE_DEVICE_KONQUEROR(devName));
        listEntry(entry, false);

	processedSize(i+1);
  }
  listEntry(entry, true);
  finished();
}


/*
 * folderMimeType() - returns mimetype of the folder itself
 */
QString KMobileProtocol::folderMimeType(int cap)
{
  QString mimetype;
  switch (cap) {
    case KMobileDevice::hasAddressBook:	mimetype = KMOBILE_MIMETYPE_INODE "addressbook";
					break;
    case KMobileDevice::hasCalendar:	mimetype = KMOBILE_MIMETYPE_INODE "calendar";
					break;
    case KMobileDevice::hasNotes:	mimetype = KMOBILE_MIMETYPE_INODE "notes";
					break;
    case KMobileDevice::hasFileStorage:
    default:				mimetype = "inode/directory";
  }
  return mimetype;
}

/*
 * entryMimeType() - returns mimetype of the entries in the given folder
 */
QString KMobileProtocol::entryMimeType(int cap)
{
  QString mimetype;
  switch (cap) {
    case KMobileDevice::hasAddressBook:	mimetype = "text/x-vcard";
					break;
    case KMobileDevice::hasCalendar:	mimetype = "text/x-vcalendar";
					break;
    case KMobileDevice::hasNotes:	mimetype = "text/plain";
					break;
    case KMobileDevice::hasFileStorage:
    default:				mimetype = "text/plain";
  }
  return mimetype;
}

/*
 * listTopDeviceDir("mobile:/<devicename>") - sub-directory of a devices
 */

void KMobileProtocol::listTopDeviceDir(const QString &devName)
{
  PRINT_DEBUG << QString("listTopDeviceDir(%1)\n").arg(devName);

  KIO::UDSEntry entry;
  unsigned int caps = m_dev.capabilities(devName);

  for (int i=0; i<31; i++) {
	unsigned int cap = 1UL<<i;
	if ((caps & cap) == 0)
		continue;

	QString filename = m_dev.nameForCap(devName, cap);
	QString mimetype = folderMimeType(cap);
	
        createDirEntry(entry, filename, QString("mobile:/%1/%2/").arg(devName).arg(filename), mimetype);
        listEntry(entry, false);
  }
  listEntry(entry, true);
  finished();
}


/*
 * listEntries("mobile:/<devicename>/<resource>") - resources of a device
 */
void KMobileProtocol::listEntries(const QString &devName, 
	const QString &resource, const QString &devPath,
	const KMobileDevice::Capabilities devCaps)
{
  PRINT_DEBUG << QString("listEntries(%1,%2,%3)\n").arg(devName).arg(resource).arg(devPath);
  switch (devCaps) {
    case KMobileDevice::hasAddressBook:	listAddressBook(devName, resource);
					break;
    case KMobileDevice::hasCalendar:	listCalendar(devName, resource);
					break;
    case KMobileDevice::hasNotes:	listNotes(devName, resource);
					break;
    case KMobileDevice::hasFileStorage: listFileStorage(devName, resource, devPath);
					break;
    default:				error( ERR_CANNOT_ENTER_DIRECTORY,
					  QString("/%1/%2").arg(devName).arg(resource) );
  }
}

/*
 * listAddressBook("mobile:/<devicename>/Addressbook) - list the addressbook
 */
void KMobileProtocol::listAddressBook(const QString &devName, const QString &resource)
{
  PRINT_DEBUG << QString("listAddressBook(%1)\n").arg(devName);
  
  KIO::UDSEntry entry;

  int fieldwidth;
  int entries = m_dev.numAddresses(devName);
  if (entries>=1000) fieldwidth=4; else 
   if (entries>=100) fieldwidth=3; else
    if (entries>=10) fieldwidth=2; else fieldwidth=1;
  totalSize(entries);
//  QRegExp rx; rx.setPattern( ".*FN:([\\w\\s]*)[\\n\\r]{2}.*" );
  QString name;
  for (int i=0; i<entries; i++) {

#if 0
  	QString content = m_dev.readAddress(devName, i);
        if ( rx.search( content ) < 0 )
		name = QString::null;
	else
		name = "_" + rx.cap(1);
#endif

	QString filename = QString("%1%2.vcf").arg(i,fieldwidth).arg(name);
	for (int p=0; p<fieldwidth; p++) { 
		if (filename[p]==' ') filename[p]='0'; else break; 
	}
	QString url = QString("mobile:/%1/%2/%3").arg(devName).arg(resource).arg(filename);

	createFileEntry(entry, filename, url, entryMimeType(KMobileDevice::hasAddressBook),
			400 /*content.utf8().length()*/ );
        listEntry(entry, false);

	processedSize(i+1);
  }
  listEntry(entry, true);
  finished();
}

/*
 * getVCard() - gives the vCard of the given file
 */
int KMobileProtocol::getVCard( const QString &devName, QCString &result, QString &mime, const QString &path )
{
  PRINT_DEBUG << QString("getVCard(%1)\n").arg(path);

  int index = path.find('.');
  if (index>0)
	index = path.left(index).toInt();
  if (index<0 || index>=m_dev.numAddresses(devName))
	return KIO::ERR_DOES_NOT_EXIST;

  QString str = m_dev.readAddress(devName, index);
  if (str.isEmpty())
	return KIO::ERR_INTERNAL;
  result = str.utf8();
  mime = entryMimeType(KMobileDevice::hasAddressBook);
//  setMetaData("plugin", "const QString &key, const QString &value);
  return 0;
}

/*
 * listCalendar("mobile:/<devicename>/Calendar) - list the calendar entries
 */
void KMobileProtocol::listCalendar( const QString &devName, const QString &resource)
{
  PRINT_DEBUG << QString("listCalendar(%1)\n").arg(devName);
  
  KIO::UDSEntry entry;

  int entries = m_dev.numCalendarEntries(devName);
  totalSize(entries);
  for (int i=0; i<entries; i++) {

	QString filename = QString("%1_%2.vcs").arg(i).arg(i18n("calendar"));
	QString url = QString("mobile:/%1/%2/%3").arg(devName).arg(resource).arg(filename);

	createFileEntry(entry, filename, url, entryMimeType(KMobileDevice::hasCalendar));
        listEntry(entry, false);

	processedSize(i+1);
  }
  listEntry(entry, true);
  finished();
}

/*
 * getCalendar() - reads a calendar entry
 */
int KMobileProtocol::getCalendar( const QString &devName, QCString &result, QString &mime, const QString &path)
{
  PRINT_DEBUG << QString("getCalendar(%1, #%2)\n").arg(devName).arg(path);

  /* TODO */
  Q_UNUSED(result);
  Q_UNUSED(mime);
  return KIO::ERR_CANNOT_ENTER_DIRECTORY;
}


/*
 * listNotes("mobile:/<devicename>/Notes) - list the notes
 */
void KMobileProtocol::listNotes( const QString &devName, const QString &resource)
{
  PRINT_DEBUG << QString("listNotes(%1)\n").arg(devName);
  
  KIO::UDSEntry entry;

  int entries = m_dev.numNotes(devName);
  totalSize(entries);
  for (int i=0; i<entries; i++) {

        QString note /*= m_dev.readNote(devName, i)*/;

	QString filename = QString("%1_%2.txt").arg(i).arg(i18n("note"));
	QString url = QString("mobile:/%1/%2/%3").arg(devName).arg(resource).arg(filename);

	createFileEntry(entry, filename, url, entryMimeType(KMobileDevice::hasNotes),
			0 /*note.utf8().length()*/);
        listEntry(entry, false);

	processedSize(i+1);
  }
  listEntry(entry, true);
  finished();
}

/*
 * getNote() - gives the Note of the given file
 */
int KMobileProtocol::getNote( const QString &devName, QCString &result, QString &mime, const QString &path )
{
  PRINT_DEBUG << QString("getNote(%1)\n").arg(path);

  int index = path.find('_');
  if (index>0)
	index = path.left(index).toInt();
  if (index<0 || index>=m_dev.numNotes(devName))
	return KIO::ERR_DOES_NOT_EXIST;

  QString note = m_dev.readNote(devName, index);
  if (note.isEmpty())
	return KIO::ERR_DOES_NOT_EXIST;

  result = note.utf8();
  mime = entryMimeType(KMobileDevice::hasNotes);
  return 0;
}

/*
 * listFileStorage("mobile:/<devicename>/Files) - list the files on the device
 */
void KMobileProtocol::listFileStorage(const QString &devName, const QString &resource, const QString &devPath)
{
  PRINT_DEBUG << QString("listFileStorage(%1,%2)\n").arg(devName).arg(devPath);
  
  /* TODO */
  error( KIO::ERR_DOES_NOT_EXIST, QString("/%1/%2/%3").arg(devName).arg(resource).arg(devPath) );
}

/*
 * getFileStorage() - gives the file contents of the given file
 */
int KMobileProtocol::getFileStorage(const QString &devName, QCString &result, QString &mime, const QString &path)
{
  PRINT_DEBUG << QString("getFileStorage(%1)\n").arg(path);

  /* TODO */
  Q_UNUSED(devName);
  Q_UNUSED(result);
  Q_UNUSED(mime);
  return KIO::ERR_CANNOT_ENTER_DIRECTORY;
}


/**
 * Test if the url contains a directory or a file.
 */
void KMobileProtocol::stat( const KURL &url )
{
  PRINT_DEBUG << "###############################\n";
  PRINT_DEBUG << QString("stat(%1)\n").arg(url.path());

  KMobileDevice::Capabilities devCaps;
  QString devName, resource, devPath;

  int err = getDeviceAndRessource(url.path(), devName, resource, devPath, devCaps);
  if (err) {
	error(err, url.path());
	return;
  }

  QStringList path = QStringList::split('/', url.path(), false);
  QString filename = (path.count()>0) ? path[path.count()-1] : "/";
  QString fullPath = path.join("/");
  QString fullUrl = QString("mobile:/%1").arg(fullPath);

  UDSEntry entry;

  bool isDir = devPath.isEmpty();

  if (isDir) {
     createDirEntry(entry, filename, fullUrl, folderMimeType(devCaps));
  } else {
     createFileEntry(entry, filename, fullUrl, entryMimeType(devCaps));
  }

  statEntry(entry);
  finished();
}

/**
 * Get the mimetype.
 */
void KMobileProtocol::mimetype(const KURL &url)
{
  PRINT_DEBUG << "###############################\n";
  PRINT_DEBUG << QString("mimetype(%1)\n").arg(url.path());

  KMobileDevice::Capabilities devCaps;
  QString devName, resource, devPath;

  int err = getDeviceAndRessource(url.path(), devName, resource, devPath, devCaps);
  if (err) {
	error(err, url.path());
	return;
  }

  // tell the mimetype
  mimeType(entryMimeType(devCaps));
  finished();
}

/**
 * List the contents of a directory.
 */
void KMobileProtocol::listDir(const KURL &url)
{
  PRINT_DEBUG << "###############################\n";
  PRINT_DEBUG << QString("listDir(%1)\n").arg(url.path());

  if (!m_dev.isKMobileAvailable()) {
	error( KIO::ERR_CONNECTION_BROKEN, i18n("KDE Mobile Device Manager") );
	return;
  }

  KMobileDevice::Capabilities devCaps;
  QString devName, resource, devPath;

  int err = getDeviceAndRessource(url.path(), devName, resource, devPath, devCaps);
  if (err) {
	error(err, url.path());
	return;
  }

  if (devName.isEmpty()) {
	listRoot(url);
	return;
  }

#if 0  
  if (!dev) {
	error( KIO::ERR_DOES_NOT_EXIST, QString("/%1").arg(devName) );
	return;
  }
#endif

  if (resource.isEmpty()) {
	listTopDeviceDir(devName);
	return;
  }

  listEntries(devName, resource, devPath, devCaps);
}
