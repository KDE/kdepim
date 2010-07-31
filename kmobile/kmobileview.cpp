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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <tqiconview.h>
#include <tqstringlist.h>

#include <ktrader.h>
#include <klibloader.h>
#include <klocale.h>
#include <kabc/vcardconverter.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kconfig.h>

#include "kmobileview.h"
#include "kmobileitem.h"


KMobileView::KMobileView(TQWidget *parent, KConfig *_config)
    : DCOPObject("kmobileIface"), TQIconView(parent)
{
   m_config = _config;
   setSelectionMode(TQIconView::Single);
   setResizeMode(TQIconView::Adjust);
   setAutoArrange(true);
   connect(this, TQT_SIGNAL(doubleClicked(TQIconViewItem *)),
                 TQT_SLOT(slotDoubleClicked(TQIconViewItem *)));
}

KMobileView::~KMobileView()
{
}

bool KMobileView::addNewDevice(KConfig *config, KService::Ptr service)
{
   kdDebug() << "New mobile device item:\n";
   kdDebug() << TQString("LIBRARY: '%1', NAME: '%2', ICON: '%3', COMMENT: '%4'\n")
		.arg(service->library()).arg(service->name()).arg(service->icon())
		.arg(service->comment());

   KMobileItem *it;
   it = new KMobileItem(this, config, service);
   bool available = it->driverAvailable();
   it->configSave();
   it->writeKonquMimeFile();
   return available;
}

void KMobileView::saveAll()
{
   m_config->setGroup( "Main" );
   m_config->writeEntry( "Entries", count() );
   for ( TQIconViewItem *item = firstItem(); item; item = item->nextItem() ) {
	KMobileItem *it = static_cast<KMobileItem *>(item);
	it->driverAvailable();
	it->configSave();
	it->writeKonquMimeFile();
   }
   m_config->sync();
   emit signalChangeStatusbar( i18n("Configuration saved") );
}

void KMobileView::restoreAll()
{
   m_config->setGroup( "Main" );
   int num = m_config->readNumEntry( "Entries" );
   for (int i=0; i<num; ++i) {
	KMobileItem *it;
	it = new KMobileItem(this, m_config, i);
	it->driverAvailable();
	it->writeKonquMimeFile();
   }
   emit signalChangeStatusbar( i18n("Configuration restored") );
}

KMobileItem *KMobileView::findDevice( const TQString &deviceName ) const
{
   for ( TQIconViewItem *item = firstItem(); item; item = item->nextItem() ) {
	if (item->text() == deviceName)
		return static_cast<KMobileItem *>(item);
   }
   return 0L;
}

bool KMobileView::startKonqueror( const TQString &devName )
{
   KProcess *proc = new KProcess;
   *proc << "kfmclient" << "openProfile" << "webbrowsing" << "mobile:/"+devName;
   return proc->start();
}

void KMobileView::slotDoubleClicked( TQIconViewItem * item )
{
   startKonqueror(item->text());
}


/**
 * DCOP - Implementation
 */

#define MUTEX_LOCK(dev)    { dev->m_mutex.lock()
#define MUTEX_UNLOCK(dev)  dev->m_mutex.unlock(); }


TQStringList KMobileView::deviceNames()
{
   TQStringList names;
   for ( TQIconViewItem *item = firstItem(); item; item = item->nextItem() ) {
	names.append(item->text());
   }
   return names;
}

void KMobileView::removeDevice( TQString deviceName )
{
   delete findDevice(deviceName);
   emit signalChangeStatusbar( i18n("%1 removed").arg(deviceName) );
}

void KMobileView::configDevice( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return;
   MUTEX_LOCK(dev->m_dev);
   dev->m_dev->configDialog(this);
   MUTEX_UNLOCK(dev->m_dev);
}


bool KMobileView::connectDevice( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return false;
   bool connected;
   MUTEX_LOCK(dev->m_dev);
   connected = dev->m_dev->connectDevice();
   MUTEX_UNLOCK(dev->m_dev);
   emit signalChangeStatusbar(
	connected ? i18n("Connection to %1 established").arg(deviceName)
	          : i18n("Connection to %1 failed").arg(deviceName) );
   return connected;
}

bool KMobileView::disconnectDevice( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return true;
   bool disconnected;
   MUTEX_LOCK(dev->m_dev);
   disconnected = dev->m_dev->disconnectDevice();
   MUTEX_UNLOCK(dev->m_dev);
   emit signalChangeStatusbar(
	disconnected ? i18n("%1 disconnected").arg(deviceName)
	             : i18n("Disconnection of %1 failed").arg(deviceName) );
   return disconnected;
}

bool KMobileView::connected( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return false;
   bool conn;
   MUTEX_LOCK(dev->m_dev);
   conn = dev->m_dev->connected();
   MUTEX_UNLOCK(dev->m_dev);
   return conn;
}


TQString KMobileView::deviceClassName( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return TQString::null;
   TQString cn;
   MUTEX_LOCK(dev->m_dev);
   cn = dev->m_dev->deviceClassName();
   MUTEX_UNLOCK(dev->m_dev);
   return cn;
}

TQString KMobileView::deviceName( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return TQString::null;
   TQString dn;
   MUTEX_LOCK(dev->m_dev);
   dn = dev->m_dev->deviceName();
   MUTEX_UNLOCK(dev->m_dev);
   return dn;
}

TQString KMobileView::revision( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return TQString::null;
   TQString rev;
   MUTEX_LOCK(dev->m_dev);
   rev = dev->m_dev->revision();
   MUTEX_UNLOCK(dev->m_dev);
   return rev;
}

int KMobileView::classType( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return KMobileDevice::Unclassified;
   int ct;
   MUTEX_LOCK(dev->m_dev);
   ct = dev->m_dev->classType();
   MUTEX_UNLOCK(dev->m_dev);
   return ct;
}

int KMobileView::capabilities( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return KMobileDevice::hasNothing;
   int cap;
   MUTEX_LOCK(dev->m_dev);
   cap = dev->m_dev->capabilities();
   MUTEX_UNLOCK(dev->m_dev);
   return cap;
}

TQString KMobileView::nameForCap( TQString deviceName, int cap )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return TQString::null;
   TQString nc;
   MUTEX_LOCK(dev->m_dev);
   nc = dev->m_dev->nameForCap(cap);
   MUTEX_UNLOCK(dev->m_dev);
   return nc;
}

TQString KMobileView::iconFileName( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return TQString::null;
   TQString fn;
   MUTEX_LOCK(dev->m_dev);
   fn = dev->m_dev->iconFileName();
   MUTEX_UNLOCK(dev->m_dev);
   return fn;
}

int KMobileView::numAddresses( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return 0;
   int num;
   MUTEX_LOCK(dev->m_dev);
   num = dev->m_dev->numAddresses();
   MUTEX_UNLOCK(dev->m_dev);
   return num;
}

TQString KMobileView::readAddress( TQString deviceName, int index )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return TQString::null;

   int err;
   KABC::Addressee adr;
   MUTEX_LOCK(dev->m_dev);
   err = dev->m_dev->readAddress(index, adr);
   MUTEX_UNLOCK(dev->m_dev);
   if (err)
	return TQString::null;

   KABC::VCardConverter converter;
   TQString str = converter.createVCard(adr);
   if (str.isEmpty())
        return TQString::null;

   emit signalChangeStatusbar( i18n("Read addressbook entry %1 from %2")
		.arg(index).arg(deviceName) );

   return str;
}

bool KMobileView::storeAddress( TQString deviceName, int index, TQString vcard, bool append )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return false;

   KABC::VCardConverter converter;
   KABC::Addressee adr = converter.parseVCard(vcard);
   if (adr.isEmpty())
        return false;

   int err;
   MUTEX_LOCK(dev->m_dev);
   err = dev->m_dev->storeAddress(index, adr, append);
   MUTEX_UNLOCK(dev->m_dev);
   emit signalChangeStatusbar(
	err ? i18n("Storing contact %1 on %2 failed").arg(index).arg(deviceName)
	    : i18n("Contact %1 stored on %2").arg(index).arg(deviceName) );
   return (err == 0);
}

int KMobileView::numCalendarEntries( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return 0;
   int num;
   MUTEX_LOCK(dev->m_dev);
   num = dev->m_dev->numCalendarEntries();
   MUTEX_UNLOCK(dev->m_dev);
   return num;
}

int KMobileView::numNotes( TQString deviceName )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return 0;
   int num;
   MUTEX_LOCK(dev->m_dev);
   num = dev->m_dev->numNotes();
   MUTEX_UNLOCK(dev->m_dev);
   return num;
}

TQString KMobileView::readNote( TQString deviceName, int index )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return TQString::null;

   TQString note;
   int err;
   MUTEX_LOCK(dev->m_dev);
   err = dev->m_dev->readNote(index, note);
   MUTEX_UNLOCK(dev->m_dev);
   if (err)
	return TQString::null;
   emit signalChangeStatusbar( i18n("Read note %1 from %2")
		.arg(index).arg(deviceName) );
   return note;
}

bool KMobileView::storeNote( TQString deviceName, int index, TQString note )
{
   KMobileItem *dev = findDevice(deviceName);
   if (!dev || !dev->driverAvailable())
	return false;

   int err;
   MUTEX_LOCK(dev->m_dev);
   err = dev->m_dev->storeNote(index, note);
   MUTEX_UNLOCK(dev->m_dev);
   if (err)
	return false;
   emit signalChangeStatusbar( i18n("Stored note %1 to %2")
		.arg(index).arg(deviceName) );
   return true;
}



/*
 * DCOP Implementation for the devices:/ kioslave
 */

/*
 * returns the information for the given deviceName for usage in the
 * the devices kioslave. The TQStringList returned is comparable to the
 * format of /etc/fstab file. Please refer to the devices kioslave for
 * further information.
 * If deviceName is empty, this functions returns information for all
 * active mobile devices.
 * (function is only used by the devices kioslave - don't use elsewhere !)
 */
TQStringList KMobileView::kio_devices_deviceInfo(TQString deviceName)
{
   TQStringList mobiles = deviceNames();
   if (mobiles.count() == 0)
	return mobiles;

   TQStringList mountList;
   for ( TQStringList::Iterator it = mobiles.begin(); it != mobiles.end(); ++it ) {
	TQString name = *it;

	if (deviceName.isEmpty())
		mountList << name;
	else
		if (deviceName!=name)
			continue;

	KMobileItem *dev = findDevice(name);
	TQString mime = dev ? dev->getKonquMimeType() : KMOBILE_MIMETYPE_DEVICE;

	mountList << name;
	mountList << " ";
	mountList << TQString("mobile:/%1").arg(name); // KIO::encodeFileName()
	mountList << mime;
	mountList << "true"; // mountState
	mountList << "---";
	if (!deviceName.isEmpty())
		break;
   }
   return mountList;
}


#include "kmobileview.moc"
