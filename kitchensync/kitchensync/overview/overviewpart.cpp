#include <kiconloader.h>
#include <kparts/genericfactory.h>

#include <konnectormanager.h>
#include <konnectorinfo.h>
#include <konnector.h>

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

OverviewPart::OverviewPart( QWidget *parent, const char *name,
                            QObject *, const char *,const QStringList & )
  : ActionPart( parent, name )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon( "kcmsystem", KIcon::Desktop, 48 );
  m_widget=0;

  connectPartChange( SLOT( slotPartChanged( ActionPart* ) ) );
  connectPartProgress( SLOT( slotPartProgress( ActionPart*, const Progress& ) ) );
  connectPartError( SLOT( slotPartError( ActionPart*, const Error& ) ) );
  connectKonnectorProgress( SLOT( slotKonnectorProgress( Konnector *, const Progress& ) ) );
  connectKonnectorError( SLOT( slotKonnectorError( Konnector *, const Error& ) ) );
  connectProfileChanged( SLOT( slotProfileChanged( const Profile& ) ) );
  connectSyncProgress( SLOT( slotSyncProgress( ActionPart*, int, int ) ) );
  connectStartSync( SLOT( slotStartSync() ) );
  connectDoneSync( SLOT( slotDoneSync() ) );
}

OverviewPart::~OverviewPart()
{
  delete m_widget;
}

KAboutData *OverviewPart::createAboutData()
{
  return new KAboutData( "KSyncOverviewPart", I18N_NOOP( "Sync Overview Part" ), "0.0" );
}

QString OverviewPart::type() const
{
  return QString::fromLatin1( "Overview" );
}

QString OverviewPart::title() const
{
  return i18n( "Overview" );
}

QString OverviewPart::description() const
{
  return i18n( "This part is the main widget of KitchenSync" );
}

QPixmap* OverviewPart::pixmap()
{
  return &m_pixmap;
}

QString OverviewPart::iconName() const
{
  return QString::fromLatin1( "kcmsystem" );
}

bool OverviewPart::hasGui() const
{
  return true;
}

QWidget* OverviewPart::widget()
{
  if ( !m_widget )
    m_widget = new OverView::Widget( 0, "part" );

  return m_widget;
}

void OverviewPart::slotPartChanged( ActionPart* part )
{
  kdDebug(5210) << "PartChanged" << part << " name" << part->name() << endl;
}

void OverviewPart::slotPartProgress( ActionPart* part, const Progress& prog )
{
  kdDebug(5210) << "PartProg: " << part << " " << prog << endl;
  m_widget->addProgress( part, prog );
}

void OverviewPart::slotPartError( ActionPart* part, const Error& err )
{
  kdDebug(5210) << "PartError: " << part << " " << err << endl;
  m_widget->addError( part, err );
}

void OverviewPart::slotKonnectorProgress( Konnector *k, const Progress& prog )
{
  kdDebug(5210) << "KonnectorProgress: " << prog << endl;
  m_widget->addProgress( k, prog );
}

void OverviewPart::slotKonnectorError( Konnector *k, const Error& prog )
{
  kdDebug(5210) << "KonnectorError : " << prog << endl;
  m_widget->addError( k, prog );
}

void OverviewPart::slotProfileChanged( const Profile & )
{
  m_widget->setProfile( core()->currentProfile() );
  kdDebug(5210) << "Profile changed " << endl;
}

void OverviewPart::slotSyncProgress( ActionPart* part, int status, int percent )
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

void OverviewPart::executeAction()
{
  kdDebug(5210) << "OverviewPart::executeAction()" << endl;
}

#include "overviewpart.moc"
