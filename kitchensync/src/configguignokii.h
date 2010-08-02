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

#include <tqvaluelist.h>
#include <tqpair.h>

class TQLabel;
class TQLineEdit;
class TQComboBox;

class ConfigGuiGnokii : public ConfigGui
{
  Q_OBJECT

  public:
    ConfigGuiGnokii( const QSync::Member &, TQWidget *parent );

    void load( const TQString &xml );
    TQString save() const;

  private:
    TQComboBox *mConnection;
    KComboBox *mPort;
    TQLabel    *mPortLabel;
    KComboBox *mModel;

    BluetoothWidget *mBluetooth;

    typedef QPair<TQString, TQString> ConnectionType;
    typedef TQValueList<ConnectionType> ConnectionTypeList;
    ConnectionTypeList mConnectionTypes;

   protected slots:
     void slotConnectionChanged( int nth );
     void slotModelChanged();
    
};

#endif
