
#include <qcheckbox.h>
#include <qdir.h>
#include <qlineedit.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kstddirs.h>

#include <kparts/genericfactory.h>

#include <kabc/resourcefile.h>

#include <ksync_mainwindow.h>


#include "addressbookbase.h"
#include "ksync_addressbookpart.h"

typedef KParts::GenericFactory< KSync::AddressBookPart> AddressBookPartFactory;
K_EXPORT_COMPONENT_FACTORY( libaddressbookpart,  AddressBookPartFactory );

using namespace KSync;


AddressBookPart::AddressBookPart( QWidget* parent,  const char* name,
                                  QObject* obj,  const char* name2,
                                  const QStringList & )
    : ManipulatorPart( parent,  name )
{
    setInstance( AddressBookPartFactory::instance() );
    m_pixmap = KGlobal::iconLoader()->loadIcon("kaddressbook",  KIcon::Desktop,  48 );
    m_widget = 0;
    m_config = new KConfig( "KitchenSyncAddressBookPart");
    m_configured = false;
}
AddressBookPart::~AddressBookPart()
{
    delete m_config;
}
KAboutData *AddressBookPart::createAboutData()
{
  return new KAboutData("KSyncAddressBookPart", I18N_NOOP("Sync AddressBook Part"), "0.0" );
}
QPixmap* AddressBookPart::pixmap()
{
    return &m_pixmap;
}
QWidget* AddressBookPart::widget()
{
    return 0l;
}
QWidget* AddressBookPart::configWidget()
{
    Profile prof = core()->currentProfile();
    if (!m_configured ) {
        m_config->setGroup( prof.name() );
        m_path = m_config->readEntry("Path");
        m_evo = m_config->readBoolEntry("Evo");
        m_configured = true;
    }
    m_widget = new AddressBookConfigBase();
    m_widget->lnePath->setText( m_path );
    m_widget->ckbEvo->setChecked( m_evo );
    return m_widget;
}
void AddressBookPart::processEntry( const Syncee::PtrList& in,
                                    Syncee::PtrList& out )
{
}
void AddressBookPart::slotConfigOk()
{
    Profile prof = core()->currentProfile();
    m_path = m_widget->lnePath->text();
    m_evo = m_widget->ckbEvo->isChecked();
    m_config->writeEntry( "Path",  m_path );
    m_config->writeEntry( "Evo",  m_evo );
}


#include "ksync_addressbookpart.moc"
