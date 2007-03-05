/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
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

#ifndef CONNECTIONWIDGETS_H
#define CONNECTIONWIDGETS_H

#include <qdom.h>

#include "configgui.h"

class QCheckBox;
class QLabel;
class QPushButton;
class QSpinBox;

class KComboBox;
class KLineEdit;

class BluetoothWidget : public QWidget
{
  Q_OBJECT

  public:
    BluetoothWidget( QWidget *parent );

    void hideChannel();
    void showChannel();

    void setAddress( const QString address );
    void setChannel( const QString  channel );
    QString address() const;
    QString channel() const;

  private:
    KLineEdit *mAddress;
    KLineEdit *mChannel;
    QLabel *mChannelLabel;
};

class IRWidget : public QWidget
{
  Q_OBJECT

  public:
    IRWidget( QWidget *parent );

    void load( const QDomElement& );
    void save( QDomDocument&, QDomElement& );

  private:
    KLineEdit *mDevice;
    KLineEdit *mSerialNumber;
};

class CableWidget : public QWidget
{
  public:
    CableWidget( QWidget *parent );

    void load( const QDomElement& );
    void save( QDomDocument&, QDomElement& );

  private:
    KComboBox *mManufacturer;
    KComboBox *mDevice;
};

class UsbWidget : public QWidget
{
  public:
    UsbWidget( QWidget *parent );

    int interface() const;
    void setInterface( int interface );

  private:
    QSpinBox *mInterface;
};

#endif // CONNECTIONWIDGETS_H
