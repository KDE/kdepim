/*  This file is part of the libkmobile library.
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

#include <qstring.h>
#include <qstringlist.h>

#include <klibloader.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <kio/global.h>
#include <kdebug.h>
#include <klocale.h>

#include "skeleton.h"


/* This is a loaded library, which is initialized with the line below */
K_EXPORT_COMPONENT_FACTORY( libkmobile_skeleton, KMobileSkeleton() );

/* createObject needs to be reimplemented by every KMobileDevice driver */
QObject *KMobileSkeleton::createObject( QObject *parent, const char *name,
        const char *, const QStringList &args )
{
  return new KMobileSkeleton( parent, name, args );
}


/**
 *  The KDE skeleton mobile device driver.
 */

KMobileSkeleton::KMobileSkeleton(QObject *obj, const char *name, const QStringList &args )
	: KMobileDevice(obj, name, args)
{
  // set initial device info
  m_deviceClassName = i18n("Skeleton Device");
  m_deviceName = i18n("LX-50-Moohoo Addressbook (Skeleton)");
  m_deviceRevision = "0.1";
  setClassType( Organizer );
  setCapabilities( hasAddressBook | hasNotes );
}

KMobileSkeleton::~KMobileSkeleton()
{
}

// connect the device and ask user to turn device on (if necessary)
bool KMobileSkeleton::connectDevice(QWidget *parent)
{
  if (KMessageBox::Continue != KMessageBox::warningContinueCancel(parent,
	i18n("Please turn on your %1 on now and press continue to proceed.").arg(m_deviceName),
	m_deviceClassName ) )
	return false;
  // connect it now...
  m_connected = true;
  return m_connected;
}

// disconnect the device and return true, if sucessful
bool KMobileSkeleton::disconnectDevice(QWidget *)
{
  m_connected = true;
  return true;
}

// returns true, if this device is read-only (default: false)
bool KMobileSkeleton::isReadOnly()
{
  return true;
}

QString KMobileSkeleton::iconFileName() const
{
  return "mobile_unknown";
}

/*
 * Addressbook / Phonebook support
 */
int KMobileSkeleton::numAddresses()
{
  return 10; /* number of addresses we simulate */
}

int KMobileSkeleton::readAddress( int index, KABC::Addressee &addr )
{
  // index is zero-based
  if (index<0 || index>=numAddresses())
    return KIO::ERR_DOES_NOT_EXIST;

  // now build our own sample name
  addr.setFamilyName(QString("Meyer_%1").arg(index+1));  
  addr.setGivenName("Peter");
  addr.setFormattedName("Peter "+addr.familyName());
  addr.setNickName("PeterM");
  addr.setBirthday(QDateTime(QDate(1970,7,22)));
  addr.setRole("KDE Software Developer");
  addr.setOrganization("KDE.ORG");
  addr.setNote("the best KDE developer ever");
  addr.setUrl(KURL("www.kde.org"));
  addr.insertEmail("peterm@kde.org");
  addr.insertPhoneNumber(KABC::PhoneNumber("+49 6110 12345"));

  // the Revision might be important for syncronisations
  addr.setRevision(QDateTime(QDate(2003,1,1)));

  return 0;
}

int KMobileSkeleton::storeAddress( int, const KABC::Addressee &, bool )
{
  /* this is a read-only device */
  return KIO::ERR_WRITE_ACCESS_DENIED;
}

/*
 * Notes support
 */
int KMobileSkeleton::numNotes()
{
  return 100;
}

int KMobileSkeleton::readNote( int index, QString &note )
{
  // index is zero-based, and we only have one simulated note
  if (index<0 || index>=numNotes())
    return KIO::ERR_DOES_NOT_EXIST;

  note = QString("NOTE #%1\n"
		 "--------\n"
	"This is a sample note #%2\n\n"
	"DeviceClassName: %3\n"
	"Device Driver  : %4\n"
	"Device Revision: %5\n")
	.arg(index).arg(index)
	.arg(deviceClassName()).arg(deviceName()).arg(revision()); 
  return 0;
}

#include "skeleton.moc"
