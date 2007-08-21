/*
    This file is part of KitchenSync.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

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
#ifndef CONFIGGUIOPIE_H
#define CONFIGGUIOPIE_H

#include "configgui.h"

class QComboBox;
class QLineEdit;
class QSpinBox;

class ConfigGuiOpie : public ConfigGui
{
  public:
    ConfigGuiOpie( const QSync::Member &, QWidget *parent );

    void load( const QString &xml );
    QString save() const;

  private:
    QLineEdit *mDeviceIP;
    QComboBox *mDeviceType;
    QLineEdit *mUserName;
    QLineEdit *mPassword;
    QComboBox *mConnectionType;
    QSpinBox *mPort;
};

#endif
