// DELETE THIS FILE!
/*
† † † †This file is part of the OPIE Project
† † † †Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
† †                   2002 Maximilian Reiﬂ <harlekin@handhelds.org>
† † † † † †

† † † † † † † †=.
† † † † † † †.=l.
† † † † † †.>+-=
†_;:, † † .> † †:=|.         This program is free software; you can
.> <`_, † > †. † <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- † :           the terms of the GNU General Public
.="- .-=="i, † † .._         License as published by the Free Software
†- . † .-<_> † † .<>         Foundation; either version 2 of the License,
† † †._= =} † † † :          or (at your option) any later version.
† † .%`+i> † † † _;_.
† † .i_,=:_. † † †-<s.       This program is distributed in the hope that
† † †+ †. †-:. † † † =       it will be useful,  but WITHOUT ANY WARRANTY;
† † : .. † †.:, † † . . .    without even the implied warranty of
† † =_ † † † †+ † † =;=|`    MERCHANTABILITY or FITNESS FOR A
† _.=:. † † † : † †:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= † † † = † † † ;      Library General Public License for more
++= † -. † † .` † † .:       details.
†: † † = †...= . :.=-
†-. † .:....=;==+<;          You should have received a copy of the GNU
† -_. . . † )=. †=           Library General Public License along with
† † -- † † † †:-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/

#ifndef KSNYC_CONFIGPART_H
#define KSNYC_CONFIGPART_H 

#include <qwidget.h>
#include <qmap.h>

#include <kapabilities.h>


class QVBox;
class QHBox;
class QGridLayout;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QLabel;
class QFrame;
class QGroupBox;
class QSpacer;
class QSpacerItem;
class QPushButton;
class QListView;

namespace KSync {

    // responsible for configuring
    class ConfigPart : public QWidget{  // no real part yet
    public:
        ConfigPart(const Kapabilities &, QWidget*, const char *name=0 );
        void setCapability(const Kapabilities & );
        Kapabilities capability(); // return the modified capability
    private:
        void init();
        void initialize(const Kapabilities & );
        class ConfigPartPrivate;
        ConfigPartPrivate *d;
        QCheckBox *m_ckbMetaSyncing; // do meta syncing
        QGridLayout *m_mainLayout; // the Main Layout of the widget

        QLabel *m_lblAutoHandle; // auto Handle label


        QGroupBox *m_grpConnection; // The connection GroupBox
        QGridLayout *m_conLayout; // the Layout of the conBox
        //QSpacerItem *m_conSpacer; // the QSpacerItem
        QComboBox *m_conSrcIp; // SRC IP
        QComboBox *m_conDestIp; // DEST IP
        QComboBox *m_conUser; // COn USER;
        QComboBox *m_conPass; // con PASSWORD;
        QComboBox *m_conPort; // The con Port
        QLabel *m_lblSrcIp;
        QLabel *m_lblDestIp;
        QLabel *m_lblUser;
        QLabel *m_lblPass;
        QLabel *m_lblPort;


        QGroupBox *m_grpModel; // The Model configuration maybe QVGroupBox
        QGridLayout *m_grpLayout;
        QLabel *m_lblDevice;
        QLabel *m_lblConnection;
        QLabel *m_grpUser;
        QLabel *m_grpPass;
        QComboBox *m_cmbPass;
        QComboBox *m_cmbUser;
        QComboBox *m_cmbDevice;
        QComboBox *m_cmbConnection;
        // user and pass?

        QGroupBox *m_grpDevice; // Device specific options
        QGridLayout *m_devLay;
        QMap<QString,  QLineEdit*> m_devGroup;

        QGroupBox *m_grpFetch;
        QPushButton* m_fetchAdd;
        QPushButton* m_fetchBrowse;
        QPushButton* m_fetchRem;
        QListView *m_view;
        bool push:1;
    };
};

#endif
