/* This file is part of the KDE libraries
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef konnectorplugin_h
#define konnectorplugin_h

#include <qcstring.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <ksyncentry.h>
#include <qptrlist.h>
#include "koperations.h"

class Kapabilities;
class KSyncEntry;


/**
 *  The base class of all plugins. The functions are
 *  the same as in konnector
 */
class KonnectorPlugin : public QObject
{
Q_OBJECT
public:
    KonnectorPlugin(QObject *obj, const char *name, const QStringList &args=QStringList() );
    virtual ~KonnectorPlugin() = 0;
    virtual void setUDI(const QString & ) = 0;
    virtual QString udi()const = 0;
    virtual Kapabilities capabilities( )  = 0 ;
    virtual void setCapabilities( const Kapabilities &kaps ) = 0;
    virtual bool startSync() = 0;
    virtual bool isConnected() = 0;
    virtual bool insertFile(const QString &fileName ) = 0;
    virtual QByteArray retrFile(const QString &path ) = 0;
    virtual KSyncEntry* retrEntry( const QString &path ) = 0;
public slots:
    virtual void slotWrite(const QString &, const QByteArray & ) = 0;
    virtual void slotWrite(QPtrList<KSyncEntry> ) = 0;
    virtual void slotWrite(QValueList<KOperations> ) = 0;
signals:
    void sync(QString,  QPtrList<KSyncEntry> );
    void errorKonnector(QString, int, QString );
};
#endif


