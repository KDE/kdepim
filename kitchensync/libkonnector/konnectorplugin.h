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
#include <qiconset.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qptrlist.h>

#include <syncer.h>

#include "koperations.h"

namespace KSync {

class Kapabilities;
class ConfigWidget;
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
    virtual void setUDI(const QString & ) ;
    virtual QString udi()const ;
    virtual Kapabilities capabilities( )  = 0 ;
    virtual void setCapabilities( const Kapabilities &kaps ) = 0;
    virtual bool startSync() = 0;
    virtual bool startRestore(const QString& path) = 0;
    virtual bool startBackup(const QString& path) = 0;
    virtual bool connectDevice() = 0;
    virtual void disconnectDevice() = 0;
    virtual bool isConnected() = 0;
    virtual bool insertFile(const QString &fileName ) = 0;
    virtual QByteArray retrFile(const QString &path ) = 0;
    virtual Syncee* retrEntry( const QString &path ) = 0;
    virtual QIconSet iconSet()const = 0;
    virtual QString  iconName()const =0;
    virtual QString id()const = 0;
    virtual QString metaId()const = 0;
    virtual ConfigWidget* configWidget( const Kapabilities&, QWidget* parent, const char* name ) = 0;
    virtual ConfigWidget* configWidget( QWidget* parent, const char* name ) = 0;
    //virtual QString metaId()const = 0;
public slots:
    virtual void slotWrite(const QString &, const QByteArray & ) = 0;
    virtual void slotWrite(Syncee::PtrList ) = 0;
    virtual void slotWrite(KOperations::ValueList ) = 0;
signals:
    void sync(const QString&,  Syncee::PtrList );
    void errorKonnector(const QString&, int, const QString& );
private:
    QString m_udi;
};
};
#endif


