
#include <qobject.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>
#include <kparts/componentfactory.h>

#include "ksync_organizerpart.h"

typedef KParts::GenericFactory< KitchenSync::OrganizerPart> OrganizerPartFactory;
K_EXPORT_COMPONENT_FACTORY( liborganizerpart, OrganizerPartFactory );

using namespace KitchenSync ;

OrganizerPart::OrganizerPart(QWidget *parent, const char *name,
			     QObject *obj, const char *na, const QStringList & )
  : KitchenSync::ManipulatorPart( parent, name )
{
  setInstance(OrganizerPartFactory::instance() );
  m_pixmap = KGlobal::iconLoader()->loadIcon("korganizer", KIcon::Desktop, 48 );
  m_widget=0;
  m_config=0;
}
OrganizerPart::~OrganizerPart()
{

}
QPixmap* OrganizerPart::pixmap()
{
  return &m_pixmap;
}
QWidget* OrganizerPart::widget()
{
  kdDebug() << "widget \n";
  if(m_widget==0 ){
    m_widget = new QWidget();
    m_widget->setBackgroundColor(Qt::green);
  }
  return m_widget;
}
QWidget* OrganizerPart::configWidget()
{
  //  if( m_config == 0 ){ cause of the reparent ;)
    kdDebug() << "configWidget \n" ;
    m_config = new QWidget();
    m_config->setBackgroundColor( Qt::green );
    //}
  return m_config;
};

KAboutData *OrganizerPart::createAboutData()
{
  return new KAboutData("KSyncOrganizerPart", I18N_NOOP("Sync organizer part"), "0.0" );
}
//#include "ksync_organizerpart.moc"
