/***************************************************************************
                          pvdaemoniface.h  -  description
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

#ifndef PVDAEMONIFACE_H
#define PVDAEMONIFACE_H

#include <qstringlist.h>
#include <qstring.h>
#include <qcstring.h>

#include <dcopobject.h>

/** This class implements the public DCOP interface of the PV Library.
  * The DCOP calls are used to get signals from the PV Konnector (e.g. connect,
  * disconnect, get all data, ...).
  * @author Maurus Erni
  */

class PVDaemonIface : virtual public DCOPObject
{
  K_DCOP
  k_dcop:

    virtual QStringList connectPV(const QString& port) = 0;

    virtual bool disconnectPV() = 0;

    virtual ASYNC getChanges(const QStringList& categories) = 0;

    virtual ASYNC getAllEntries(const QStringList& categories) = 0;

    virtual ASYNC setChanges(const QString& optionalCode, const QByteArray& array) = 0;

    virtual ASYNC setAllEntries(const QByteArray& array) = 0;

    virtual bool isConnected() = 0;
};

#endif
