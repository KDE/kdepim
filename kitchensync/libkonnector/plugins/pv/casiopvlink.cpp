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
#include "addressbook.h"
#include "event.h"
#include "todo.h"

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
  // m_first sync is set to false, if connected PV was already synchronized 
  m_firstSync = true;
  // set categories to be synchronized
}

CasioPVLink::~CasioPVLink()
{
  kdDebug() << "CasioPVLink destructor" << endl;
}

bool CasioPVLink::startSync()
{
  kdDebug() << "startSync CasioPVLink" << endl;
  if (callConnectPV())
  {
    m_mode = SYNC;
    // Prepare DCOP send to PVDaemon
    QByteArray data;
    QDataStream dataStream(data, IO_WriteOnly);
    // Set categories to be synchronized
    QStringList strList;
    strList << "Contact Business" << "Contact Private" << "Contact Untitled 1"
             << "Contact Untitled 2" << "Contact Untitled 3";    
    // Additional categories on PV-750
    if (m_model == "PV-750 Plus")
    {
      strList << "Contact Untitled 4" << "Contact Untitled 5";
    }
    strList << "To Do";
    dataStream << strList;
    if (m_firstSync)
    {
      kdDebug() << "DCOP send getAllEntries()" << endl;
      if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                     "getAllEntries(QStringList)", data))
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
                                     "getChanges(QStringList)", data))
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

  if (callConnectPV())
  {
    m_mode = BACKUP;
    m_backupPath = path;
    // Prepare DCOP send to PVDaemon
    QByteArray data;
    QDataStream dataStream(data, IO_WriteOnly);    
    // Set categories to be synchronized
    QStringList strList;
    strList << "Contact Business" << "Contact Private" << "Contact Untitled 1"
             << "Contact Untitled 2" << "Contact Untitled 3";
    // Additional categories on PV-750
    if (m_model == "PV-750 Plus")
    {
      strList << "Contact Untitled 4" << "Contact Untitled 5";
    }
    strList << "To Do";
    dataStream << strList;
    kdDebug() << "DCOP send getAllEntries()" << endl;
    if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                   "getAllEntries(QStringList)", data))
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
  if (callConnectPV())
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
      QByteArray data;
      QDataStream dataStream(data, IO_WriteOnly);
      dataStream << array;

      kdDebug() << "DCOP send setAllEntries()" << endl;
      if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                     "setAllEntries(QByteArray)", data))
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
  kdDebug() << "write back to PV" << endl;
  
  // Convert synchronized data to XML
  //  ok lets write back the changes to the Konnector
  QByteArray array;
  QBuffer buffer(array);
  if (buffer.open( IO_WriteOnly))
  {
    if (!lis.isEmpty())
    {
      Syncee* syncee;
      QTextStream stream( &buffer );
      stream.setEncoding( QTextStream::UnicodeUTF8 );
      stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE pvdataentries>" << endl
              << "<pvdataentries>" << endl;
      // Go through all categories
      for (syncee = lis.first(); syncee != 0; syncee = lis.next())
      {
        kdDebug() << syncee->type() << " found!" << endl;      
        if (syncee->type() == QString::fromLatin1("AddressBookSyncee"))
        {
          stream << (AddressBook::toXML(dynamic_cast<AddressBookSyncee*>(syncee)));
        }
        else if (syncee->type() == QString::fromLatin1("EventSyncee"))
        {
          stream << (Event::toXML(dynamic_cast<EventSyncee*>(syncee)));
        }
        if (syncee->type() == QString::fromLatin1("TodoSyncee"))
        {
          stream << (Todo::toXML(dynamic_cast<TodoSyncee*>(syncee)));
        }
      }
      stream << "</pvdataentries>" << endl;
    }  // end if

    /* xxx not used yet. first meta sync has to be implemented!    
    // Store data to be written to HD (for meta syncing) after writing to PV
    m_array = array; */

    kdDebug() << "dumping data: " << array.data() << endl;
      
    // Prepare DCOP send to PVDaemon
    QByteArray data;
    QDataStream dataStream(data, IO_WriteOnly);
    dataStream << m_optionalCode << array;

    kdDebug() << "DCOP send setChanges()" << endl;
    if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                   "setChanges(QString, QByteArray)", data))
    {
      kdDebug() << "DCOP send failed" << endl;
    }
  }
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

// ----------------------- Private Methods ----------------------- //
bool CasioPVLink::callConnectPV()
{
  kdDebug(5202) << "connectPV()" << endl;

  if (m_state == CONNECTED)
  {
    kdDebug() << "Is already syncing!!" << endl;
    emit errorKonnector(10000/*xxx Fehlernummer*/,
          "Device is already connected!\nPlease wait until running process is finished.");
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
      kdDebug() << "Model Code: " << m_modelCode << endl;
      m_optionalCode = (*it); ++it;         
      if (!(it == strList.end()))
      {
        m_secretArea = true;
      }
      else
      {
        m_secretArea = false;
      }
      // xxx Check if the correct PV model was connected
      
      // Check if the connected PV was already synchronized before.
      /* xxx not used yet. first meta sync has to be implemented!
      if (m_meta)
      {
        m_metaPath = QString(QDir::homeDirPath() + "/.kitchensync/meta/" 
                              + m_optionalCode.right(12) + "/pvmeta.xml");
        kdDebug() << "Path: " << m_metaPath << " Code: " << m_optionalCode << endl;
        // Optional code starts correct (maybe PV was already synced)
        if ((m_optionalCode.left(11) == "KitchenSync") && (QFile::exists(m_metaPath)))
        {
          kdDebug() << "Was already synching with this one!" << endl;
          m_firstSync = false;
        }
        else
        {
          kdDebug() << "Never synched with this one before! Optional Code: " << m_optionalCode << endl;
          // Generate a new optional Code and store the new path
          m_optionalCode = "KitchenSync-" + kapp->randomString(13);
          m_metaPath = QString(QDir::homeDirPath() + "/.kitchensync/meta/"
                               + m_optionalCode.right(12) + "/pvmeta.xml");
          // Create dir to store meta data
          QDir dir;
          dir.mkdir(QDir::homeDirPath() + "/.kitchensync");
          dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta");
          dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta/" + m_optionalCode.right(12));
        }
      } */
      m_state = CONNECTED;
      emit stateChanged(true);
      kdDebug() << "CasioLVLink:: emit stateChanged" << endl;
    }
    else
    {
      emit errorKonnector(10000, "Error with DCOP!\nIs pvdaemon running?");
      kdDebug() << "Error with DCOP" << endl;
      return false;
    }
  }  // if m_state == DISCONNECTED
  return true;
}

void CasioPVLink::callDisconnectPV()
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
      emit errorKonnector(10000/*xxx Fehlernummer*/, "Connection could not be released!");
    }
  }
}

// -------------------- Public DCOP interface -------------------- //
void CasioPVLink::getChangesDone(const QByteArray& array)
{
  kdDebug() << "CasioPVLink::getChangesDone() received" << endl;
    
  // Convert received data to Syncee::PtrList
 
  Syncee::PtrList lis;
  QDomDocument doc("mydocument");
  // Set content of DOM document
  if (doc.setContent(array))
  {
    // Parse DOM Document
    QDomElement docElem = doc.documentElement();
    if (docElem.tagName() == QString::fromLatin1("pvdataentries"))
    {
      kdDebug() << "pvdataentries found!" << endl;
      QDomNode n =  docElem.firstChild(); // child of pvdataentries -> type of entries (e.g. contacts)
      while(!n.isNull())
      {
        QDomElement e = n.toElement();      
        kdDebug() << e.tagName() << " found!" << endl;
        QDomNode n = e.firstChild();        
        if (e.tagName() == QString::fromLatin1("contacts"))
        {
          AddressBookSyncee* syncee = new AddressBookSyncee();
          syncee = AddressBook::toAddressBookSyncee(n);
          lis.append(syncee);
        }
        else if (e.tagName() == QString::fromLatin1("events"))
        {
          EventSyncee* syncee = new EventSyncee();
          syncee = Event::toEventSyncee(n);
          lis.append(syncee);
        }
        else if (e.tagName() == QString::fromLatin1("todos"))
        {
          TodoSyncee* syncee = new TodoSyncee();
          syncee = Todo::toTodoSyncee(n);
          lis.append(syncee);
        }
        n = n.nextSibling(); // jump to next element
      }  
    }  // end of if pvdataentries
    else
    {
      kdDebug() << "PVHelper::XML2Syncee -> pvdataentries not found" << endl;
      // xxx fehlermeldung!!! syncee auf null setzen? wie geht das?
    }
  }  // end of if doc.setContents()
  else
  {
    kdDebug() << "PVHelper::XML2Syncee !doc.setContent() " << endl;
    // xxx fehlermeldung!!!
  }
    
  Syncee* syncee;
  // Set all Syncee's to meta sync mode
  for (syncee = lis.first(); syncee != 0; syncee = lis.next())
  {
    syncee->setSyncMode(KSync::Syncee::MetaMode);
  }


  // Do meta sync if enabled and not first sync
  /* xxx not used yet. first meta sync has to be implemented!
  if (m_meta && !m_firstSync)
  {
    Syncee::PtrList synceePtrListOld;
    // Get meta info from file    
    QFile file(m_metaPath);
    if (file.open(IO_ReadOnly))
    {
      QByteArray array = file.readAll();
      file.close();
      synceePtrListOld = Helper::XML2Syncee(array, m_meta);
      // Do meta sync with stored data from last sync
      synceePtrListNew = Helper::doMeta(&synceePtrListOld, &synceePtrListNew);
      Syncee* syncee;
      // Set all Syncee's to meta sync mode
    }
    else
    {
      // xxx error handling!!
      kdDebug() << "Can't open file :-(" << endl;
    }
  }*/
  // emit the sync signal -> PV is ready to sync!
  emit sync(lis);
}

void CasioPVLink::getAllEntriesDone(const QByteArray& array)
{
  kdDebug() << "CasioPVLink::getAllEntriesDone() received" << endl;
  
  // Check connection mode
  if (m_mode == SYNC)
  {
    // Convert received data to Syncee::PtrList
  
    Syncee::PtrList lis;
    QDomDocument doc("mydocument");
    // Set content of DOM document
    if (doc.setContent(array))
    {
      // Parse DOM Document
      QDomElement docElem = doc.documentElement();
      if (docElem.tagName() == QString::fromLatin1("pvdataentries"))
      {
        kdDebug() << "pvdataentries found!" << endl;
        QDomNode n =  docElem.firstChild(); // child of pvdataentries -> type of entries (e.g. contacts)
        while(!n.isNull())
        {
          QDomElement e = n.toElement();      
          kdDebug() << e.tagName() << " found!" << endl;
          if (e.tagName() == QString::fromLatin1("contacts"))
          {
            QDomNode n = e.firstChild();
            AddressBookSyncee* syncee = new AddressBookSyncee();
            syncee = AddressBook::toAddressBookSyncee(n);
            lis.append(syncee);
          }
          else if (e.tagName() == QString::fromLatin1("events"))
          {
            QDomNode n = e.firstChild();          
            EventSyncee* syncee = new EventSyncee();
            syncee = Event::toEventSyncee(n);
            lis.append(syncee);
          }
          else if (e.tagName() == QString::fromLatin1("todos"))
          {
            QDomNode n = e.firstChild();          
            TodoSyncee* syncee = new TodoSyncee();
            syncee = Todo::toTodoSyncee(n);
            lis.append(syncee);
          }
          n = n.nextSibling(); // jump to next element
        }  
      }  // end of if pvdataentries
      else
      {
        kdDebug() << "PVHelper::XML2Syncee -> pvdataentries not found" << endl;
        // xxx fehlermeldung!!! syncee auf null setzen? wie geht das?
      }
    }  // end of if doc.setContents()
    else
    {
      kdDebug() << "PVHelper::XML2Syncee !doc.setContent() " << endl;
      // xxx fehlermeldung!!!
    }
    
    Syncee* syncee;
    // Set all Syncee's to first sync mode
    for (syncee = lis.first(); syncee != 0; syncee = lis.next())
    {
      syncee->setFirstSync(m_firstSync);
    }
    // emit the sync signal -> PV is ready to sync!
    emit sync(lis);
  }
  else if (m_mode == BACKUP)
  {
     kdDebug() << "CasioPVLink::getAllEntriesDone() backup_mode" << endl;
    // Store received data from PV to a file
    QFile file(m_backupPath);
    if (file.open(IO_WriteOnly))
    {
      file.writeBlock(array);
      file.close();
    }
    // Call disconnectPV()
    callDisconnectPV();
  }
}

void CasioPVLink::setChangesDone(const bool ok)
{
  kdDebug() << "CasioPVLink::setChangesDone() received" << endl;
  if (ok)
  {
    // Store synchronized data to a file for next meta syncing
    kdDebug(5205) << "storing meta data to: " << m_metaPath << endl;
    // xxx Has to be removed if meta sync will be implemented    
    QFile file(QDir::homeDirPath() + "/.kitchensync/meta/idhelper/konnector-ids.conf");
    file.remove();
    /* xxx not used yet. first meta sync has to be implemented!
    QFile file(m_metaPath);
    if (file.open(IO_WriteOnly))
    {
      file.writeBlock(m_array);
      file.close();
    } */
    // Call disconnectPV()
    callDisconnectPV();    
  }
  else
  {
    emit errorKonnector(20000, "Error while writing to PV");
  }
}

void CasioPVLink::setAllEntriesDone(const bool ok)
{
  kdDebug() << "CasioPVLink::setAllEntriesDone() received" << endl;
  if (ok)
  {
    // Call disconnectPV()
    callDisconnectPV();  
  }
  else
  { 
    emit errorKonnector(20000, "Error while writing to PV");
  }
}

void CasioPVLink::errorPV(const QString& msg, const unsigned int errorcode)
{
  kdDebug() << "CasioPVLink::errorPV::Error received." << endl;
  
  emit errorKonnector(errorcode, msg);
  m_state = DISCONNECTED;
  emit stateChanged(false);
}

#include "casiopvlink.moc"
