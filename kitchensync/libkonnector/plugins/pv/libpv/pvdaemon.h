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

namespace CasioPV {

  class pvDaemon : virtual public PVDaemonIface
  {
    public:
      pvDaemon();
      ~pvDaemon();

      // DCOP interface
      QStringList connectPV(const QString& port);

      bool disconnectPV(void);

      void getChanges(const QStringList& categories);

      void getAllEntries(const QStringList& categories);

      void setChanges(const QString& optionalCode, const QByteArray& array);

      void setAllEntries(const QByteArray& array);

      bool isConnected(void);

    private:
      unsigned int getCategory(const QString& strCategory);
      PVDataEntry* ClearEntry(unsigned int category, unsigned int uid);
      QString getAllEntriesFromPV(unsigned int category);
      QString getChangesFromPV(unsigned int category);
      void writeEntries(QDomNode& n);
      
      void sendException(const QString& msg, const unsigned int number, const bool disconnect = true);

      QMap<QString, unsigned int> m_mapCategories;

      CasioPV* casioPV;
  };
};  // namespace

#endif
