/***************************************************************************
                        casiopvlink.cpp  -  description
                             -------------------
    begin                : Sat Sept 21 2002
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

// KDE includes
#include <kapplication.h>
#include <koperations.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kfileitem.h>
#include <kurl.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <qregexp.h>
#include <qfile.h>
#include <qdir.h>
#include <qcstring.h>
#include <dcopclient.h>

// KitchenSync includes
#include <idhelper.h>

// Project includes
#include "helper.h"

#include "casiopvlink.h"

using namespace KSync;
using namespace PVHelper;

CasioPVLink::CasioPVLink(QObject *obj, const char *name)
    : QObject(obj, name), DCOPObject("CasioPVLinkIface")
{
  kdDebug() << "CasioPVLink constructor" << endl;

  // Init variables
  m_state = DISCONNECTED;
  m_mode = NONE;
  m_firstSync = true; // xxx remove me -> has to be done depending on optional code
}

CasioPVLink::~CasioPVLink()
{
  kdDebug() << "CasioPVLink destructor" << endl;
}

bool CasioPVLink::startSync()
{
  kdDebug() << "startSync CasioPVLink" << endl;
  if (connectPV())
  {
    m_mode = SYNC;
    // Prepare DCOP send to PVDaemon
    QByteArray data2;
    QDataStream dataStream2(data2, IO_WriteOnly);
    QStringList strList;
    strList << "Contact Business";
    dataStream2 << strList;
    if (m_firstSync)
    {
      kdDebug() << "DCOP send getAllEntries()" << endl;
      if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                     "getAllEntries(QStringList)", data2))
      {
        emit errorKonnector(1, "Error with DCOP!");
        kdDebug() << "DCOP send failed" << endl;
        return false;
      }
    }
    else
    {
      kdDebug() << "DCOP send getChanges()" << endl;
      if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                     "getChanges(QStringList)", data2))
      {
        emit errorKonnector(1, "Error with DCOP!");
        kdDebug() << "DCOP send failed" << endl;
        return false;
      }
    }
  }  // end if
  return false;
}

bool CasioPVLink::startBackup(const QString& path)
{
  kdDebug() << "startBackup CasioPVLink" << endl;

  // xxx check if first sync depending on optional code
  if (connectPV())
  {
    m_mode = BACKUP;
    m_path = path;
    // Prepare DCOP send to PVDaemon
    QByteArray data2;
    QDataStream dataStream2(data2, IO_WriteOnly);
    QStringList strList;
    strList << "Contact Business";
    dataStream2 << strList;
    kdDebug() << "DCOP send getAllEntries()" << endl;
    if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                   "getAllEntries(QStringList)", data2))
    {
      emit errorKonnector(1, "Error with DCOP!");
      kdDebug() << "DCOP send failed" << endl;
      return false;
    }
    return true;
  }  // end if
  return false;
}

// restore data on pv
bool CasioPVLink::startRestore(const QString& path)
{
  kdDebug() << "CsaioPVLink::restore" << endl;
  if (connectPV())
  {
    // Read backup file from HD
    QByteArray array;
    QFile file(path);
    if (file.open(IO_ReadOnly))
    {
      kdDebug() << "CsaioPVLink::restore backup file found!" << endl;
      array = file.readAll();
      file.close();
      // Prepare DCOP send to PVDaemon
      QByteArray data2;
      QDataStream dataStream2(data2, IO_WriteOnly);
      QStringList strList;
      strList << "Contact Business";
      dataStream2 << array;

      kdDebug() << "DCOP send setAllEntries()" << endl;
      if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                     "setAllEntries(QByteArray)", data2))
      {
        emit errorKonnector(1, "Error with DCOP!");
        kdDebug() << "DCOP send failed" << endl;
        return false;
      }
    }
    else
    {
      emit errorKonnector(1, "File not found!"); // xxx bessere fehlermeldung
    }
  }
  return false;
}

void CasioPVLink::setModel(const QString& model)
{
  m_model = model;
}

void CasioPVLink::setConnectionMode(const QString& connectionMode)
{
  m_connectionMode = connectionMode;
}

void CasioPVLink::setMetaSyncing(const bool meta)
{
  m_meta = meta;
}

bool CasioPVLink::isConnected()
{
  if(m_state == CONNECTED)
  {
    return true;
  }
  else
  {
    return false;
  }
}

Syncee* CasioPVLink::retrEntry( const QString& path )
{
  // not used yet
  return 0l;
}
QByteArray CasioPVLink::retrFile(const QString &path )
{
  // not used yet
  return 0l;
}
bool CasioPVLink::insertFile( const QString &fileName )
{
  // not used yet
  return true;
}

void CasioPVLink::write(const QString &path, const QByteArray &array )
{
  // not used yet
}

// write back to my PV
void CasioPVLink::write(Syncee::PtrList lis)
{
  kdDebug() << "write back to PV lis count =" << lis.count() << endl;

  Syncee* syncee;
  // Convert synchronized data to XML
  for (syncee = lis.first(); syncee != 0; syncee = lis.next())
  {
    if (syncee->type() == QString::fromLatin1("AddressBookSyncee"))
    {
      // We just need the added entries
      QPtrList<KSync::SyncEntry> changed = syncee->added();

      kdDebug() << "Added entries count: " << changed.count();
    }
    else if (syncee->type() == QString::fromLatin1("EventSyncee"))
    {
    }
    if (syncee->type() == QString::fromLatin1("TodoSyncee"))
    {
    }
  }


  QByteArray array = Helper::Syncee2XML(&lis);

  // Prepare DCOP send to PVDaemon
  QByteArray data2;
  QDataStream dataStream2(data2, IO_WriteOnly);
  QStringList strList;
  strList << "Contact Business";
  dataStream2 << array;

  kdDebug() << "DCOP send setChanges()" << endl;
  if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                 "setChanges(QByteArray)", data2))
  {
    kdDebug() << "DCOP send failed" << endl;
  }

// xxx for what?  d->isSyncing = false; // do it in the write back later on*/
}

void CasioPVLink::write(KOperations::ValueList )
{
  // not used yet
}

QString CasioPVLink::metaId() const
{
  return m_modelCode;
}

Syncee* retrEntry( const QString& )
{
  return 0l;
}

bool CasioPVLink::connectPV()
{
  kdDebug(5202) << "connectPV()" << endl;

  if( m_state == SYNCING )
  {
    kdDebug() << "isSyncing" << endl;
    return false;
  }
  else if (m_state == DISCONNECTED)
  {
    // Prepare DCOP call to connectPV()
    QByteArray data, replyData;
    QCString replyType;
    QDataStream stream(data, IO_WriteOnly);
    stream << m_connectionMode;

    // Call connectPV()
    if (kapp->dcopClient()->call("pvDaemon", "PVDaemonIface", "connectPV(QString)", data, replyType, replyData ) )
    {
      kdDebug() << "Reply Type: " << replyType << endl;
      QDataStream answer(replyData, IO_ReadOnly);
      // Store model data of connected PV -> xxx not nice but it works; data should be passed different!
      QStringList strList;
      answer >> strList;
      QStringList::Iterator it = strList.begin();
      m_modelCode = (*it); ++it;
      m_optionalCode = (*it); ++it;
      if (!(it == strList.end()))
      {
        m_secretArea = true;
      }
      else
      {
        m_secretArea = false;
      }
      m_state = CONNECTED;
      emit stateChanged(true);
      kdDebug() << "CasioLVLink:: emit stateChanged" << endl;
    }
    else
    {
      emit errorKonnector(1, "Error with DCOP! Connection could not be established");
      kdDebug() << "Error with DCOP" << endl;
      return false;
    }
  }  // if m_state == DISCONNECTED
  return true;
}

// -------------------- Public DCOP interface -------------------- //
void CasioPVLink::getChangesDone(const QByteArray& array)
{
  kdDebug() << "CasioPVLink::getChangesDone() received" << endl;

  // Convert received data to Syncee::PtrList
  Syncee::PtrList synceePtrListNew = Helper::XML2Syncee(array);

  if (m_meta)
  {
    Syncee::PtrList synceePtrListOld;
    QFile file(QDir::homeDirPath() + "/.kitchensync/meta/" + m_optionalCode + "/pvdataentries.xml");
    if (file.open(IO_ReadOnly))
    {
      QByteArray array = file.readAll();
      file.close();
      synceePtrListOld = Helper::XML2Syncee(array);
      synceePtrListNew = Helper::doMeta(&synceePtrListNew, &synceePtrListOld);
    }
    else
    {
      // xxx error handling!!
    }
// xxx set sync mode for each syncee, not for the whole PtrList    synceePtrListNew.setSyncMode(KSync::Syncee::MetaMode);
  }

  emit sync(synceePtrListNew);
}

void CasioPVLink::getAllEntriesDone(const QByteArray& array)
{
  kdDebug() << "CasioPVLink::getAllEntriesDone() received" << endl;

  // Convert received data to Syncee::PtrList
  Syncee::PtrList synceePtrList = Helper::XML2Syncee(array);
  Syncee* syncee;
  for (syncee = synceePtrList.first(); syncee != 0; syncee = synceePtrList.next())
  {
    syncee->setFirstSync(m_firstSync);
  }

  QDir dir;
  dir.mkdir(QDir::homeDirPath() + "/.kitchensync");
  dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta");
  dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta/" + m_optionalCode );

  if (m_mode == SYNC)
  {
    if (m_firstSync/* && m_meta*/)
    {
      emit sync(synceePtrList);
    }
  }
  else if (m_mode == BACKUP)
  {
     kdDebug() << "CasioPVLink::getAllEntriesDone() backup_mode" << endl;
    // Store received data from PV to a file
    QFile file(m_path);
//    QFile file(QDir::homeDirPath() + "/.kitchensync/meta/" + m_optionalCode + "/pvbackup.xml");
    if (file.open(IO_WriteOnly))
    {
      file.writeBlock(array);
      file.close();
    }
    // Prepare DCOP call to disconnectPV()
    QCString replyType;
    QByteArray data, replyData;
    QDataStream stream(data, IO_WriteOnly);
    // Call disconnectPV()
    if (kapp->dcopClient()->call("pvDaemon", "PVDaemonIface", "disconnectPV()", data, replyType, replyData))
    {
      QDataStream answer(replyData, IO_ReadOnly);
      bool ok;
      answer >> ok;
      if (ok)
      {
        m_state = DISCONNECTED;
        emit stateChanged(false);
      }
      else
      {
        emit errorKonnector(1, "Connection could not be released!");
      }
    }
  }
}

void CasioPVLink::setChangesDone(const bool ok)
{
  kdDebug() << "CasioPVLink::setChangesDone() received" << endl;
  if (ok)
  {
    // Prepare DCOP call to disconnectPV()
    QCString replyType;
    QByteArray data, replyData;
    QDataStream stream(data, IO_WriteOnly);
    // Call disconnectPV()
    if (kapp->dcopClient()->call("pvDaemon", "PVDaemonIface", "disconnectPV()", data, replyType, replyData))
    {
      QDataStream answer(replyData, IO_ReadOnly);
      bool ok;
      answer >> ok;
      if (ok)
      {
        m_state = DISCONNECTED;
        emit stateChanged(false);
      }
      else
      {
        emit errorKonnector(1, "Connection could not be released!");
      }
    }
  }
}

void CasioPVLink::setAllEntriesDone(const bool ok)
{
  kdDebug() << "CasioPVLink::setAllEntriesDone() received" << endl;
  if (ok)
  {
    // Prepare DCOP call to disconnectPV()
    QCString replyType;
    QByteArray data, replyData;
    QDataStream stream(data, IO_WriteOnly);
    // Call disconnectPV()
    if (kapp->dcopClient()->call("pvDaemon", "PVDaemonIface", "disconnectPV()", data, replyType, replyData))
    {
      QDataStream answer(replyData, IO_ReadOnly);
      bool ok;
      answer >> ok;
      if (ok)
      {
        m_state = DISCONNECTED;
        emit stateChanged(false);
      }
      else
      {
        emit errorKonnector(1, "Connection could not be released!");
      }
    }
  }
}

void CasioPVLink::errorPV(const QString& msg, const unsigned int errorcode)
{
  kdDebug() << "CasioPVLink::errorPV::Error received" << endl;
  emit errorKonnector(errorcode, msg);
}

#include "casiopvlink.moc"
