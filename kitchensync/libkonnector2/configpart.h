/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <freyther@kde.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

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
#ifndef KSNYC_CONFIGPART_H
#define KSNYC_CONFIGPART_H

#include <qwidget.h>
#include <qmap.h>

#include "kapabilities.h"
#include "configwidget.h"

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
    /** @internal only*/
    class ConfigPart : public ConfigWidget{  // no real part yet
    public:
        /**
	 * src is a Kapabilities object determining the layout
	 * of the Widget
	 */
        ConfigPart(const Kapabilities &src, QWidget*, const char *name=0 );

	/**
	 * @param base the Base of the layout
	 * @param config The config object fills into the layout
	 */
	ConfigPart(const Kapabilities& base,
		   const Kapabilities& config,
		   QWidget*, const char* name );
        ~ConfigPart();
        void setCapabilities(const Kapabilities & );
        Kapabilities capabilities()const; // return the modified capability
    private:
        void init();
        void initialize(const Kapabilities & );
        void apply( const Kapabilities& );
        class ConfigPartPrivate;
        ConfigPartPrivate *d;
        Kapabilities m_kap;
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

        QLabel* m_lblName;
        QLineEdit* m_lneName;
        bool push:1;
    };
}

#endif
