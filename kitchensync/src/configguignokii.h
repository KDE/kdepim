/*
    This file is part of KitchenSync.

    Copyright (c) 2006 David Förster <david@dfoerster.de>

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
#ifndef CONFIGGUIGNOKII_H
#define CONFIGGUIGNOKII_H

#include "configgui.h"
#include "connectionwidgets.h"

#include <qvaluelist.h>
#include <qpair.h>

class QLabel;
class QLineEdit;
class QComboBox;

class ConfigGuiGnokii : public ConfigGui
{
  Q_OBJECT

  public:
    ConfigGuiGnokii( const QSync::Member &, QWidget *parent );

    void load( const QString &xml );
    QString save() const;

  private:
    QComboBox *mConnection;
    KComboBox *mPort;
    QLabel    *mPortLabel;
    KComboBox *mModel;

    BluetoothWidget *mBluetooth;

    typedef QPair<QString, QString> ConnectionType;
    typedef QValueList<ConnectionType> ConnectionTypeList;
    ConnectionTypeList mConnectionTypes;

   protected slots:
     void slotConnectionChanged( int nth );
     void slotModelChanged();
    
};

#endif
