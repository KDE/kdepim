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

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
//#include <qspacer.h>
#include <qcombobox.h>

#include <kapplication.h>
#include <klocale.h>

#include "configpart.h"

using namespace KSync;

namespace {
    void setCurrent( const QString& str, QComboBox* box ) {
        if (str.isEmpty() ) return;
        uint b = box->count();
        for ( uint i = 0; i < b; i++ ) {
            if ( box->text(i) == str ) {
                box->setCurrentItem(i );
                return;
            }
        }
        box->insertItem( str );
        box->setCurrentItem( b );
    }
}

ConfigPart::ConfigPart(const Kapabilities &kaps, QWidget *parent, const char *name )
  : ConfigWidget( parent, name )
{
    init();
    initialize( kaps );
    m_kap = kaps;
}
ConfigPart::ConfigPart(const Kapabilities& kaps, const Kapabilities &src,
		       QWidget* parent, const char* name )
    : ConfigWidget( parent, name ) {
    init();
    initialize( kaps );
    apply( src );
    m_kap = kaps;
}
ConfigPart::~ConfigPart() {
}
void ConfigPart::setCapabilities( const Kapabilities& caps) {
    apply( caps );
}
void ConfigPart::initialize(const Kapabilities &kaps ){
//    kaps.dump();
    m_mainLayout = new QGridLayout( this, 6, 3 );

    if (  kaps.supportsMetaSyncing() ) {
        m_ckbMetaSyncing = new QCheckBox(i18n( "Enable metasyncing"),  this );
        m_mainLayout->addWidget(m_ckbMetaSyncing,  0,  0);
        m_ckbMetaSyncing->setChecked( kaps.isMetaSyncingEnabled() );
    }

    QLabel *lbl;
    push = false;
    if ( kaps.supportsPushSync() ) {
        push = true;
        lbl = new QLabel(i18n("You can push syncs to this device"), this );
    }else
        lbl = new QLabel(i18n("You need to start the synchronization from your device"),  this );
    m_mainLayout->addWidget(lbl,  1,  0 );

    if ( kaps.canAutoHandle() )
        m_lblAutoHandle = new QLabel( i18n("This konnector establishes a connection to the device"),  this );
    else
        m_lblAutoHandle = new QLabel( i18n("To function properly you need to establish a connection"), this );
    m_mainLayout->addWidget( m_lblAutoHandle,  2,  0 );

    // Connection
    // column 0 = Label, 1= ComboBox, 2 = Space, 3 = Label, 4 = Combo
    m_grpConnection = new QGroupBox( i18n("Connection"),  this );
    m_conLayout = new QGridLayout(m_grpConnection,  4,  5);

    if ( !kaps.needsNetworkConnection() || kaps.canAutoHandle() ) {

        m_grpConnection->setEnabled( false );
    }else{
        m_grpConnection->setEnabled( true );
    }

    m_conLayout->setMargin( 12 );
    QSpacerItem *iti1b = new QSpacerItem(2, 10, QSizePolicy::Fixed,
                                         QSizePolicy::Fixed );
    m_conLayout->addItem( iti1b,  0, 0);

    // Source
    m_lblSrcIp = new QLabel(i18n("Source address: "),  m_grpConnection);
    m_conSrcIp = new QComboBox(m_grpConnection);
    m_conSrcIp->setEditable( TRUE );
    m_lblSrcIp->setBuddy( m_conSrcIp );
    m_conLayout->addWidget(m_lblSrcIp, 1, 0 );
    m_conLayout->addWidget(m_conSrcIp, 1, 1 );
    if (!kaps.needsIPs() || !kaps.needsSrcIP() ) {
        m_lblSrcIp->setEnabled( false );
        m_conSrcIp->setEnabled( false );
    }else{
        m_conSrcIp->insertItem( kaps.srcIP() );
    }

    //Destination
    m_lblDestIp = new QLabel(i18n("Destination address: "),  m_grpConnection );
    m_conDestIp = new QComboBox(m_grpConnection);
    m_conDestIp->setEditable( TRUE );
    m_lblDestIp->setBuddy( m_conDestIp );
    m_conLayout->addWidget( m_lblDestIp, 1, 3 );
    m_conLayout->addWidget( m_conDestIp, 1, 4 );
    if (!kaps.needsIPs() || !kaps.needsDestIP() ) {
        m_lblDestIp->setEnabled( false );
        m_conDestIp->setEnabled( false );
    }else{
        QStringList ips = kaps.ipProposals();
        QStringList::ConstIterator it;
        for ( it = ips.begin(); it != ips.end(); ++it ) {
            m_conDestIp->insertItem( (*it) );
        }
        //m_conDestIp->insertItem(kaps.destIP(),  0 );
    }
    //user
    m_lblUser = new QLabel(i18n("User:"), m_grpConnection );
    m_conUser = new QComboBox(m_grpConnection );
    m_conUser->setEditable( TRUE );
    m_lblUser->setBuddy( m_conUser );
    m_conLayout->addWidget( m_lblUser,  2,  0 );
    m_conLayout->addWidget( m_conUser,  2,  1 );

    //pass
    m_lblPass = new QLabel(i18n("Password:"),  m_grpConnection );
    m_conPass = new QComboBox( m_grpConnection );
    m_conPass->setEditable( TRUE );
    m_lblPass->setBuddy( m_conPass );
    m_conLayout->addWidget( m_lblPass,  2,  3 );
    m_conLayout->addWidget( m_conPass,  2,  4 );

    if ( kaps.needAuthentication() ) {
        QValueList<QPair<QString, QString> > list = kaps.userProposals();
        QValueList<QPair<QString, QString> >::ConstIterator it;
        for (it = list.begin(); it != list.end(); ++it ) {
            m_conUser->insertItem( (*it).first );
            m_conPass->insertItem( (*it).second);
        }
        m_conUser->insertItem(kaps.user(),  0);
        m_conPass->insertItem(kaps.password(),  0 );
    }else{
        m_lblPass->setEnabled( false );
        m_conPass->setEnabled( false );
        m_lblUser->setEnabled( false );
        m_conUser->setEnabled( false );
    }
    // port
    m_lblPort = new QLabel( i18n("Port:"),  m_grpConnection);
    m_conPort = new QComboBox( m_grpConnection );
    m_conPort->setEditable(TRUE);
    m_lblPort->setBuddy( m_conPort );
    m_conLayout->addWidget( m_lblPort,  3,  0 );
    m_conLayout->addWidget( m_conPort,  3,  1 );
    QMemArray<int> ints = kaps.ports();
    if ( ints.isEmpty() ) {
        m_lblPort->setEnabled( false );
        m_conPort->setEnabled( false );
    }else{
        for (uint i = 0; i < ints.size(); i++ ) {
            m_conPort->insertItem( QString::number( ints[i] ) );
        }
        //_conPort->insertItem( QString::number( kaps.currentPort() ),  0 );
    }

    // add the Connection Groupbox
    m_mainLayout->addWidget( m_grpConnection,  3,  0 );

    // Model specific
    m_grpModel = new QGroupBox( i18n("Model"),  this );
    m_grpLayout = new QGridLayout( m_grpModel, 6,  2 );

    m_grpLayout->setMargin( 12 );
    QSpacerItem *iti1c = new QSpacerItem(2, 10, QSizePolicy::Fixed,
                                         QSizePolicy::Fixed );
    m_grpLayout->addItem( iti1c,  0, 0);
    // Devices
    m_lblDevice = new QLabel( i18n("Device: "), m_grpModel );
    m_cmbDevice = new QComboBox( m_grpModel );
    m_cmbDevice->setEditable( false );
    m_lblDevice->setBuddy( m_cmbDevice );
    m_grpLayout->addWidget( m_lblDevice, 1,  0 );
    m_grpLayout->addWidget( m_cmbDevice, 1,  1 );
    QStringList devices = kaps.models();
    if ( devices.isEmpty() ) {
        m_lblDevice->setEnabled( false );
        m_cmbDevice->setEnabled( false );
    }else{
        for ( QStringList::ConstIterator it = devices.begin(); it != devices.end(); ++it ) {
            m_cmbDevice->insertItem( (*it) );
        }
        //m_cmbDevice->insertItem( kaps.currentModel() , 0);
    }
    // the device Name
    m_lblName = new QLabel( i18n("Name:"), m_grpModel );
    m_lneName = new QLineEdit(m_grpModel );
    m_lblName->setBuddy( m_lneName );
    m_lblName->setEnabled( kaps.needsModelName() );
    m_lneName->setEnabled( kaps.needsModelName() );
    m_grpLayout->addWidget( m_lblName, 2, 0 );
    m_grpLayout->addWidget( m_lneName, 2, 1 );


    // Connection Mode usb, paralell, net,....
    m_lblConnection = new QLabel( i18n("Connection:"),  m_grpModel );
    m_cmbConnection = new QComboBox( m_grpModel );
    m_cmbConnection->setEditable( TRUE );
    m_lblConnection->setBuddy( m_cmbConnection );
    m_grpLayout->addWidget( m_lblConnection, 3, 0 );
    m_grpLayout->addWidget( m_cmbConnection, 3, 1 );
    QStringList conList = kaps.connectionModes();
    if ( conList.isEmpty() ) {
        m_lblConnection->setEnabled( false );
        m_cmbConnection->setEnabled( false );
    }else{
        for ( QStringList::ConstIterator it = conList.begin(); it != conList.end(); ++it ) {
            m_cmbConnection->insertItem( (*it) );
        }
        //m_cmbConnection->insertItem( kaps.currentConnectionMode(), 0);

    }
    //Mode USER
    m_grpUser = new QLabel( i18n("User:"),  m_grpModel );
    m_cmbUser = new QComboBox( m_grpModel );
    m_cmbUser->setEditable( TRUE );
    m_grpUser->setBuddy( m_cmbUser );
    m_grpLayout->addWidget(m_grpUser,  4, 0);
    m_grpLayout->addWidget(m_cmbUser,  4, 1);
    m_grpUser->setEnabled( false );
    m_cmbUser->setEnabled( false );

    // MODE PASS
    m_grpPass = new QLabel( i18n("Pass:"),  m_grpModel );
    m_cmbPass = new QComboBox( m_grpModel );
    m_grpPass->setBuddy( m_cmbPass );
    m_cmbPass->setEditable( TRUE );
    m_grpLayout->addWidget( m_grpPass,  5, 0 );
    m_grpLayout->addWidget( m_cmbPass,  5, 1 );
    m_grpPass->setEnabled( false );
    m_cmbPass->setEnabled( false );

    m_mainLayout->addWidget( m_grpModel,  4,  0 );

    QMap<QString, QString> specs = kaps.extras();
    if ( !specs.isEmpty() ) {
        m_grpDevice = new QGroupBox( i18n("Device Specific"),  this );
        m_devLay = new QGridLayout(m_grpDevice, specs.count()+1, 2);
        QSpacerItem *iti1d = new QSpacerItem(4, 14, QSizePolicy::Fixed,
                                             QSizePolicy::Fixed );
        m_devLay->setMargin( 12 );
        m_devLay->addItem( iti1d,  0,  0 );
        int i = 0;
        QLabel *lbl;
        QLineEdit *edit;
        m_devGroup.clear();
        for ( QMap<QString,  QString>::ConstIterator it = specs.begin(); it != specs.end(); ++it ) {
            lbl = new QLabel(it.key() , m_grpDevice );
            edit = new QLineEdit(m_grpDevice,  it.key().latin1() );
            edit->setText( it.data() );
            lbl->setBuddy(edit);
            m_devGroup.insert(it.key(),  edit);

            m_devLay->addWidget( lbl,  i,  0 );
            m_devLay->addWidget( edit, i,  1 );
            ++i;
        }
        m_mainLayout->addWidget( m_grpDevice,  5,  0 );
    }
}
Kapabilities ConfigPart::capabilities()const
{
    Kapabilities kaps = m_kap;
    // ok first read all the extras which is fairly easy
    if ( !m_devGroup.isEmpty() ) {
        for ( QMap<QString, QLineEdit*>::ConstIterator it = m_devGroup.begin(); it != m_devGroup.end(); ++it ) {
            kaps.setExtraOption( it.key(), it.data()->text() );
        }
    }
    // meta syncing
    if ( m_ckbMetaSyncing != 0 ) {
        kaps.setMetaSyncingEnabled( m_ckbMetaSyncing->isChecked() );
    }
    // GRP IPs + User + Password
    if ( m_grpConnection != 0 && m_grpConnection->isEnabled() ) {
        if ( m_conSrcIp->isEnabled() )
            kaps.setSrcIP( m_conSrcIp->currentText() );
        if ( m_conDestIp->isEnabled( ) )
            kaps.setDestIP(m_conDestIp->currentText() );
        if ( m_conUser->isEnabled() )
            kaps.setUser( m_conUser->currentText() );
        if ( m_conPass->isEnabled() )
            kaps.setPassword( m_conPass->currentText() );
        if ( m_conPort->isEnabled() )
            kaps.setCurrentPort( m_conPort->currentText().toInt() );
    }
    if ( m_cmbPass != 0 && m_cmbPass->isEnabled() )
        ;
    if ( m_cmbUser != 0 && m_cmbUser->isEnabled() )
        ;
    if ( m_cmbDevice != 0 && m_cmbDevice->isEnabled() )
        kaps.setCurrentModel(m_cmbDevice->currentText() );
    if ( m_cmbConnection != 0 && m_cmbConnection->isEnabled() )
        kaps.setCurrentConnectionMode( m_cmbConnection->currentText() );

    /* model name */
    if ( m_lneName->isEnabled() ) {
        QString str = m_lneName->text().isEmpty() ? kapp->randomString(10): m_lneName->text();
        kaps.setModelName( str );
    }

    return kaps;
}

void ConfigPart::init()
{
    m_ckbMetaSyncing = 0;
    m_mainLayout = 0;
    m_lblAutoHandle = 0;
    m_grpConnection = 0;
    m_conLayout = 0;
//    m_conSpacer = 0;
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

    m_lneName =0;
    m_lblName =0;
}
/*
 * here we're going to apply the choices
 */
void ConfigPart::apply( const Kapabilities& caps ) {
    if (m_kap.needsIPs() || m_kap.needsSrcIP() ) {
        setCurrent( caps.srcIP(), m_conSrcIp );
    }
    if (m_kap.needsIPs() || m_kap.needsDestIP() ) {
        setCurrent( caps.destIP(), m_conDestIp );
    }
    if (m_kap.needAuthentication() ) {
        setCurrent( caps.user(), m_conUser );
        setCurrent( caps.password(), m_conPass );
    }
    if ( !m_kap.models().isEmpty() ) {
        setCurrent( caps.currentModel(), m_cmbDevice );
    }
    if ( m_kap.needsModelName() ) {
        m_lneName->setText( caps.modelName() );
    }
    if ( !m_kap.connectionModes().isEmpty() ) {
        setCurrent( caps.currentConnectionMode(), m_cmbConnection );
    }
    QMap<QString, QString> specs = caps.extras();
    for ( QMap<QString, QString>::ConstIterator it = specs.begin(); it != specs.end(); ++it ) {
        if (!m_devGroup.contains( it.key() ) ) continue;
        QLineEdit* edit = m_devGroup[it.key()];
        if (edit)
            edit->setText( it.data() );
    }
    if (!m_kap.ports().isEmpty() ) {
        setCurrent( QString::number( caps.currentPort() ), m_conPort );
    }
}
