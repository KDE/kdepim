/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSYNC_KONNECTOR_H
#define KSYNC_KONNECTOR_H

#include <qcstring.h>
#include <qiconset.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qptrlist.h>

#include <syncer.h>

#include "stderror.h"
#include "stdprogress.h"
#include "koperations.h"

namespace KSync {

class KonnectorInfo;
class Kapabilities;
class ConfigWidget;

/**
 *  The base class of all plugins. The functions are
 *  the same as in konnector
 */
class Konnector : public QObject
{
    Q_OBJECT
  public:
    Konnector( QObject *obj, const char *name,
               const QStringList &args = QStringList() );
    virtual ~Konnector();

    virtual Kapabilities capabilities() = 0;
    virtual void setCapabilities( const Kapabilities &kaps ) = 0;
    virtual bool startSync() = 0;
    virtual bool startRestore( const QString &path ) = 0;
    virtual bool startBackup( const QString &path ) = 0;
    virtual bool connectDevice() = 0;
    virtual bool disconnectDevice() = 0;

    virtual KonnectorInfo info() const = 0;

    virtual ConfigWidget *configWidget( const Kapabilities &, QWidget *parent,
                                        const char *name = 0 );
    virtual ConfigWidget *configWidget( QWidget *parent, const char *name = 0 );

    virtual void add( const QString &res );
    virtual void remove( const QString &res );
    virtual QStringList resources() const;

    /**
     * can be a file, a resource, a Syncee...
     */
    virtual void download( const QString &resource ) = 0;

    /**
     * the Syncees that are supported builtIn
     */
    virtual QStringList builtIn() const;
    bool isConnected() const;
    void doWrite( Syncee::PtrList );

  protected:
    void progress( const Progress & );
    void error( const Error & );

    //virtual QString metaId() const = 0;

//  public:
//    virtual void slotWrite( const QString &, const QByteArray & ) = 0;

  protected:
    virtual void write(Syncee::PtrList ) = 0;
//    virtual void slotWrite(KOperations::ValueList ) = 0;

  signals:
    void sync( Konnector *,  Syncee::PtrList );
    void sig_progress( Konnector *, const Progress & );
    void sig_error( Konnector *, const Error & );
    void sig_downloaded( Konnector *, Syncee::PtrList );

  private:
    QStringList m_resources;
    bool m_isCon : 1;
};

}

#endif
