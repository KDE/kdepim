/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#include <qobject.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "kmobileitem.h"


#define PRINT_DEBUG kdDebug() << "KMobileItem: "

KMobileItem::KMobileItem(QIconView *parent, KConfig *_config, KService::Ptr service)
	: QObject(parent), QIconViewItem(parent), m_dev(0L)
{
   config = _config;

   Q_CHECK_PTR(service);
   if (service) {
	setText(service->name());
	m_deviceDesktopFile = service->desktopEntryName();
	m_deviceConfigFile = QString("kmobile_%1_rc").arg(text());
	m_deviceConfigFile = m_deviceConfigFile.replace(' ', "");
	m_iconName = service->icon();
   };

   if (m_iconName.isEmpty())
        m_iconName = KMOBILE_ICON_UNKNOWN;

   setPixmap(getIcon());
   setRenameEnabled(true);
}

/* restore this item from the config file */
KMobileItem::KMobileItem(QIconView *parent, KConfig *_config, int reload_index)
	: QObject(parent), QIconViewItem(parent), m_dev(0L)
{
   config = _config;

   if (!configLoad(reload_index)) {
	delete this;
	return;
   }

   setPixmap(getIcon());
   setRenameEnabled(true);
}

KMobileItem::~KMobileItem()
{
   delete m_dev;
}


void KMobileItem::configSave() const
{
   config->setGroup( config_SectionName() );
   config->writeEntry( "Name", text() );
   config->writeEntry( "Config", m_deviceConfigFile );
   config->writeEntry( "DesktopFile", m_deviceDesktopFile );
   config->writeEntry( "IconName", m_iconName );
   config->sync();

   writeKonquMimeFile();
}

bool KMobileItem::configLoad(int idx)
{
   config->setGroup( config_SectionName(idx) );
   setText( config->readEntry("Name") );
   m_deviceConfigFile	= config->readEntry( "Config" );
   m_deviceDesktopFile	= config->readEntry( "DesktopFile" );
   m_iconName		= config->readEntry( "IconName" );

   if (text().isEmpty() || m_deviceConfigFile.isEmpty() || 
	m_deviceDesktopFile.isEmpty() || m_iconName.isEmpty() )
	return false;

   return true;
}

QPixmap KMobileItem::getIcon() const
{
   return KGlobal::instance()->iconLoader()->loadIcon(m_iconName, KIcon::Desktop );
}

QString KMobileItem::config_SectionName(int idx) const
{
   if (idx == -1) idx = index();
   return QString("MobileDevice_%1").arg(idx);
}

/* this MimeType is used by konqueror */
QString KMobileItem::getKonquMimeType() const
{
   return KMOBILE_MIMETYPE_DEVICE_KONQUEROR(text());
}

/* provide MimeType for konqueror */
void KMobileItem::writeKonquMimeFile() const
{
   // strip path and file extension of icon name
   QString icon = m_iconName;
   int p = icon.findRev('/');
   if (p>=0) icon = icon.mid(p+1);
   p = icon.find('.');
   if (p>=0) icon = icon.left(p);

   KConfig conf( getKonquMimeType()+".desktop", false, true, "mime" );
   conf.setDesktopGroup();
   conf.writeEntry("Encoding", "UTF-8");
   conf.writeEntry("Comment", "Kommentar");
   conf.writeEntry("Type", "MimeType");
   conf.writeEntry("Icon", icon );
   conf.writeEntry("MimeType", getKonquMimeType());
   conf.writeEntry("Patterns", "" );
   conf.sync();
}


/*
 * get a list of all services providing a libkmobile device driver
 */
KTrader::OfferList KMobileItem::getMobileDevicesList()
{ 
  KTrader::OfferList offers = KTrader::self()->query(KMOBILE_MIMETYPE_DEVICE);
  return offers;
}


KService::Ptr KMobileItem::getServicePtr() const
{
  KTrader::OfferList list = getMobileDevicesList();
  KTrader::OfferListIterator it;
  KService::Ptr ptr;
  for ( it = list.begin(); it != list.end(); ++it ) {
    KService::Ptr ptr = *it;
    if (ptr->desktopEntryName() == m_deviceDesktopFile)
 	return ptr;
  }
  PRINT_DEBUG << QString("Service for library '%1' not found in KService list\n")
			.arg(m_deviceDesktopFile);
  return 0L;
}

/*
 * loads & initializes the device and returns a pointer to it.
 */
bool KMobileItem::driverAvailable()
{
   if (m_dev)
	return true;

   KService::Ptr ptr = getServicePtr();
   if (!ptr)
	return false;

   PRINT_DEBUG << QString("Loading library %1\n").arg(ptr->library());
   KLibFactory *factory = KLibLoader::self()->factory( ptr->library().utf8() );
   if (!factory)
	return false;

   m_dev = static_cast<KMobileDevice *>(factory->create(this, ptr->name().utf8(),
				"KMobileDevice", QStringList(m_deviceConfigFile)));
   PRINT_DEBUG << QString("Got KMobileDevice object at 0x%1, configfile=%2\n")
			.arg((unsigned long)m_dev, 0, 16).arg(m_deviceConfigFile);

   return (m_dev != 0);
}

#include "kmobileitem.moc"

