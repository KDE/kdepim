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

/**
   * Constructor.
   * @param obj The parent object
   * @param name The name of the object
   */
CasioPVLink::CasioPVLink(QObject *obj, const char *name)
    : QObject(obj, name), DCOPObject("CasioPVLinkIface")
{
  kdDebug(5205) << "CasioPVLink constructor" << endl;

  // Init variables
  m_state = DISCONNECTED;
  m_mode = NONE;
  // m_first sync is set to false, if connected PV was already synchronized
  m_firstSync = true;
  // set categories to be synchronized
}

/**
   * Destructor.
   */
CasioPVLink::~CasioPVLink()
{
  kdDebug(5205) << "CasioPVLink destructor" << endl;
}

/**
   * Signals that the synchronisation has to be started.
   * @return bool Starting of synchronisation successful (yes / no)
   */
bool CasioPVLink::startSync()
{
  kdDebug(5205) << "startSync CasioPVLink" << endl;
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
    strList << "To Do" << "Schedule" << "Schedule Multi Date" << "Schedule Reminder";
    dataStream << strList;
    if (m_firstSync)
    {
      // Get all entries from PV if first synchronisation
      kdDebug(5205) << "DCOP send getAllEntries()" << endl;
      if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                     "getAllEntries(QStringList)", data))
      {
        emit errorKonnector(10001, "PVPluginException: Error with DCOP!\nIs pvdaemon running?");
        kdDebug(5205) << "DCOP send failed" << endl;
        return false;
      }
    }
    else
    {
      // Get changes from PV if this PV was already synchronized
      kdDebug(5205) << "DCOP send getChanges()" << endl;
      if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                     "getChanges(QStringList)", data))
      {
        emit errorKonnector(10001, "PVPluginException: Error with DCOP!\nIs pvdaemon running?");
        kdDebug(5205) << "DCOP send failed" << endl;
        return false;
      }
    }
  }  // end if
  return false;
}

/**
   * Signals that the backup has to be started.
   * @param path The path of the backup file
   * @return bool Starting of backup successful (yes / no)
   */
bool CasioPVLink::startBackup(const QString& path)
{
  kdDebug(5205) << "startBackup CasioPVLink" << endl;

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
    strList << "To Do" << "Schedule" << "Schedule Multi Date" << "Schedule Reminder";
    dataStream << strList;
    kdDebug(5205) << "DCOP send getAllEntries()" << endl;
    if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                   "getAllEntries(QStringList)", data))
    {
      emit errorKonnector(10001, "PVPluginException: Error with DCOP!\nIs pvdaemon running?");
      kdDebug(5205) << "DCOP send failed" << endl;
      return false;
    }
    return true;
  }  // end if
  return false;
}

/**
   * Signals that the restore has to be started.
   * @param path The path of the restore file
   * @return bool Starting of restore successful (yes / no)
   */
bool CasioPVLink::startRestore(const QString& path)
{
  kdDebug(5205) << "CasioPVLink::restore" << endl;
  if (callConnectPV())
  {
    // Read backup file from HD
    QByteArray array;
    QFile file(path);
    if (file.open(IO_ReadOnly))
    {
      kdDebug(5205) << "CasioPVLink::restore backup file found!" << endl;
      array = file.readAll();
      file.close();
      // Prepare DCOP send to PVDaemon
      QByteArray data;
      QDataStream dataStream(data, IO_WriteOnly);
      dataStream << array;

      kdDebug(5205) << "DCOP send setAllEntries()" << endl;
      if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                     "setAllEntries(QByteArray)", data))
      {
        emit errorKonnector(10001, "PVPluginException: Error with DCOP!\nIs pvdaemon running?");
        kdDebug(5205) << "DCOP send failed" << endl;
        return false;
      }
    }
    else
    {
      emit errorKonnector(10007, "PVPluginException: Restore file not found!");
    }
  }
  return false;
}

/**
   * Sets the PV model to be synchronized depending on kapabilities
   * chosen in configuration
   * @param model The PV model to be synchronized
   */
void CasioPVLink::setModel(const QString& model)
{
  m_model = model;
}

/**
   * Sets the connection mode depending on kapabilities chosen
   * in configuration.
   * @param connectionMode The connection mode (/dev/ttyS0 or /dev/ttyS1)
   */
void CasioPVLink::setConnectionMode(const QString& connectionMode)
{
  m_connectionMode = connectionMode;
}

/**
   * Sets the meta syncing depending on kapabilities chosen in
   * configuration. Meta syncing can be enabled or disabled.
   * @param meta Meta synchronisation enabled (true) or disabled (false)
   */
void CasioPVLink::setMetaSyncing(const bool meta)
{
  m_meta = meta;
}

/**
   * Returns whether the PV is connected
   * @return bool PV connected (yes / no)
   */
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

/**
   * Signals that the synchronisation was done and the synchronized data
   * can be written to the PV.
   * @param lis A Syncee::PtrList of the synchronized date
   */
void CasioPVLink::write(Syncee::PtrList lis)
{
  kdDebug(5205) << "write back to PV" << endl;

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
        kdDebug(5205) << syncee->type() << " found!" << endl;
        if (syncee->type() == QString::fromLatin1("AddressBookSyncee"))
        {
		  AddressBookSyncee* abs = dynamic_cast<AddressBookSyncee*>(syncee);
		  if ( abs )
            stream << (AddressBook::toXML(abs));
        }
        else if (syncee->type() == QString::fromLatin1("EventSyncee"))
        {
		  EventSyncee* es = dynamic_cast<EventSyncee*>(syncee);
		  if ( es )
          	stream << (Event::toXML(es));
        }
        else if (syncee->type() == QString::fromLatin1("TodoSyncee"))
        {
		  TodoSyncee* ts = dynamic_cast<TodoSyncee*>(syncee)
		  if (ts)
          	stream << (Todo::toXML(ts));
        }
      }
      stream << "</pvdataentries>" << endl;
    }  // end if

    /* xxx not used yet. first meta sync has to be implemented!
    // Store data to be written to HD (for meta syncing) after writing to PV
    m_array = array; */

    // Prepare DCOP send to PVDaemon
    QByteArray data;
    QDataStream dataStream(data, IO_WriteOnly);
    dataStream << m_optionalCode << array;

    kdDebug(5205) << "DCOP send setChanges()" << endl;
    if (!kapp->dcopClient()->send("pvDaemon", "PVDaemonIface",
                                   "setChanges(QString, QByteArray)", data))
    {
      kdDebug(5205) << "DCOP send failed" << endl;
    }
  }
}

/**
   * Returns which PV model is connected to the PC.
   * @return QString The PV model connected to the PC
   */
QString CasioPVLink::metaId() const
{
  return m_modelCode;
}

// ---------------------------- Private Methods ---------------------------- //

/**
   * Makes a DCOP call to the PV Library to connect the PV.
   * @return bool DCOP call successful (yes / no)
   */
bool CasioPVLink::callConnectPV()
{
  kdDebug(5202) << "callConnectPV()" << endl;
  
  if (m_state == CONNECTED)
  {
    kdDebug(5205) << "Is already syncing!!" << endl;
    emit errorKonnector(10002, "PVPluginException: Device is already connected!\nPlease wait until running process is finished.");
    return false;
  }
  else if (m_state == DISCONNECTED)
  {
    if ((m_connectionMode != "/dev/ttyS0") && (m_connectionMode != "/dev/ttyS1"))
    {
      emit errorKonnector(10009, "PVPluginException: Wrong or no serial port selected in configuration!");
      return false;    
    }
    // Prepare DCOP call to connectPV()
    QByteArray data, replyData;
    QCString replyType;
    QDataStream stream(data, IO_WriteOnly);
    stream << m_connectionMode;

    // Call connectPV()
    if (kapp->dcopClient()->call("pvDaemon", "PVDaemonIface",
                              "connectPV(QString)", data, replyType, replyData))
    {
      kdDebug(5205) << "Reply Type: " << replyType << endl;
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
      
      // Check if the correct PV model was connected -> xxx add more models!
      if (((m_model == "PV-750 Plus") && (!m_modelCode.startsWith("h"))) ||
           ((m_model == "PV-S660") && (!m_modelCode.startsWith("v"))) ||
             (m_model == ""))
      {       
        // disconnect PV and emit an exception
        callDisconnectPV();
        emit errorKonnector(10008, "PVPluginException: Wrong or no PV model selected in configuration!");
        return false;      
      }
      // Check if the connected PV was already synchronized before.
      /* xxx not used yet. first meta sync has to be implemented!
      if (m_meta)
      {
        m_metaPath = QString(QDir::homeDirPath() + "/.kitchensync/meta/"
                              + m_optionalCode.right(12) + "/pvmeta.xml");
        kdDebug(5205) << "Path: " << m_metaPath << " Code: " << m_optionalCode << endl;
        // Optional code starts correct (maybe PV was already synced)
        if ((m_optionalCode.left(11) == "KitchenSync") && (QFile::exists(m_metaPath)))
        {
          kdDebug(5205) << "Was already synching with this one!" << endl;
          m_firstSync = false;
        }
        else
        {
          kdDebug(5205) << "Never synched with this one before! Optional Code: " << m_optionalCode << endl;
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
      kdDebug(5205) << "CasioLVLink:: emit stateChanged" << endl;
    }
    else
    {
      emit errorKonnector(10001, "PVPluginException: Error with DCOP!\nIs pvdaemon running?");
      kdDebug(5205) << "Error with DCOP" << endl;
      return false;
    }
  }  // if m_state == DISCONNECTED
  return true;
}

/**
   * Makes a DCOP call to the PV Library to disconnect the PV.
   * @return bool DCOP call successful (yes / no)
   */
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
      emit errorKonnector(10006, "PVPluginException: Connection could not be released!");
    }
  }
}


// ------------------------ Public DCOP interface -------------------------- //

/**
  * Belongs to the DCOP interface of the PV Plugin.
  * This method is called when all changes are fetched from the PV.
  * @param stream The changes from PV to be synchronized.
  */
void CasioPVLink::getChangesDone(const QByteArray& array)
{
  kdDebug(5205) << "CasioPVLink::getChangesDone() received" << endl;

  // xxx Has to be removed if meta sync will be implemented
  if (QFile::exists(QDir::homeDirPath() + "/.kitchensync/meta/idhelper/konnector-ids.conf"))
  {
    QFile file(QDir::homeDirPath() + "/.kitchensync/meta/idhelper/konnector-ids.conf");
    file.remove();
  }

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
      kdDebug(5205) << "pvdataentries found!" << endl;
      QDomNode n =  docElem.firstChild(); // child of pvdataentries -> type of entries (e.g. contacts)
      while(!n.isNull())
      {
        QDomElement e = n.toElement();
        kdDebug(5205) << e.tagName() << " found!" << endl;
        QDomNode n = e.firstChild();
        // Convert entries to Syncees
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
      kdDebug(5205) << "PVHelper::XML2Syncee -> pvdataentries not found" << endl;
      emit errorKonnector(10005, "PVPluginException: XML file format error. 'pvdataentries' not found!");
    }
  }  // end of if doc.setContents()
  else
  {
    kdDebug(5205) << "PVHelper::XML2Syncee !doc.setContent() " << endl;
      emit errorKonnector(10005, "PVPluginException: XML file format error. File is not valid!");
  }

  Syncee* syncee;
  // Set all Syncee's to first sync mode
  for (syncee = lis.first(); syncee != 0; syncee = lis.next())
  {
    syncee->setFirstSync(m_firstSync);
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
      kdDebug(5205) << "Can't open file :-(" << endl;
    }
  }*/
  // emit the sync signal -> PV is ready to sync!
  emit sync(lis);
}

/**
   * Belongs to the DCOP interface of the PV Plugin.
   * This method is called when all data is fetched from the PV.
   * @param stream The data from PV to be synchronized.
   */
void CasioPVLink::getAllEntriesDone(const QByteArray& array)
{
  kdDebug(5205) << "CasioPVLink::getAllEntriesDone() received" << endl;

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
        kdDebug(5205) << "pvdataentries found!" << endl;
        QDomNode n =  docElem.firstChild(); // child of pvdataentries -> type of entries (e.g. contacts)
        while(!n.isNull())
        {
          QDomElement e = n.toElement();
          kdDebug(5205) << e.tagName() << " found!" << endl;
          // Convert entries to Syncees
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
        kdDebug(5205) << "PVHelper::XML2Syncee -> pvdataentries not found" << endl;
      emit errorKonnector(10005, "PVPluginException: XML file format error. 'pvdataentries' not found!");
      }
    }  // end of if doc.setContents()
    else
    {
      kdDebug(5205) << "PVHelper::XML2Syncee !doc.setContent() " << endl;
      emit errorKonnector(10005, "PVPluginException: XML file format error. File is not valid!");
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
     kdDebug(5205) << "CasioPVLink::getAllEntriesDone() backup_mode" << endl;
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

/**
   * Belongs to the DCOP interface of the PV Plugin.
   * This method is called when all changes were written to the PV
   * after synchronisation.
   * @param ok Writing successful (yes / no)
   */
void CasioPVLink::setChangesDone(const bool ok)
{
  kdDebug(5205) << "CasioPVLink::setChangesDone() received" << endl;
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
    emit errorKonnector(10003, "PVPluginException: Error while writing to PV");
  }
}

/**
   * Belongs to the DCOP interface of the PV Plugin.
   * This method is called when all data was written to the PV
   * after synchronisation.
   * @param ok Writing successful (yes / no)
   */
void CasioPVLink::setAllEntriesDone(const bool ok)
{
  kdDebug(5205) << "CasioPVLink::setAllEntriesDone() received" << endl;
  if (ok)
  {
    // Call disconnectPV()
    callDisconnectPV();
  }
  else
  {
    emit errorKonnector(10003, "PVPluginException: Error while writing to PV");
  }
}

/**
   * Belongs to the DCOP interface of the PV Plugin.
   * This method is called when an error occurred in the PV Library
   * @param msg The error message to be displayed
   * @param errorcode The error number of the exception
   */
void CasioPVLink::errorPV(const QString& msg, const unsigned int errorcode)
{
  kdDebug(5205) << "CasioPVLink::errorPV::Error received." << endl;

  emit errorKonnector(errorcode, msg);
  m_state = DISCONNECTED;
  emit stateChanged(false);
}

#include "casiopvlink.moc"
