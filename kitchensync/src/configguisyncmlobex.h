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

#include <tqdom.h>

#include "configgui.h"
#include "connectionwidgets.h"

class QCheckBox;
class QComboBox;
class QGridLayout;
class QSpinBox;
class KComboBox;
class KLineEdit;
class KURLRequester;

class ConfigGuiSyncmlObex : public ConfigGui
{
  Q_OBJECT

  public:
    ConfigGuiSyncmlObex( const QSync::Member &, TQWidget *parent = 0 );

    void load( const TQString &xml );
    TQString save() const;

  public slots:
     void slotConnectionChanged( int pos );

  private:
    // Connection
    typedef QPair<int, TQString> ConnectionType;
    typedef TQValueList<ConnectionType> ConnectionTypeList;
    ConnectionTypeList mConnectionTypes;

    TQComboBox *mConnection;
    BluetoothWidget *mBluetooth;
    UsbWidget *mUsb;

    // Options
    typedef QPair<int, TQString> SyncmlVersion;
    typedef TQValueList<SyncmlVersion> SyncmlVersionList;
    SyncmlVersionList mSyncmlVersions;

    TQStringList mIdentiferList;
    KLineEdit *mUsername;
    KLineEdit *mPassword;
    TQCheckBox *mUseStringTable;
    TQCheckBox *mOnlyReplace;
    TQSpinBox *mRecvLimit;
    TQSpinBox *mMaxObjSize;
    TQComboBox *mSyncmlVersion;
    KComboBox *mIdentifier;
    TQCheckBox *mWbxml;

    TQGridLayout *mGridLayout;

    KComboBox *mContactDb;
    KComboBox *mCalendarDb;
    KComboBox *mNoteDb;

  protected slots:
    void addLineEdit( TQWidget *parent, const TQString &text, KComboBox **edit, int row );
};

#endif
