/***************************************************************************
                        casiopvlink.h  -  description
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

#ifndef casiopvlink_h
#define casiopvlink_h

#include <qcstring.h>
#include <qbuffer.h>
#include <qobject.h>

#include <konnector.h>

#include "casiopvlinkiface.h"


/** This class is the link between the PVKonnector and the library. It handles
  * - Signals from and to KitchenSync
  * - DCOP calls from the PV Library
  * - Conversion of entries (XML to Syncee and vice versa)
  * @author Maurus Erni
  */

namespace KSync
{
  class AddressBookSyncee;
  class EventSyncee;
  class TodoSyncee;

  class CasioPVLink : public QObject, virtual public CasioPVLinkIface
  {
    Q_OBJECT
    public:
      /**
         * Constructor.
         * @param obj The parent object
         * @param name The name of the object
         */
      CasioPVLink(QObject *obj, const char *name );

      /**
         * Destructor.
         */
      ~CasioPVLink();

      /**
         * Signals that the synchronization has to be started.
         * @return bool Starting of synchronization successful (yes / no)
         */
      bool startSync();

      /**
         * Signals that the backup has to be started.
         * @param path The path of the backup file
         * @return bool Starting of backup successful (yes / no)
         */
      bool startBackup(const QString& path);

      /**
         * Signals that the restore has to be started.
         * @param path The path of the restore file
         * @return bool Starting of restore successful (yes / no)
         */
      bool startRestore(const QString& path);

      /**
         * Sets the PV model to be synchronized depending on kapabilities
         * chosen in configuration
         * @param model The PV model to be synchronized
         */
      void setModel(const QString& model );

      /**
         * Sets the connection mode depending on kapabilities chosen
         * in configuration.
         * @param connectionMode The connection mode (/dev/ttyS0 or /dev/ttyS1)
         */
      void setConnectionMode(const QString& connectionMode );

      /**
         * Sets the meta syncing depending on kapabilities chosen in
         * configuration. Meta syncing can be enabled or disabled.
         * @param meta Meta synchronization enabled (true) or disabled (false)
         */
      void setMetaSyncing(const bool meta);

      /**
         * Returns whether the PV is connected
         * @return bool PV connected (yes / no)
         */
      bool isConnected();

      /**
         * Signals that the synchronization was done and the synchronized data
         * can be written to the PV.
         * @param lis A Syncee::PtrList of the synchronized date
         */
      void write(Syncee::PtrList lis);

      /**
         * Returns which PV model is connected to the PC.
         * @return QString The PV model connected to the PC
         */
      QString metaId()const;


      // ---------------------- Public DCOP interface ---------------------- //

      /**
         * Belongs to the DCOP interface of the PV Plugin.
         * This method is called when all changes are fetched from the PV.
         * @param stream The changes from PV to be synchronized.
         */
      void getChangesDone(const QByteArray& stream);

      /**
         * Belongs to the DCOP interface of the PV Plugin.
         * This method is called when all data is fetched from the PV.
         * @param stream The data from PV to be synchronized.
         */
      void getAllEntriesDone(const QByteArray& stream);

      /**
         * Belongs to the DCOP interface of the PV Plugin.
         * This method is called when all changes were written to the PV
         * after synchronization.
         * @param ok Writing successful (yes / no)
         */
      void setChangesDone(const bool ok);

      /**
         * Belongs to the DCOP interface of the PV Plugin.
         * This method is called when all data was written to the PV
         * after synchronization.
         * @param ok Writing successful (yes / no)
         */
      void setAllEntriesDone(const bool ok);

      /**
         * Belongs to the DCOP interface of the PV Plugin.
         * This method is called when an error occurred in the PV Library
         * @param msg The error message to be displayed
         * @param errorcode The error number of the exception
         */
      void errorPV(const QString& msg, const unsigned int errorcode);


    private:
      /**
         * Makes a DCOP call to the PV Library to connect the PV.
         * @return bool DCOP call successful (yes / no)
         */
      bool callConnectPV();

      /**
         * Makes a DCOP call to the PV Library to disconnect the PV.
         * @return bool DCOP call successful (yes / no)
         */
      void callDisconnectPV();


      // ---------------------- Konnector attributes ----------------------- //

      /**
         * Holds the connected PV model
         */
      QString m_model;

      /**
         * Meta synchronization enabled?
         */
      bool m_meta;

      /**
         * Holds the connection mode (e.g. /dev/ttyS0)
         */
      QString m_connectionMode;


      // ----------- Actual properties of the connected device ------------- //

      /**
         * enum Enumeration with the possible states of the
         * connection to the PV
         */
      enum Status {DISCONNECTED, CONNECTED, SYNCING};

      /**
         * enum Enumeration with the possible processing modes
         */
      enum Mode {NONE, SYNC, BACKUP};

      /**
         * int The state of the connection to the PV
         */
      int m_state;

      /**
         * int The actual processing mode
         */
      int m_mode;

      /**
         * QString The path of the backup file
         */
      QString m_backupPath;

      /**
         * QString The path of the meta data file
         */
      QString m_metaPath;

      /**
         * bool First synchronization? This flag is set, when the connected
         * PV was never synchronized before
         */
      bool m_firstSync;


      // ------------------- Model data of connected PV -------------------- //

      /**
         * QString The model code of the PV to be connected
         */
      QString m_modelCode;

      /**
         * QString The optional code of the PV to be connected
         */
      QString m_optionalCode;

      /**
         * bool Is the connected PV in secret area?
         */
      bool m_secretArea;

      /**
         * QByteArray Data to be stored to a file after updating PV
         */
      QByteArray m_array;


      // --------------------- Signals to be emitted ----------------------- //

    signals:
      /**
         * Signals that all data of the PV are available and the
         * synchronization process can be started
         * @param lis The data of the PV as a Syncee::PtrList
         */
        void sync(Syncee::PtrList lis);

      /**
         * Signals that an error occurred on the PV
         * @param number The error number
         * @param number The error message
         */
        void errorKonnector(int number, QString msg);

      /**
         * Signals that an error occurred on the PV
         * @param state The state of the PV (connected / disconnected)
         */
        void stateChanged(bool state);
  };
};

#endif

