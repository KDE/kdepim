/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef _KSERIALDEVICEEMULATORWIDGET_H_
#define _KSERIALDEVICEEMULATORWIDGET_H_

#include "ui_kserialdeviceemulatorwidgetbase.h"
#include "commandslist.h"
#include <k3listview.h>
class CommandListViewItem : public K3ListViewItem
{
    public:
        CommandListViewItem(K3ListView *parent, const Command &command);
        Command command() { return cmd; }
    private:
        Command cmd;
};

class KSerialDeviceEmulatorWidget : public QWidget, private Ui::KSerialDeviceEmulatorWidgetBase
{
    Q_OBJECT

public:
    KSerialDeviceEmulatorWidget(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~KSerialDeviceEmulatorWidget();
    /*$PUBLIC_FUNCTIONS$*/

public slots:
    /*$PUBLIC_SLOTS$*/
    void addToLog(const QString &text, const QString &color);
    void loadClicked();
    void updateCommandListView();
    void removeCmd();
    void commandClicked ( Q3ListViewItem * item );
    void removeAllCmds();
    void resetEvent();
    void slotSendEvent();
    void eventSelected(int);
protected:
    /*$PROTECTED_FUNCTIONS$*/

protected slots:
    /*$PROTECTED_SLOTS$*/
    void slotSendEvent(const QString &);
    signals:
        void loadFile(const QString &);
    void sendEvent(const QString &);

};

#endif

