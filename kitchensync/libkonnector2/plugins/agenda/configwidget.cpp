#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <kapplication.h>
#include <klocale.h>

#include "configwidget.h"

using namespace Vr3;

namespace {
    void setCurrent( const QString& str, QComboBox* box, bool insert = true ) {
        if (str.isEmpty() ) return;
        uint b = box->count();
        for ( uint i = 0; i < b; i++ ) {
            if ( box->text(i) == str ) {
                box->setCurrentItem(i );
                return;
            }
        }
        if (!insert ) return;

        box->insertItem( str );
        box->setCurrentItem( b );
    }
}

ConfigWidget::ConfigWidget( const KSync::Kapabilities& cap,
                            QWidget* parent, const char* name )
    : KSync::ConfigWidget( cap, parent, name ) {
    initUI();
    setCapabilities( cap );
}
ConfigWidget::ConfigWidget( QWidget* parent, const char* name )
    : KSync::ConfigWidget( parent, name ){
    initUI();
}
ConfigWidget::~ConfigWidget() {
}
void ConfigWidget::initUI() {
    m_lay = new QGridLayout( this, 2, 5 );
    m_lay->addColSpacing( 3, 20 );


    QLabel* label = new QLabel(this);
    label->setText("<qt><h1>Agenda Vr3 Konnector</h1></qt>");

    m_lblIP = new QLabel( this );
    m_lblIP->setText(i18n("IP Address:") );
    m_cmbIP = new QComboBox( this );
    m_cmbIP->setEditable(true);

    m_lblName = new QLabel( this );
    m_lblName->setText(i18n("Name:") );
    m_lneName = new QLineEdit( this );

    m_lay->addMultiCellWidget( label, 0, 0, 0, 2, AlignLeft );

    m_lay->addWidget( m_lblIP, 1, 0 );
    m_lay->addWidget( m_cmbIP, 1, 1 );

    m_lay->addWidget( m_lblName, 1, 3 );
    m_lay->addWidget( m_lneName, 1, 4 );
}

KSync::Kapabilities ConfigWidget::capabilities()const {
    KSync::Kapabilities caps;

    caps.setSupportMetaSyncing( true ); // we can meta sync
    caps.setSupportsPushSync( true ); // we can initialize the sync from here
    caps.setNeedsConnection( true ); // we need to have pppd running
    caps.setSupportsListDir( true ); // we will support that once there is API for it...
    caps.setNeedsIPs( true ); // we need the IP
    caps.setNeedsSrcIP( false ); // we do not bind to any address...
    caps.setNeedsDestIP( true ); // we need to know where to connect
    caps.setAutoHandle( false ); // we currently do not support auto handling
    caps.setNeedAuthentication( false ); // HennevL says we do not need that
    caps.setNeedsModelName( true ); // we need a name for our meta path!

    caps.setDestIP( m_cmbIP->currentText() );
    caps.setModelName( name() );

    return caps;
}

void ConfigWidget::setCapabilities( const KSync::Kapabilities& caps) {
    setCurrent( caps.destIP(), m_cmbIP );
    m_lneName->setText( caps.modelName() );
    m_lneName->setEnabled( false ); // needed for the Meta dir path!!!
}

QString ConfigWidget::name()const {
    return m_lneName->text().isEmpty() ? "AgendaVr3" + kapp->randomString(5) : m_lneName->text();
}


#include "configwidget.moc"
