/* This file is part of the KDE libraries
    Copyright (C) 2002, 2003 Holger Freyther <freyher@kde.org>

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

class KonnectorInfo;
class Kapabilities;
class ConfigWidget;
typedef QString UDI;
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
    virtual bool connect() = 0;
    virtual void disconnect() = 0;

    virtual KonnectorInfo info()const = 0;

    virtual ConfigWidget* configWidget( const Kapabilities&, QWidget* parent, const char* name ) = 0;
    virtual ConfigWidget* configWidget( QWidget* parent, const char* name ) = 0;

    virtual void add( const QString& res  );
    virtual void download( const QString& resource ) = 0;

protected:
    QString m_adds;
    bool isConnected()const;
    void progress( const Progress& );
    void error( const Error& );

    //virtual QString metaId()const = 0;
public slots:
    virtual void slotWrite(const QString &, const QByteArray & ) = 0;
    virtual void slotWrite(Syncee::PtrList ) = 0;
    virtual void slotWrite(KOperations::ValueList ) = 0;
signals:
    void sync(const UDI&,  Syncee::PtrList );
    void sig_progress(const UDI&, Progress );
    void sig_error( const UDI&, Error );
private:
    QString m_udi;
    bool m_isCon : 1;
};
}
#endif

