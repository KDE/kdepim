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

namespace KSync
{
  class AddressBookSyncee;
  class EventSyncee;
  class TodoSyncee;

  class CasioPVLink : public QObject, virtual public CasioPVLinkIface
  {
    Q_OBJECT
    public:
      CasioPVLink(QObject *obj, const char *name );
      ~CasioPVLink();
      bool startSync();
      bool startBackup(const QString& path);
      bool startRestore(const QString& path);
      bool isConnected();
      void setModel(const QString& model );
      void setConnectionMode(const QString& connectionMode );
      void setMetaSyncing(const bool meta);
      void write(Syncee::PtrList);
      QString metaId()const;

      // -------------------- Public DCOP interface -------------------- //
      void getChangesDone(const QByteArray& stream);
      void getAllEntriesDone(const QByteArray& stream);
      void setChangesDone(const bool ok);
      void setAllEntriesDone(const bool ok);
      void errorPV(const QString& msg, const unsigned int errorcode);

    private:
      bool callConnectPV();
      void callDisconnectPV();
      // Kapabilities
      QString m_udi;
      QString m_model;
      bool m_meta;
      QString m_connectionMode;
      // Actual properties of the connected device
      enum Status {DISCONNECTED, CONNECTED, SYNCING};
      enum Mode {NONE, SYNC, BACKUP};
      int m_state;
      int m_mode;
      QString m_backupPath;
      QString m_metaPath;
      bool m_firstSync;
      // Model data of connected PV
      QString m_modelCode;
      QString m_optionalCode;
      bool m_secretArea;
      QByteArray m_array; // Data to be stored to a file after updating PV

    signals:
        void sync( Syncee::PtrList );
        void errorKonnector(int, QString );
        void stateChanged( bool );
  };
};

#endif

