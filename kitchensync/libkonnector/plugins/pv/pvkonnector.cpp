/***************************************************************************
                          pvconnector.cpp  -  description
                             -------------------
    begin                : Wed Sep 18 2002
    copyright            : (C) 2002 by Maurus Erni
    email                : erni@pocketviewer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qvaluelist.h>
#include <qpair.h>
#include <qptrlist.h>
#include <qhostaddress.h>
#include <kapabilities.h>
#include <koperations.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>

#include "casiopvlink.h"
#include "pvkonnector.h"

typedef KGenericFactory<KSync::PVPlugin, QObject>  PVKonnectorPlugin;
K_EXPORT_COMPONENT_FACTORY(libpvkonnector, PVKonnectorPlugin);

using namespace KSync;

PVPlugin::PVPlugin(QObject *obj, const char *name, const QStringList)
  : KonnectorPlugin(obj, name)
{
  kdDebug(5202) << "PVPlugin constructor";
  casioPVLink = new CasioPVLink(this, "CasioPVLink");

  // Connect signals from casioPVLink with pvkonnector
  connect(casioPVLink, SIGNAL(sync(Syncee::PtrList)),
    this, SLOT(slotSync(Syncee::PtrList)));

  connect(casioPVLink, SIGNAL(errorKonnector(int, QString)),
    this, SLOT(slotErrorKonnector(int, QString)));

  connect(casioPVLink, SIGNAL(stateChanged(bool)),
    this, SLOT(slotChanged(bool)));
}

PVPlugin::~PVPlugin()
{
    kdDebug(5202) << "PVPlugin destructor";
  delete casioPVLink;
}
void PVPlugin::setUDI(const QString &udi)
{
  m_udi = udi;
}

QString PVPlugin::udi()const
{
  return m_udi;
}

QIconSet PVPlugin::iconSet()const
{
    kdDebug(5205) << "iconSet" << endl;
    QPixmap logo;
    logo.load( locate("appdata",  "pics/pv_logo.png" ) );
    return QIconSet( logo );
}
QString PVPlugin::iconName()const
{
    kdDebug(5205) << "icon Name from PV" << endl;
    return QString::fromLatin1("pics/pv_logo.png");
};
Kapabilities PVPlugin::capabilities( )
{
  // create the capabilities Apply
  kdDebug(5202) << "PVPlugin capabilities" << endl;
  Kapabilities caps;
  caps.setSupportMetaSyncing(false);
  caps.setSupportsPushSync(true);
  caps.setNeedsConnection(false);
  caps.setSupportsListDir(false);
  caps.setNeedsIPs(false);
  caps.setNeedsSrcIP(false);
  caps.setNeedsDestIP(false);
  caps.setAutoHandle(false);
  caps.setNeedAuthentication(false);
  // Set possible models
  QStringList models;
  models.append("PV-450");
  models.append("PV-S660");
  models.append("PV-750 Plus");
  caps.setModels(models);
  // Set possible connections
  QStringList connections;
  connections.append("/dev/ttyS0");
  connections.append("/dev/ttyS1");
  caps.setConnectionMode(connections);

  return caps;
}

void PVPlugin::setCapabilities(const Kapabilities &kaps)
{
  // Set important kapabilities for syncing
  kdDebug(5205) << "PVPlugin setCapabilities" << endl;
  casioPVLink->setModel(kaps.currentModel());
  casioPVLink->setConnectionMode(kaps.currentConnectionMode());
  casioPVLink->setMetaSyncing(kaps.isMetaSyncingEnabled());
}

bool PVPlugin::startSync()
{
  kdDebug(5205) << "start Sync PVPlugin" << endl;
  return casioPVLink->startSync();
}

bool PVPlugin::startBackup(const QString& path)
{
  kdDebug(5205) << "start Backup PVPlugin" << endl;
  return casioPVLink->startBackup(path);
}

bool PVPlugin::startRestore(const QString& path)
{
  kdDebug(5205) << "start Restore PVPlugin" << endl;
  return casioPVLink->startRestore(path);
}

bool PVPlugin::isConnected()
{
  return casioPVLink->isConnected();
}

bool PVPlugin::insertFile(const QString &fileName )
{
  // Not used yet
  return false;
}

QByteArray PVPlugin::retrFile(const QString &path )
{
  // Not used yet
  return 0;
}

void PVPlugin::slotWrite(const QString &path, const QByteArray &array )
{
  // Not used yet
}

void PVPlugin::slotWrite(Syncee::PtrList entry)
{
  casioPVLink->write(entry);
};

void PVPlugin::slotWrite(KOperations::ValueList operations )
{
  // Not used yet
}

Syncee* PVPlugin::retrEntry( const QString& path )
{
  return 0;// Not used yet
}

// Public slots for signals from casioPVLink
void PVPlugin::slotSync(Syncee::PtrList entry )
{
  emit sync(m_udi, entry);
}

void PVPlugin::slotChanged( bool state)
{
  kdDebug(5202) << "slotChanged PVkonnector" << endl;
  emit stateChanged(m_udi, state);
}

void PVPlugin::slotErrorKonnector(int mode, QString error)
{
  kdDebug(5205) << "slotError PVKonnector" << endl;
  emit errorKonnector(m_udi, mode, error);
}

QString PVPlugin::metaId()const
{
  return casioPVLink->metaId();
}

#include "pvkonnector.moc"
