#include <qcombobox.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>

#include "qtopiaconfig.h"

using KSync::Kapabilities;
using namespace OpieHelper;


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


QtopiaConfig::QtopiaConfig( const Kapabilities& cap, QWidget* parent, const char* name )
    : KSync::ConfigWidget( cap, parent, name ) {
    initUI();
    setCapabilities( cap );
}
QtopiaConfig::QtopiaConfig( QWidget* parent, const char* name )
    : KSync::ConfigWidget( parent, name ) {
    initUI();
}
Kapabilities QtopiaConfig::capabilities()const {
    Kapabilities caps;
    caps.setSupportMetaSyncing( true );
    caps.setSupportsPushSync( true );
    caps.setNeedsConnection( true );
    caps.setSupportsListDir( true );
    caps.setNeedsIPs( true );
    caps.setNeedsSrcIP( false );
    caps.setNeedsDestIP( true );
    caps.setAutoHandle( false );
    caps.setNeedAuthentication( true );
    caps.setNeedsModelName( true );
    caps.setMetaSyncingEnabled( true );

    caps.setDestIP( m_cmbIP->currentText() );
    caps.setUser( m_cmbUser->currentText() );
    caps.setPassword( m_cmbPass->currentText() );
    caps.setCurrentModel( m_cmbDev->currentText() );
    caps.setModelName( name() );

    return caps;
}
QString QtopiaConfig::name()const {
    return m_name->text().isEmpty() ? "Zaurus" + kapp->randomString(5 ) : m_name->text();
}
void QtopiaConfig::setCapabilities( const Kapabilities& caps ) {
    setCurrent( caps.user(), m_cmbUser );
    setCurrent( caps.password(), m_cmbPass );
    setCurrent( caps.destIP(), m_cmbIP );
    setCurrent( caps.currentModel(), m_cmbDev, false );
    if ( m_cmbDev->currentText() == QString::fromLatin1("Sharp Zaurus ROM") )
        m_name->setText( caps.modelName() );


    slotTextChanged( m_cmbDev->currentText() );
    m_name->setEnabled( false );
    m_cmbDev->setEnabled( false );
}
void QtopiaConfig::initUI() {
    m_layout = new QGridLayout( this, 4,5 );

    QLabel* label = new QLabel(this);
    label->setText("<qt><h1>Qtopia Konnector</h1></qt>");

    m_lblUser = new QLabel(this);
    m_lblUser->setText(i18n("User:"));
    m_cmbUser = new QComboBox(this);
    m_cmbUser->setEditable( true );
    m_cmbUser->insertItem( "root");

    m_lblPass = new QLabel(this);
    m_lblPass->setText(i18n("Password") );
    m_cmbPass = new QComboBox(this);
    m_cmbPass->setEditable( true );
    m_cmbPass->insertItem("Qtopia");

    m_lblName = new QLabel(this);
    m_lblName->setText(i18n("Name:"));
    m_name = new QLineEdit(this);
    m_name->setEnabled( false );

    m_lblIP = new QLabel( this );
    m_lblIP->setText(i18n("Destination Address:") );
    m_cmbIP = new QComboBox(this);
    m_cmbIP->setEditable( true );
    m_cmbIP->insertItem("1.1.1.1", 0);
    m_cmbIP->insertItem("192.168.129.201", 1);

    m_lblDev = new QLabel(this);
    m_lblDev->setText(i18n("Distribution") );
    m_cmbDev = new QComboBox(this);
    m_cmbDev->insertItem("Sharp Zaurus ROM");
    m_cmbDev->insertItem("Opie and Qtopia1.6", 0 );


    m_layout->addColSpacing( 3, 10 );
    m_layout->addMultiCellWidget( label, 0, 0, 0, 2, AlignLeft ); // Qtopia label top left stretching
    m_layout->addWidget( m_lblUser, 1, 0 );
    m_layout->addWidget( m_cmbUser, 1, 1 );

    m_layout->addWidget( m_lblPass, 1, 3 );
    m_layout->addWidget( m_cmbPass, 1, 4 );

    m_layout->addWidget( m_lblIP, 2, 0 );
    m_layout->addWidget( m_cmbIP, 2, 1 );

    m_layout->addWidget( m_lblName, 2, 3 );
    m_layout->addWidget( m_name, 2, 4 );

    m_layout->addWidget( m_lblDev, 3, 0 );
    m_layout->addWidget( m_cmbDev, 3, 1 );

    connect(m_cmbDev, SIGNAL(activated(const QString&) ),
            this, SLOT(slotTextChanged(const QString&  ) ) );
}
void QtopiaConfig::slotTextChanged( const QString& str ) {
    bool b  = str == QString::fromLatin1("Sharp Zaurus ROM");
    kdDebug(5225) << "Text Changed to " << str << " " << b <<endl;

    m_name->setEnabled( b );
    m_lblName->setEnabled( b );

    m_cmbUser->setEnabled( !b );
    m_lblUser->setEnabled( !b );

    m_cmbPass->setEnabled( !b );
    m_lblPass->setEnabled( !b );
}
QtopiaConfig::~QtopiaConfig() {

}

#include "qtopiaconfig.moc"
