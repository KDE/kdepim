
#include <kiconloader.h>
#include <kparts/genericfactory.h>

#include <konnectormanager.h>
#include <konnectorinfo.h>
#include <konnectorplugin.h>

#include <mainwindow.h>

#include "overviewwidget.h"
#include "overviewpart.h"

typedef KParts::GenericFactory< KSync::OverviewPart> OverviewPartFactory;
K_EXPORT_COMPONENT_FACTORY( liboverviewpart, OverviewPartFactory )

using namespace KSync;

namespace {

kdbgstream operator<<( kdbgstream str, const Notify& no )
{
    str << no.code() << " " << no.text();
    return str;
}

kndbgstream operator <<( kndbgstream str, const Notify & )
{
    return str;
}

}

OverviewPart::OverviewPart(QWidget *parent, const char *name,
			   QObject *, const char *,const QStringList & )
  : ManipulatorPart( parent, name )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon("kcmsystem", KIcon::Desktop, 48 );
  m_widget=0;

  /* connections here */
  connectPartChange(SLOT(slotPartChanged(ManipulatorPart*) ) );
  connectPartProgress(SLOT(slotPartProgress(ManipulatorPart*, const Progress& ) ) );
  connectPartError(SLOT(slotPartError(ManipulatorPart*, const Error& ) ) );
  connectKonnectorProgress(SLOT(slotKonnectorProgress(Konnector *, const Progress& ) ) );
  connectKonnectorError(SLOT(slotKonnectorError(Konnector *, const Error& ) ) );
  connectProfileChanged(SLOT(slotProfileChanged(const Profile& ) ) );
  connectKonnectorChanged(SLOT(slotKonnectorChanged( Konnector * ) ) );
  connectSyncProgress(SLOT(slotSyncProgress( ManipulatorPart*, int, int) ) );
  connectStartSync(SLOT(slotStartSync() ) );
  connectDoneSync(SLOT(slotDoneSync() ) );
}

KAboutData *OverviewPart::createAboutData()
{
    return new KAboutData("KSyncOverviewPart",  I18N_NOOP("Sync Overview Part"), "0.0" );
}

OverviewPart::~OverviewPart()
{
    delete m_widget;
}

QString OverviewPart::type() const
{
    return QString::fromLatin1("Overview");
}

QString OverviewPart::name() const
{
    return i18n("Overview" );
}

QString OverviewPart::description() const
{
    return i18n("This part is the main widget of KitchenSync");
}

QPixmap* OverviewPart::pixmap()
{
  return &m_pixmap;
}

QString OverviewPart::iconName() const
{
    return QString::fromLatin1("kcmsystem");
}

bool OverviewPart::partIsVisible() const
{
    return true;
}

QWidget* OverviewPart::widget()
{
  if( !m_widget ) {
      m_widget = new OverView::Widget( 0, "part");
  }
  return m_widget;
}

void OverviewPart::slotPartChanged(ManipulatorPart* part)
{
    kdDebug(5210) << "PartChanged" << part << " name" << part->name() << endl;
}

void OverviewPart::slotPartProgress( ManipulatorPart* part, const Progress& prog)
{
    kdDebug(5210) << "PartProg: " << part << " " << prog << endl;
    m_widget->addProgress( part, prog );
}

void OverviewPart::slotPartError( ManipulatorPart* part, const Error& err)
{
    kdDebug(5210) << "PartError: " << part << " " << err << endl;
    m_widget->addError( part, err );
}

void OverviewPart::slotKonnectorProgress( Konnector *k, const Progress& prog)
{
    kdDebug(5210) << "KonnectorProgress: " << prog << endl;
    m_widget->addProgress( k, prog );
}

void OverviewPart::slotKonnectorError( Konnector *k, const Error& prog)
{
    kdDebug(5210) << "KonnectorError : " << prog << endl;
    m_widget->addError( k, prog );
}

void OverviewPart::slotProfileChanged(const Profile& )
{
    m_widget->setProfile( core()->currentProfile() );
    slotKonnectorChanged( core()->currentKonnectorProfile().konnector() );
    kdDebug(5210) << "Profile changed " << endl;
}

void OverviewPart::slotKonnectorChanged( Konnector * )
{
    KonnectorProfile prof = core()->currentKonnectorProfile();
    if ( prof.konnector() ) {
      QPixmap pix = DesktopIcon( prof.konnector()->info().iconName(), KIcon::User );
      m_widget->setProfile( prof.name(), pix );
      kdDebug(5210) << "Konnector Changed " << endl;
    }
}

void OverviewPart::slotSyncProgress( ManipulatorPart* part, int status, int percent )
{
    m_widget->syncProgress( part, status, percent );
}

void OverviewPart::slotStartSync()
{
    m_widget->startSync();
    kdDebug(5210) << "Start Sync " << endl;
}

void OverviewPart::slotDoneSync()
{
    kdDebug(5210) << "Done Sync " << endl;
}

#include "overviewpart.moc"
