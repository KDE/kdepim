/*
    This file is part of KitchenSync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef CONFIGCONNECTIONWIDGET_H
#define CONFIGCONNECTIONWIDGET_H

#include <QtGui/QWidget>

#include <libqopensync/pluginconnection.h>

class KLineEdit;
class QSpinBox;
class QStackedLayout;

class ConfigConnectionWidget : public QWidget
{
  Q_OBJECT

  public:
    ConfigConnectionWidget( const QSync::PluginConnection &localization, QWidget *parent = 0 );

    void load();
    void save();

  private Q_SLOTS:
    void typeChanged( int );

  private:
    QSync::PluginConnection mConnection;

    KComboBox *mType;

    QStackedLayout *mStack;
    QWidget *mBluetoothPage;
    QWidget *mUsbPage;
    QWidget *mNetworkPage;
    QWidget *mSerialPage;
    QWidget *mIrdaPage;

    KLineEdit *mBluetoothAddress;
    QSpinBox  *mBluetoothChannel;
    KLineEdit *mBluetoothSdpUuid;

    KLineEdit *mUsbVendorId;
    KLineEdit *mUsbProductId;
    QSpinBox *mUsbInterface;

    KLineEdit *mNetworkAddress;
    QSpinBox *mNetworkPort;
    KLineEdit *mNetworkProtocol;
    KLineEdit *mNetworkDnsSd;

    KLineEdit *mSerialSpeed;
    KComboBox *mSerialDevice;

    KLineEdit *mIrdaService;
};

#endif
