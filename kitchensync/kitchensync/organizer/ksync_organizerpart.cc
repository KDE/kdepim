
#include <qobject.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>

#include "ksync_organizerpart.h"

//typedef KGenericFactory< KitchenSync::OrganizerPart> OrganizerPartFactory;
//K_EXPORT_COMPONENT_FACTORY( liborganizerpart, OrganizerPartFactory );

using namespace KitchenSync ;

OrganizerPart::OrganizerPart(QWidget *parent, const char *name, const QStringList & )
  : KitchenSync::ManipulatorPart( parent, name )
{
  // setInstance(OrganizerPartFactory::instance() );
  m_pixmap = KGlobal::iconLoader()->loadIcon("korganizer", KIcon::Desktop, 48 );
  m_widget=0;
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
  kdDebug() << "configWidget \n" ;
  m_config = new QWidget();
  m_config->setBackgroundColor( Qt::green );
  
  return m_config;
};

//#include "ksync_organizerpart.moc"
