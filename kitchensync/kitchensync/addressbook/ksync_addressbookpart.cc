
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
    : ManipulatorPart( parent ? parent : obj ,  name )
{
    setInstance( AddressBookPartFactory::instance() );
    m_pixmap = KGlobal::iconLoader()->loadIcon("kaddressbook",  KIcon::Desktop,  48 );
    m_widget = 0;
}
AddressBookPart::~AddressBookPart()
{
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
    QString path = prof.path("AddressBook");

    m_widget = new AddressBookConfigBase();
    if ( QString::fromLatin1("evolution") == path ) {
        m_widget->ckbEvo->setChecked( true );
    }else {
        m_widget->ckbEvo->setChecked( false );
        m_widget->lnePath->setText( path );
    }

    return m_widget;
}
void AddressBookPart::processEntry( const Syncee::PtrList& in,
                                    Syncee::PtrList& out )
{
}
void AddressBookPart::slotConfigOk()
{
    Profile prof = core()->currentProfile();
    if ( m_widget->ckbEvo->isChecked() ) {
        prof.setPath( "AddressBook", "evolution" );
    }else {
        prof.setPath("AddressBook",m_widget->lnePath->text() );
    }
    core()->profileManager()->replaceProfile( prof );
    core()->profileManager()->setCurrentProfile( prof );


}


#include "ksync_addressbookpart.moc"
