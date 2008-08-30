/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2006 Daniel Gollub <dgollub@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/
#ifndef CONFIGGUISYNCMLOBEX_H
#define CONFIGGUISYNCMLOBEX_H

#include <QtXml/QtXml>

#include "configgui.h"
#include "connectionwidgets.h"

class QCheckBox;
class QComboBox;
class QGridLayout;
class QSpinBox;
class KComboBox;
class KLineEdit;

class ConfigGuiSyncmlObex : public ConfigGui
{
  Q_OBJECT

  public:
    explicit ConfigGuiSyncmlObex( const QSync::Member &, QWidget *parent = 0 );

    void load( const QString &xml );
    QString save() const;

  public Q_SLOTS:
     void slotConnectionChanged( int pos );

  private:
    // Connection
    typedef QPair<int, QString> ConnectionType;
    typedef QList<ConnectionType> ConnectionTypeList;
    ConnectionTypeList mConnectionTypes;

    QComboBox *mConnection;
    BluetoothWidget *mBluetooth;
    UsbWidget *mUsb;

    // Options
    typedef QPair<int, QString> SyncmlVersion;
    typedef QList<SyncmlVersion> SyncmlVersionList;
    SyncmlVersionList mSyncmlVersions;

    QStringList mIdentiferList;
    KLineEdit *mUsername;
    KLineEdit *mPassword;
    QCheckBox *mUseStringTable;
    QCheckBox *mOnlyReplace;
    QCheckBox *mOnlyLocalTime;
    QSpinBox *mRecvLimit;
    QSpinBox *mMaxObjSize;
    QComboBox *mSyncmlVersion;
    KComboBox *mIdentifier;
    QCheckBox *mWbxml;

    QGridLayout *mGridLayout;

    KComboBox *mContactDb;
    KComboBox *mCalendarDb;
    KComboBox *mNoteDb;

  protected Q_SLOTS:
    void addLineEdit( QWidget *parent, const QString &text, KComboBox **edit, int row );
};

#endif
