/***************************************************************************
                          pvdaemon.cpp  -  description
                             -------------------
    begin                : Wed Oct 02 2002
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

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <fstream>

#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>

#include <qbuffer.h>
#include <qstring.h>
#include <qtextstream.h>
#include <dcopclient.h>

// project includes
#include "casiopv.h"
#include "casiopvexception.h"
#include "pvdataentry.h"
#include "pvcontact.h"
#include "pvcontactuntitled.h"
#include "pvexpense.h"
#include "pvmemo.h"
#include "pvpocketsheet.h"
#include "pvquickmemo.h"
#include "pvschedule.h"
#include "pvmultidate.h"
#include "pvreminder.h"
#include "pvtodo.h"
#include "ModeCode.h"
#include "utils.h"

#include "pvdaemon.h"

using namespace std;

using namespace CasioPV;

/**
   * Constructor.
   */
pvDaemon::pvDaemon() : DCOPObject("PVDaemonIface")
{
  kdDebug(5205) << "pvDaemon constructor" << endl;
  // Construct a CasioPV object
  casioPV = new CasioPV();
}

/**
   * Destructor.
   */
pvDaemon::~pvDaemon()
{
  kdDebug(5205) << "pvDaemon destructor" << endl;
  delete casioPV;
}


// ------------------------- Public DCOP interface ------------------------- //

/**
   * Belongs to the DCOP interface of the PV Daemon.
   * This method is called when the PV has to be connected.
   * @param port The port where the PV is connected to (e.g. /dev/ttyS0)
   * @return QString The parameters of the connected PV (mode code,
   * optional code, secret area)
   */
QStringList pvDaemon::connectPV(const QString& port)
{
  try
  {
    kdDebug() << "pvDaemon connectpv on port: " <<  port.latin1() << endl;
    // Connect PV
    casioPV->OpenPort(port.latin1());
    casioPV->WakeUp();
    casioPV->WaitForLink(38400);
    // Get Model Code, Optional Code and Secret Area
    QString modelCode = (QString)(casioPV->GetModelCode().c_str());
    QString optionalCode = (QString)(casioPV->GetOptionalCode().c_str());
    bool secretArea = casioPV->isInSecretArea();
    QStringList strList;
    if (secretArea)
    {
      strList << modelCode << optionalCode << "secretArea";
    }
    else
    {
      strList << modelCode << optionalCode;
    }
    return strList;
  }
  catch (BaseException e)
  {
    sendException(e.getMessage().c_str(), e.getErrorCode());
  }
  return QStringList();
}

/**
   * Belongs to the DCOP interface of the PV Daemon.
   * This method is called when the PV has to be disconnected.
   * @return bool Successful disconnecting of PV (yes / no)
   */
bool pvDaemon::disconnectPV( void )
{
  try
  {
    kdDebug(5202) << "pvDaemon disconnectpv" << endl;
    // Release link and close port
    casioPV->ReleaseLink();
    casioPV->ClosePort();

    return true;
  }
  catch (BaseException e)
  {
    sendException(e.getMessage().c_str(), e.getErrorCode());
  }
  return false;
}

/**
   * Belongs to the DCOP interface of the PV Daemon.
   * This method is called to get the changes on the PV since the last
   * synchronisation.
   * @param categories List of categories to fetch data
   */
void pvDaemon::getChanges(const QStringList& categories)
{
  try
  {
    kdDebug(5202) << "getChanges" << endl;
    // Prepare stream
    QByteArray array;
    QBuffer buffer(array);
    if (buffer.open(IO_WriteOnly))
    {
      if (!categories.isEmpty())
      {
        QStringList contacts;
        QStringList events;
        QStringList todos;
        QTextStream textStream(&buffer);
        textStream.setEncoding(QTextStream::UnicodeUTF8);
        // Start of XML file
        textStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE pvdataentries>" << endl
                    << "<pvdataentries>" << endl;

        // Get all entries of all categories
        for (QStringList::ConstIterator it = categories.begin(); it != categories.end(); it++)
        {
          // Split into big categories
          if ((*it).contains("Contact"))
          {
            contacts << (*it);
          }
          else if ((*it).contains("Schedule"))
          {
            events << (*it);
          }
          else if ((*it).contains("To Do"))
          {
            todos << (*it);
          }
        }
        if (!contacts.isEmpty())
        {
          textStream << "<contacts>" << endl;
          for (QStringList::ConstIterator it = contacts.begin(); it != contacts.end(); it++)
          {
            unsigned int category = Utils::getCategoryPV((*it).latin1());
            kdDebug() << "Category: " << category << endl;
            // Get the PVDataEntries of one category and convert them to xml
            textStream << getChangesFromPV(category);
          }
          textStream << "</contacts>" << endl;
        }
        if (!events.isEmpty())
        {
          textStream << "<events>" << endl;
          for (QStringList::ConstIterator it = events.begin(); it != events.end(); it++)
          {
            unsigned int category = Utils::getCategoryPV((*it).latin1());
            kdDebug() << "Category: " << category << endl;
            // Get the PVDataEntries of one category and convert them to xml
            textStream << getChangesFromPV(category);
          }
          textStream << "</events>" << endl;
        }
        if (!todos.isEmpty())
        {
          textStream << "<todos>" << endl;
          for (QStringList::ConstIterator it = todos.begin(); it != todos.end(); it++)
          {
            unsigned int category = Utils::getCategoryPV((*it).latin1());
            kdDebug() << "Category: " << category << endl;
            // Get the PVDataEntries of one category and convert them to xml
            textStream << getChangesFromPV(category);
          }
          textStream << "</todos>" << endl;
        }
        // Finish the XML file
        textStream << "</pvdataentries>" << endl;

        // Prepare DCOP send to CasioPVLink
        QByteArray data;
        QDataStream dataStream(data, IO_WriteOnly);
        dataStream << array;
        kdDebug() << "DCOP send getChangesDone" << endl;
        if (!kapp->dcopClient()->send("kitchensync", "CasioPVLinkIface",
                                     "getChangesDone(QByteArray)", data))
        {
          kdDebug() << "DCOP send failed" << endl;
        }
      } // if (!categories.isEmpty())
    } //if buffer.open xxx noch besser abfangen
  } // try
  catch (BaseException e)   // xxx jeder exception typ einzeln catchen
  {
    sendException(e.getMessage().c_str(), e.getErrorCode());
  }
}

/**
   * Belongs to the DCOP interface of the PV Daemon.
   * This method is called to get all data from the PV.
   * @param categories List of categories to fetch data
   */
void pvDaemon::getAllEntries(const QStringList& categories)
{
  try
  {
    kdDebug(5202) << "getAllEntries" << endl;
    // Prepare stream
    QByteArray array;
    QBuffer buffer(array);
    if (buffer.open(IO_WriteOnly))
    {
      if (!categories.isEmpty())

      {
        QStringList contacts;
        QStringList events;
        QStringList todos;
        QTextStream textStream(&buffer);
        textStream.setEncoding(QTextStream::UnicodeUTF8);
        // Start of XML file
        textStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE pvdataentries>" << endl
                    << "<pvdataentries>" << endl;

        // Get all entries of all categories
        for (QStringList::ConstIterator it = categories.begin(); it != categories.end(); it++)
        {
          // Split into big categories
          if ((*it).contains("Contact"))
          {
            contacts << (*it);
          }
          else if ((*it).contains("To Do"))
          {
            todos << (*it);
          }
          else if ((*it).contains("Schedule"))
          {
            events << (*it);
          }
        }
        if (!contacts.isEmpty())
        {
          textStream << "<contacts>" << endl;
          for (QStringList::ConstIterator it = contacts.begin(); it != contacts.end(); it++)
          {
            unsigned int category = Utils::getCategoryPV((*it).latin1());
            kdDebug() << "Category: " << category << endl;
            // Get the PVDataEntries of one category and convert them to xml
            textStream << getAllEntriesFromPV(category);
          }
          textStream << "</contacts>" << endl;
        }
        if (!todos.isEmpty())
        {
          textStream << "<todos>" << endl;
          for (QStringList::ConstIterator it = todos.begin(); it != todos.end(); it++)
          {
            unsigned int category = Utils::getCategoryPV((*it).latin1());
            kdDebug() << "Category: " << category << endl;
            // Get the PVDataEntries of one category and convert them to xml
            textStream << getAllEntriesFromPV(category);
          }
          textStream << "</todos>" << endl;
        }
        if (!events.isEmpty())
        {
          textStream << "<events>" << endl;
          for (QStringList::ConstIterator it = events.begin(); it != events.end(); it++)
          {
            unsigned int category = Utils::getCategoryPV((*it).latin1());
            kdDebug() << "Category: " << category << endl;
            // Get the PVDataEntries of one category and convert them to xml
            textStream << getAllEntriesFromPV(category);
          }
          textStream << "</events>" << endl;
        }
        // Finish the XML file
        textStream << "</pvdataentries>" << endl;

        kdDebug() << "dumping entries" << array.data() << endl;

        // Prepare DCOP send to CasioPVLink
        QByteArray data;
        QDataStream dataStream(data, IO_WriteOnly);
        dataStream << array;
        kdDebug() << "DCOP send getAllEntriesDone" << endl;
        if (!kapp->dcopClient()->send("kitchensync", "CasioPVLinkIface",
                                     "getAllEntriesDone(QByteArray)", data))
        {
          kdDebug() << "DCOP send failed" << endl;
        }
      } // if (!categories.isEmpty())
    } //if buffer.open xxx noch besser abfangen
  } // try
  catch (BaseException e)   // xxx jeder exception typ einzeln catchen
  {
    sendException(e.getMessage().c_str(), e.getErrorCode());
  }
}

/**
   * Belongs to the DCOP interface of the PV Daemon.
   * This method is called when the changes after synchronisation have
   * to be written to the PV.
   * @param optionalCode The optional code to be stored on the PV
   * @param array The changed data as QByteArray
*/
void pvDaemon::setChanges(const QString& optionalCode, const QByteArray& array)
{
  try
  {
    bool ok;
    kdDebug() << "Begin:pvdaemon::setChanges: " << endl;
    QDomDocument doc("mydocument");
    if(doc.setContent(array))
    {
      // Parse DOM Document
      QDomElement docElem = doc.documentElement();
      if (docElem.tagName() == QString::fromLatin1("pvdataentries"))
      {
        kdDebug() << "pvdataentries found!" << endl;
        // Get child of pvdataentries
        // -> QDomNode points to first type of entries (e.g. contacts)
        QDomNode n =  docElem.firstChild();

        // Write entries to PV
        writeEntries(n);
      }  // end of if pvdataentries
      else
      {
        sendException("PVPluginException: XML file format error. 'pvdataentries' not found!", 10005);
        return /*0l*/; // xxx syncee auf null setzen? wie geht das?
      }
      ok = true;
      // Change the optional code to detect at next sync if device was
      // already synced with kitchensync before
      casioPV->ChangeOptionalCode(optionalCode.latin1());
    }  // end of if doc.setContents()
    else
    {
      sendException("PVPluginException: XML file format error! Can't set content.", 10005);
      ok = false;
    }

    // Prepare DCOP send to CasioPVLink
    QByteArray data2;
    QDataStream dataStream(data2, IO_WriteOnly);
    dataStream << ok;
    kdDebug() << "DCOP send setChangesDone" << endl;
    if (!kapp->dcopClient()->send("kitchensync", "CasioPVLinkIface",
                                   "setChangesDone(bool)", data2))
    {
      kdDebug() << "DCOP send failed" << endl;
    }
  }  // try
  catch (BaseException e)   // xxx jeder exception typ einzeln catchen
  {
    sendException(e.getMessage().c_str(), e.getErrorCode());
  }
}

/**
   * Belongs to the DCOP interface of the PV Daemon.
   * This method is called when all entries have to be written to the
   * PV (restore, first synchronisation).
   * @param array The changed data as QByteArray
   */
void pvDaemon::setAllEntries(const QByteArray& array)
{
  try
  {
    bool ok;
    kdDebug() << "Begin:pvdaemon::setAllEntries: " << endl;
    QDomDocument doc("mydocument");
    if(doc.setContent(array))
    {
      // Parse DOM Document
      QDomElement docElem = doc.documentElement();
      if (docElem.tagName() == QString::fromLatin1("pvdataentries"))
      {
        kdDebug() << "pvdataentries found!" << endl;
        // Get child of pvdataentries
        // -> QDomNode points to first type of entries (e.g. contacts)
        QDomNode n =  docElem.firstChild();

        // Write entries to PV
        writeEntries(n, true);
      }  // end of if pvdataentries
      else
      {
        sendException("PVPluginException: XML file format error. 'pvdataentries' not found!", 10005);
        return /*0l*/; // xxx syncee auf null setzen? wie geht das?
      }
      ok = true;
    }  // end of if doc.setContents()
    else
    {
      sendException("PVPluginException: XML file format error! Can't set content.", 10005);
      ok = false;
    }

    // Prepare DCOP send to CasioPVLink
    QByteArray data2;
    QDataStream dataStream(data2, IO_WriteOnly);
    dataStream << ok;
    kdDebug() << "DCOP send setChangesDone" << endl;
    if (!kapp->dcopClient()->send("kitchensync", "CasioPVLinkIface",
                                   "setAllEntriesDone(bool)", data2))
    {
      kdDebug() << "DCOP send failed" << endl;
    }
  }  // try
  catch (BaseException e)   // xxx jeder exception typ einzeln catchen
  {
    sendException(e.getMessage().c_str(), e.getErrorCode());
  }
}

/**
  * Belongs to the DCOP interface of the PV Daemon.
  * This method can be used to check whether the PV is connected
  * @return bool PV connected (true / false)
  */
bool pvDaemon::isConnected( void ) // xxx evtl. löschen xxx
{
  return true;
}


// ------------------------------ private methods -----------------------------

/**
   * Constructs a new empty entry derived from PVDataEntry. The category
   * and the uid are used in the constructor.
   * @param category The category of the entry
   * @param uid The uid of the entry
   * @return PVDataEntry* The new empty PVDataEntry
   */
PVDataEntry* pvDaemon::ClearEntry(unsigned int category, unsigned int uid)
{
  PVDataEntry* entry = 0;
  // create new data element
  switch ( category ) {
    case CONTACT_PRIVATE :
    case CONTACT_BUSINESS :
      entry = new PVContact(category, uid);
      break;

    case CONTACT_UNTITLED_1 :
    case CONTACT_UNTITLED_2 :
    case CONTACT_UNTITLED_3 :
//   only for PV-750
    case CONTACT_UNTITLED_4 :
    case CONTACT_UNTITLED_5 :
        entry = new PVContactUntitled(category, uid);
      break;

    case MEMO_1 :
    case MEMO_2 :
    case MEMO_3 :
    case MEMO_4 :
    case MEMO_5 :
        entry = new PVMemo(category, uid);
      break;

    case SCHEDULE :
        entry = new PVSchedule(uid);
      break;

    case SCHEDULE_MULTI_DATE :
        entry = new PVMultiDate(uid);
      break;

    case SCHEDULE_REMINDER :
        entry = new PVReminder(uid);
      break;

    case EXPENSE_PV :
        entry = new PVExpense(uid);
      break;

    case TODO :
        entry = new PVTodo(uid);
      break;

    case POCKET_SHEET_PV :
        entry = new PVPocketSheet(uid);
      break;

    case QUICK_MEMO :
        entry = new PVQuickMemo(uid);
      break;
  }
  return entry;
}

/**
   * Gets all entries of the specified category from the PV.
   * @param category The category where the entries are fetched from
   * @return QString The entries as XML string
   */
QString pvDaemon::getAllEntriesFromPV(unsigned int category)
{
  try
  {
    QString str;
    unsigned int NoOfData =  casioPV->GetNumberOfData(category);
    PVDataEntry* entry = 0;
    if (NoOfData > 0)
    {
      // get all entries
      for (unsigned int i = 0; i < NoOfData; i++)
      {
        entry = ClearEntry(category, i+1);
        // get the entry from PV
        casioPV->GetEntry(*entry, i);
        // convert the entry to an XML string
        str.append(QString(entry->toXML().c_str()));
      }
    } // end if
    return str;
  }
  catch (CasioPVException e)
  {
    kdDebug() << "CasioPVException catched" << endl;
    sendException(e.getMessage().c_str(), e.getErrorCode());
  }
  return QString::null;
}

/**
   * Gets the changes of the specified category from the PV.
   * @param category The category where the entries are fetched from
   * @return QString The entries as XML string
   */
QString pvDaemon::getChangesFromPV(unsigned int category)
{
  try
  {
    QString str;
    PVDataEntry* entry = 0;
    ModifyList modifylist; // Used to store the id's of the modified entries
    // Check which entries have changed
    modifylist = casioPV->CheckForModifiedEntries(category);

    // get all changes
    for (ModifyList::iterator iter = modifylist.begin(); iter !=modifylist.end(); iter++)
    {
      entry = ClearEntry(category, (*iter).number);
      if ( (*iter).modified )
      {
        kdDebug() << "Modified entry found. Nr: " << (*iter).number << endl;
        casioPV->GetModifiedEntry(*entry, (*iter).number);
        entry->setState(PVDataEntry::MODIFIED);
        // convert the entry to an XML string
        str.append(QString(entry->toXML().c_str()));
      }
      else if ( (*iter).number == 0xffffff )
      {
        kdDebug() << "New entry found. Nr: " << (*iter).number << endl;
        entry->setUid(casioPV->GetNewEntry(*entry));
        entry->setState(PVDataEntry::ADDED);
        // convert the entry to an XML string
        str.append(QString(entry->toXML().c_str()));
      }
      else
      {
        // unchanged entry -> has to be added to stream to detect removed
        // entries since the last sync later on (in doMeta).
        kdDebug() << "Unchanged entry found. Nr: " << (*iter).number << endl;
        // convert the entry to an XML string
        str.append(QString(entry->toXML().c_str()));
      }
    } // end for
    return str;
  }
  catch (CasioPVException e)
  {
    kdDebug() << "CasioPVException catched" << endl;
    sendException(e.getMessage().c_str(), e.getErrorCode());
  }
  return QString::null;
}

/**
   * Writes the entries included in a DOM node to the PV.
   * @param ignoreState If set, the state of the entries are ignored and
   * all entries are written to the PV
   */
void pvDaemon::writeEntries(QDomNode& n, bool ignoreState)
{
  try
  {
    PVDataEntry* entry = 0; // container to hold the PV entry to be written

    // Process all categories (contacts, events, todos, ...)
    while(!n.isNull())
    {
      QDomElement e = n.toElement();
      kdDebug() << e.tagName() << " found!" << endl;

      if ((e.tagName() == QString::fromLatin1("contacts"))
           || (e.tagName() == QString::fromLatin1("events"))
             || (e.tagName() == QString::fromLatin1("todos")))
      {
        QDomNode n = e.firstChild(); // child of contacts -> all entities "contact"
        // Process all entries of one category
        while(!n.isNull())
        {
          QDomElement e = n.toElement();
          // Get uid and state from entry
          unsigned int uid = (e.attribute("uid")).toUInt();
          QString state = e.attribute("state");
          unsigned int category = 0;

          if (e.tagName() == QString::fromLatin1("contact"))
          {
            // get category from entry
            category = Utils::getCategoryPV((e.attribute("category")).latin1());
            switch (category)
            {
              case CONTACT_PRIVATE :
              case CONTACT_BUSINESS :
              case CONTACT_UNTITLED_1 :
              case CONTACT_UNTITLED_2 :
              case CONTACT_UNTITLED_3 :
              //   only for PV-750
              case CONTACT_UNTITLED_4 :
              case CONTACT_UNTITLED_5 :
                break;
              default:
                // if category is missing or wrong
                kdDebug() << "Contact category not found. Setting default category!" << endl;
                category = CONTACT_BUSINESS;
                break;
            }
          }
          // xxx wird wohl nicht gebraucht!!!
          // (category geht verloren, da pv die kategorie nicht speichern kann wie bei Kontakten)
          else if (e.tagName() == QString::fromLatin1("event"))
          {
            // get category from entry
            category = Utils::getCategoryPV((e.attribute("category")).latin1());
            switch (category)
            {
              case SCHEDULE :
              case SCHEDULE_MULTI_DATE :
              case SCHEDULE_REMINDER :
                break;
              default:
                // if category is missing or wrong
                kdDebug() << "Event category not found. Setting default category!" << endl;
                category = SCHEDULE;
                break;
            }
          }
          else
          {
            if (e.tagName() == QString::fromLatin1("todo"))
            {
              category = TODO;
            }
          }
          QString str;
          QTextStream textStream(&str, IO_WriteOnly);

          entry = ClearEntry(category, uid);
          textStream << e;
          //kdDebug() << e.tagName() << " as string: " << endl << str << endl;
          if ((state == "added") || ignoreState)
          {
            // convert xml to PVDataEntry
            entry->fromXML(str.latin1());

            kdDebug() << entry->toXML().c_str() << endl;

            if (entry->isSendable())
            {
                kdDebug() << "adding entry" << endl;
              // write entry to PV if sendable
              uid = casioPV->AddEntry(*entry);
            }
            else
            {
              sendException("PVPluginException: Entry is not sendable. Not all necessary fields are entered!", 10004, false);
            }
          }
          else if (state == "modified")
          {
            // convert xml to PVDataEntry
            entry->fromXML(str.latin1());
            if (entry->isSendable())
            {
              // write modified entry to PV
              casioPV->ModifyEntry(*entry, entry->getUid());
            }
            else
            {
              sendException("PVPluginException: Entry is not sendable. Not all necessary fields are entered!", 10004, false);
            }
          }
          else if (state == "removed")
          {
            // convert xml to PVDataEntry
            entry->fromXML(str.latin1());
            // remove entry from PV
            // xxx wieder einschalten!!! casioPV->DeleteEntry(category,entry->getUid());
          }
          n = n.nextSibling(); // jump to next entry
        }  // while(!n.isNull())
      }
      else
      {
        sendException("PVPluginException: XML file format error. Wrong category entity!", 10005);
      }
      n = n.nextSibling(); // jump to next category
    } // while(!n.isNull())
  } // try
  catch (CasioPVException e)
  {
    kdDebug() << "CasioPVException catched" << endl;
    sendException(e.getMessage().c_str(), e.getErrorCode());
  }
}

/**
   * Sends an exception as a DCOP call to the PV Plugin.
   * @param msg The error message
   * @param number The error number
   * @param disconnected Specifies whether the connection to the PV has
   * to be released due to this error
   */
void pvDaemon::sendException(const QString& msg, const unsigned int number, bool disconnect)
{
  kdDebug(5205) << "sendException: " << msg << endl;
  // Prepare DCOP send to CasioPVLink
  QByteArray data;
  QDataStream dataStream(data, IO_WriteOnly);
  dataStream << i18n(msg) << number;
  if (!kapp->dcopClient()->send("kitchensync", "CasioPVLinkIface",
                                 "errorPV(QString,unsigned int)", data))
  {
    kdDebug() << "DCOP send failed" << endl;
  }
  if (disconnect)
  {
    // Release link and close port
    casioPV->ReleaseLink();
    casioPV->ClosePort();
  }
}
