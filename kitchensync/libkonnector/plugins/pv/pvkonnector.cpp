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

/** Constructor
  *
  * @param obj The Parent QObject
  * @param name The name of this instance
  */
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

/** Destructor
  *
  */
PVPlugin::~PVPlugin()
{
    kdDebug(5202) << "PVPlugin destructor";
  delete casioPVLink;
}

/**
  * Sets the uid of the konnector. This uid is given at loading of
  * the plugin.
  * @param uid The uid of the konnector
  */ 
void PVPlugin::setUDI(const QString &udi)
{
  m_udi = udi;
}

/**
  * Returns the uid of the konnector.
  * @return QString The uid of the konnector
  */  
QString PVPlugin::udi()const
{
  return m_udi;
}

/**
  * Returns the icon of the Konnector. Not used yet!
  * @return QIconSet The icon of the Konnector
  */ 
QIconSet PVPlugin::iconSet()const
{
    kdDebug(5205) << "iconSet" << endl;
    QPixmap logo;
    logo.load( locate("appdata",  "pics/pv_logo.png" ) );
    return QIconSet( logo );
}

/**
  * Returns the icon name of the Konnector. Not used yet!
  * @return QString The icon name of the Konnector
  */  
QString PVPlugin::iconName()const
{
    kdDebug(5205) << "icon Name from PV" << endl;
    return QString::fromLatin1("pics/pv_logo.png");
};

/**
  * Returns the kapabilities of the konnector.
  * @return Kapabilities The kapabilities of the konnector
  */   
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

/**
  * Sets the kapabilities of the konnector depending on the
  * configuration.
  * @param uid The uid of the konnector
  */ 
void PVPlugin::setCapabilities(const Kapabilities &kaps)
{
  // Set important kapabilities for syncing
  kdDebug(5205) << "PVPlugin setCapabilities" << endl;
  casioPVLink->setModel(kaps.currentModel());
  casioPVLink->setConnectionMode(kaps.currentConnectionMode());
  casioPVLink->setMetaSyncing(kaps.isMetaSyncingEnabled());
}

/**
  * Starts the synchronization procedure of the Plugin
  * @return bool Starting of synchronization successful (yes / no)
  */
bool PVPlugin::startSync()
{
  kdDebug(5205) << "start Sync PVPlugin" << endl;
  return casioPVLink->startSync();
}

/**
  * Starts the backup procedure of the Plugin.
  * @param path The path of the backup file
  * @return bool Starting of backup successful (yes / no)
  */
bool PVPlugin::startBackup(const QString& path)
{
  kdDebug(5205) << "start Backup PVPlugin" << endl;
  return casioPVLink->startBackup(path);
}

/**
  * Starts the restore procedure of the Plugin.
  * @param path The path of the restore file
  * @return bool Starting of restore successful (yes / no)
  */
bool PVPlugin::startRestore(const QString& path)
{
  kdDebug(5205) << "start Restore PVPlugin" << endl;
  return casioPVLink->startRestore(path);
}
 
/**
  * Returns whether the PV is connected.
  * @return bool PV connected (yes / no)
  */  
bool PVPlugin::isConnected()
{
  return casioPVLink->isConnected();
}

/**
  * Filedownload to the PV. Not used yet!
  * @param filename The path of the file to be donwloaded
  * @return bool File download successful (yes / no)
  */  
bool PVPlugin::insertFile(const QString &fileName )
{
  // Not used yet
  return false;
}

/**
  * Fileupload from the PV. Not used yet!
  * @param filename The path of the file to be donwloaded
  * @return QByteArray The requested file as a QByteArray
  */ 
QByteArray PVPlugin::retrFile(const QString &path )
{
  // Not used yet
  return 0;
}

/**
  * This will write the QByteArray to the PV. Not used yet!
  * @param dest The destination of the array
  * @param array The array
  */
void PVPlugin::slotWrite(const QString &path, const QByteArray &array )
{
  // Not used yet
}

/**
  * This will write a List of Syncee to the PV. Is called from
  * KitchenSync after synchronization.
  * @param lis The list of Syncee
  */
void PVPlugin::slotWrite(Syncee::PtrList entry)
{
  casioPVLink->write(entry);
};

/**
  * This will do the KOperations. Not used yet!
  * @param ops Operations like delete
  */
void PVPlugin::slotWrite(KOperations::ValueList operations )
{
  // Not used yet
}

/**
  * Getting of a file from the PV returned as Syncee*.
  * @param path The path of the file to be donwloaded
  * @return Syncee* The requested file as a Syncee*
  */
Syncee* PVPlugin::retrEntry( const QString& path )
{
  return 0;// Not used yet
}


/**
  * Will be called from the PV Library if all data was fetched from the PV
  * @param lis The data of the PV as a Syncee::PtrList
  */
void PVPlugin::slotSync(Syncee::PtrList entry )
{
  emit sync(m_udi, entry);
}

/**
  * Will be called from the PV Library when the state of the connected PV
  * has changed.
  * @param state The state of the PV (connected / disconnected)
  */ 
void PVPlugin::slotChanged( bool state)
{
  kdDebug(5202) << "slotChanged PVkonnector" << endl;
  emit stateChanged(m_udi, state);
}

/**
  * Will be called from the PV Library when the connected PV reported an
  * error
  * @param number The error number
  * @param msg The error message
  */
void PVPlugin::slotErrorKonnector(int mode, QString error)
{
  kdDebug(5205) << "slotError PVKonnector" << endl;
  emit errorKonnector(m_udi, mode, error);
}

/**
  * Returns the Id of the connected PV.
  * @return QString The Id of the connected PV
  */  
QString PVPlugin::metaId()const
{
  return casioPVLink->metaId();
}

#include "pvkonnector.moc"
