/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNC_QTOPIAKONNECTOR_H
#define KSYNC_QTOPIAKONNECTOR_H

#include <qiconset.h>
#include <qptrlist.h>

#include <konnector.h>

namespace KSync {

class QtopiaKonnector : public Konnector
{
    Q_OBJECT
  public:
    QtopiaKonnector( const KConfig * );
    ~QtopiaKonnector();

    void writeConfig( KConfig *cfg );

    SynceeList syncees();

    bool readSyncees();
    bool writeSyncees();

    bool connectDevice();
    bool disconnectDevice();

    KonnectorInfo info() const;
    virtual QStringList supportedFilterTypes() const;

    void setDestinationIP( const QString &IP ) { mDestinationIP = IP; }
    QString destinationIP() const { return mDestinationIP; }

    void setUserName( const QString &name ) { mUserName = name; }
    QString userName() const { return mUserName; }

    void setPassword( const QString &password ) { mPassword = password; }
    QString password() const { return mPassword; }

    void setModel( const QString &model ) { mModel = model; }
    QString model() const { return mModel; }

    void setModelName( const QString &name ) { mModelName = name; }
    QString modelName() const { return mModelName; }

    void appendSyncee( KSync::Syncee * );

  protected:
    QIconSet iconSet() const;
    QString iconName() const;

  private slots:
    void slotSync( SynceeList );

  private:
    QString mDestinationIP;
    QString mUserName;
    QString mPassword;
    QString mModel;
    QString mModelName;

    SynceeList mSynceeList;

    /* for compiling purposes */
    class Private;
    Private *d;
};

}

#endif
