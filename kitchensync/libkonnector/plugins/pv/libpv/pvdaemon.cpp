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

#include <iostream.h>
#include <iomanip.h>
#include <stdlib.h>
#include <fstream.h> 

#include <kdebug.h>
#include <kapp.h>
#include <klocale.h>

#include <qbuffer.h>
#include <qstring.h>
#include <qtextstream.h>
#include <dcopclient.h>

// project includes
#include "casiopv.h"
#include "casiopvexception.h"
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

using namespace CasioPV;

pvDaemon::pvDaemon() : DCOPObject("PVDaemonIface")
{
  kdDebug(5202) << "pvDaemon constructor" << endl;
  // Construct a CasioPV object
  casioPV = new CasioPV();
}

pvDaemon::~pvDaemon()
{
  kdDebug(5202) << "pvDaemon destructor" << endl;
  delete casioPV;
}


// -------------------- Public DCOP interface -------------------- //

QStringList pvDaemon::connectPV(const QString& port)
{
  try
  {
    kdDebug() << "pvDaemon connectpv on port: " <<  port.latin1() << endl;
    casioPV->OpenPort(port.latin1());
    casioPV->WakeUp();
    casioPV->WaitForLink(38400);
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
    // Prepare DCOP send to CasioPVLink
    QByteArray data;
    QDataStream dataStream(data, IO_WriteOnly);
    dataStream << i18n(e.getMessage().c_str()) << e.getErrorCode();
    if (!kapp->dcopClient()->send("kitchensync", "CasioPVLinkIface",
                                   "errorPV(QString,unsigned int)", data))
    {
      kdDebug() << "DCOP send failed" << endl;
    }
  }
}

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
    // Prepare DCOP send to CasioPVLink
    QByteArray data;
    QDataStream dataStream(data, IO_WriteOnly);
    dataStream << i18n(e.getMessage().c_str()) << e.getErrorCode();
    if (!kapp->dcopClient()->send("kitchensync", "CasioPVLinkIface",
                                   "errorPV(QString,unsigned int)", data))
    {
      kdDebug() << "DCOP send failed" << endl;
    }
  }
}

void pvDaemon::getChanges(const QStringList& categories)
{


}

void pvDaemon::getAllEntries(const QStringList& categories)
{
  try
  {
    // Prepare stream
    QByteArray array;
    QBuffer buffer(array);
    if (buffer.open(IO_WriteOnly))
    {
      if (!categories.isEmpty())
      {
        QTextStream textStream(&buffer);
        textStream.setEncoding(QTextStream::UnicodeUTF8);
        // Start of XML file
        textStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE pvdataentries>" << endl
                    << "<pvdataentries>" << endl;

        // Get all entries of all categories
        for (QStringList::ConstIterator it = categories.begin(); it != categories.end(); it++)
        {
          unsigned int category = Utils::getCategoryPV((*it).latin1());
          // Get the PVDataEntries of one category and convert them to xml         
          textStream << getEntries(category).data();
        }
        // Finish the XML file
        textStream << "</pvdataentries>" << endl;
        
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
    // Prepare DCOP send to CasioPVLink
    QByteArray data;
    QDataStream dataStream(data, IO_WriteOnly);
    dataStream << i18n(e.getMessage().c_str()) << e.getErrorCode();
    if (!kapp->dcopClient()->send("kitchensync", "CasioPVLinkIface",
                                   "errorPV(QString,unsigned int)", data))
    {
      kdDebug() << "DCOP send failed" << endl;
    }
  }
}

void pvDaemon::setChanges(const QByteArray& array)
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
        kdDebug() << "PVHelper::PV2Syncee -> pvdataentries not found" << endl;
        return /*0l*/; // xxx syncee auf null setzen? wie geht das?
      }
      ok = true;
    }  // end of if doc.setContents()
    else
    {
      kdDebug() << "PVHelper::PV2Syncee !doc.setContent() " << endl;
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
    // Prepare DCOP send to CasioPVLink
    QByteArray data;
    QDataStream dataStream(data, IO_WriteOnly);
    dataStream << i18n(e.getMessage().c_str()) << e.getErrorCode();
    if (!kapp->dcopClient()->send("kitchensync", "CasioPVLinkIface",
                                   "errorPV(QString,unsigned int)", data))
    {
      kdDebug() << "DCOP send failed" << endl;
    }
  }
}

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
        writeEntries(n);
      }  // end of if pvdataentries
      else
      {
        kdDebug() << "PVHelper::PV2Syncee -> pvdataentries not found" << endl;
        return /*0l*/; // xxx syncee auf null setzen? wie geht das?
      }
      ok = true;
    }  // end of if doc.setContents()
    else
    {
      kdDebug() << "PVHelper::PV2Syncee !doc.setContent() " << endl;
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
    // Prepare DCOP send to CasioPVLink
    QByteArray data;
    QDataStream dataStream(data, IO_WriteOnly);
    dataStream << i18n(e.getMessage().c_str()) << e.getErrorCode();
    if (!kapp->dcopClient()->send("kitchensync", "CasioPVLinkIface",
                                   "errorPV(QString,unsigned int)", data))
    {
      kdDebug() << "DCOP send failed" << endl;
    }
  }
}

bool pvDaemon::isConnected( void ) // xxx evtl. löschen xxx
{
  return true;
}


// ------------------------------ private methods -----------------------------

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

QByteArray pvDaemon::getEntries(unsigned int category)
{
  try
  {
    QString strCategory;  // used to set start and end tag of XML categories
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
        strCategory = "contacts";
        break;
      case MEMO_1 :
      case MEMO_2 :
      case MEMO_3 :
      case MEMO_4 :
      case MEMO_5 :
        strCategory = "memos";
        break;
      case SCHEDULE :
      case SCHEDULE_MULTI_DATE :
      case SCHEDULE_REMINDER :
        strCategory = "events";
        break;
      case TODO :
        strCategory = "todos";
        break;
      case EXPENSE_PV :
      case POCKET_SHEET_PV :
      case QUICK_MEMO :
      default:
        // not handled yet!
        kdDebug() << "Category " << category << " not implemented yet" << endl;
        break;
    } // switch

    // Prepare stream
    QByteArray array;
    QBuffer buffer(array);
    if (buffer.open(IO_WriteOnly))
    {
      QTextStream textStream(&buffer);
      textStream.setEncoding(QTextStream::UnicodeUTF8);
      unsigned int NoOfData =  casioPV->GetNumberOfData(category);
      PVDataEntry* entry = 0;
      // insert start tag of category
      textStream << "<" << strCategory << ">" << endl;
      // get all entries
      for (unsigned int i = 0; i < NoOfData; i++)
      {
        entry = ClearEntry(category, i);
        // get the entry from PV
        casioPV->GetEntry(*entry, i);
        // convert the entry to an XML string
        textStream << QString(entry->toXML().c_str());
      }
      // insert end tag of category
      textStream << "</" << strCategory << ">" << endl;
    } // end for NoOfData
        
    return array;
  }
  catch (CasioPVException e)
  {
    kdDebug() << "CasioPVException catched" << endl;
    // xxx fehlerbehandlung
  }
}

void pvDaemon::writeEntries(QDomNode& n)
{
  try
  {
    PVDataEntry* entry = 0; // container to hold the PV entry to be written

    QDomElement e = n.toElement();
    // Process all categories (contacts, events, todos, ...)
    while(!n.isNull())
    {
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
          // Get uid, category and state from entry
          unsigned int uid = (e.attribute("uid")).toUInt();
          kdDebug() << (e.attribute("category")).latin1() << " found" << endl;
          unsigned int category = Utils::getCategoryPV((e.attribute("category")).latin1());
          kdDebug() << "category as unsigned int: " << category << endl;
          QString state = e.attribute("state");
          kdDebug() << e.tagName() << " found" << endl;

          if (e.tagName() == QString::fromLatin1("contact"))
          {
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
          
          QString str;
          QTextStream textStream(&str, IO_WriteOnly);
          
          entry = ClearEntry(category, uid);
          textStream << e;
          kdDebug() << e.tagName() << " as string: " << endl << str << endl;
          if (state == "added")
          {
            // convert xml to PVDataEntry
            entry->fromXML(str.latin1());
            // write entry to PV
            uid = casioPV->AddEntry(*entry);
          }
          else if (state == "modified")
          {
            // xxx noch hinzufügen -> datensatz ändern
          }
          else if (state == "removed")
          {
            // xxx noch hinzufügen -> datensatz löschen
          }

          n = n.nextSibling(); // jump to next entry
        }  // while(!n.isNull())
      }
      else
      {
        kdDebug() << "Category " << e.tagName() << " is not yet implemented!" << endl;
      }
      n = n.nextSibling(); // jump to next category
    } // while(!n.isNull())
  } // try
  catch (CasioPVException e)
  {
    kdDebug() << "CasioPVException catched" << endl;
    // xxx fehlerbehandlung
  }
}
