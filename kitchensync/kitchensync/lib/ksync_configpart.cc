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

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qframe.h>
//#include <qspacer.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <klocale.h>

#include "ksync_configpart.h"

using namespace KitchenSync;

ConfigPart::ConfigPart(const Kapabilities &, QWidget *parent, const char *name )
  : QWidget( parent, name )
{



}
void ConfigPart::initialize(const Kapabilities &kaps ){
    m_mainLayout = new QGridLayout( this, 6, 3 );

    if (  kaps.supportsMetaSyncing() ) {
        m_ckbMetaSyncing = new QCheckBox(i18n( "Enable Metasyncing"),  this );
        m_mainLayout->addWidget(m_ckbMetaSyncing,  0,  0);
    }

    QLabel *lbl;
    if ( kaps.supportsPushSync() ) {
        lbl = new QLabel(i18n("You can push syncs to this device"), this );
    }else
        lbl = new QLabel(i18n("You need to start the synchronization from your device"),  this );
    m_mainLayout->addWidget(lbl,  1,  0 );

    if ( kaps.canAutoHandle() )
        m_lblAutoHandle = new QLabel( i18n("This konnector establishes a connection to the device"),  this );
    else
        m_lblAutoHandle = new QLabel( i18n("To function properly you need to establish a connection"), this );
    m_mainLayout->addWidget( m_lblAutoHandle,  2,  0 );
}

Kapabilities ConfigPart::capability()
{

}

void ConfigPart::init()
{
    m_ckbMetaSyncing = 0;
    m_mainLayout = 0;
    m_lblAutoHandle = 0;
    m_grpConnection = 0;
    m_conLayout = 0;
    m_conSpacer = 0;
    m_conSrcIp = 0;
    m_conDestIp = 0;
    m_conUser = 0;
    m_conPass = 0;
    m_conPort = 0;

    m_lblSrcIp = 0;
    m_lblDestIp = 0;
    m_lblUser = 0;
    m_lblPass = 0;
    m_lblPort = 0;

    m_grpModel = 0;
    m_grpLayout = 0;
    m_lblDevice = 0;
    m_lblConnection = 0;
    m_cmbDevice = 0;
    m_cmbConnection = 0;

    m_grpDevice = 0;
    m_devLay = 0;
    m_grpFetch = 0;
    m_fetchAdd = 0;

    m_fetchBrowse = 0;
    m_fetchRem = 0;
    m_view = 0;
}
