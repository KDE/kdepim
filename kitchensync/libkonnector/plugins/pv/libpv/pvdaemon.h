/***************************************************************************
                          pvdaemon.h  -  description
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

#ifndef PVDAEMON_H
#define PVDAEMON_H

#include <qmap.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qdom.h>

#include <casiopv.h>

#include "pvdaemoniface.h"


/** This class implements the PVDaemon. The daemon is used to communicate with
  * the PV. It handles
  * - Signals from and to PV Plugin of KitchenSync
  * - DCOP calls from the PV Plugin
  * - Conversion of entries (XML to PVDataEntry and vice versa)
  * @author Maurus Erni
  */

namespace CasioPV {

  class pvDaemon : virtual public PVDaemonIface
  {
    public:
      /**
         * Constructor.
         */
      pvDaemon();

      /**
         * Destructor.
         */
      ~pvDaemon();


      // ---------------------- Public DCOP interface ---------------------- //

      /**
         * Belongs to the DCOP interface of the PV Daemon.
         * This method is called when the PV has to be connected.
         * @param port The port where the PV is connected to (e.g. /dev/ttyS0)
         * @return QString The parameters of the connected PV (mode code,
         * optional code, secret area)
         */
      QStringList connectPV(const QString& port);

      /**
         * Belongs to the DCOP interface of the PV Daemon.
         * This method is called when the PV has to be disconnected.
         * @return bool Successful disconnecting of PV (yes / no)
         */
      bool disconnectPV(void);

      /**
         * Belongs to the DCOP interface of the PV Daemon.
         * This method is called to get the changes on the PV since the last
         * synchronization.
         * @param categories List of categories to fetch data
         */
      void getChanges(const QStringList& categories);

      /**
         * Belongs to the DCOP interface of the PV Daemon.
         * This method is called to get all data from the PV.
         * @param categories List of categories to fetch data
         */
      void getAllEntries(const QStringList& categories);

      /**
         * Belongs to the DCOP interface of the PV Daemon.
         * This method is called when the changes after synchronization have
         * to be written to the PV.
         * @param optionalCode The optional code to be stored on the PV
         * @param array The changed data as QByteArray
         */
      void setChanges(const QString& optionalCode, const QByteArray& array);

      /**
         * Belongs to the DCOP interface of the PV Daemon.
         * This method is called when all entries have to be written to the
         * PV (restore, first synchronization).
         * @param array The changed data as QByteArray
         */
      void setAllEntries(const QByteArray& array);

      /**
         * Belongs to the DCOP interface of the PV Daemon.
         * This method can be used to check whether the PV is connected
         * @return bool PV connected (true / false)
         */
      bool isConnected(void);


    private:

      /**
         * Constructs a new empty entry derived from PVDataEntry. The category
         * and the uid are used in the constructor.
         * @param category The category of the entry
         * @param uid The uid of the entry
         * @return PVDataEntry* The new empty PVDataEntry
         */
      PVDataEntry* ClearEntry(unsigned int category, unsigned int uid);

      /**
         * Gets all entries of the specified category from the PV.
         * @param category The category where the entries are fetched from
         * @return QString The entries as XML string
         */
      QString getAllEntriesFromPV(unsigned int category);

      /**
         * Gets the changes of the specified category from the PV.
         * @param category The category where the entries are fetched from
         * @return QString The entries as XML string
         */
      QString getChangesFromPV(unsigned int category);

      /**
         * Writes the entries included in a DOM node to the PV.
         * @param n The dome node which holds the entries
         * @param ignoreState If set, the state of the entries are ignored and
         * all entries are written to the PV
         */
      void writeEntries(QDomNode& n, bool ignoreState=false);

      /**
         * Sends an exception as a DCOP call to the PV Plugin.
         * @param msg The error message
         * @param number The error number
         * @param disconnected Specifies whether the connection to the PV has
         * to be released due to this error
         */
      void sendException(const QString& msg, const unsigned int number, const bool disconnect = true);

      /**
         * CasioPV* The CasioPV object
         */
      CasioPV* casioPV;
  };
};  // namespace

#endif
