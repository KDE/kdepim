
#include <qobject.h>
#include <qwidget.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>

#include "ksync_organizerpart.h"

typedef KGenericFactory< KitchenSync::OrganizerPart, QWidget > OrganizerPartFactory;
K_EXPORT_COMPONENT_FACTORY( liborganizerpart,
			    OrganizerPartFactory( "organizerpart") );

using namespace KitchenSync ;

OrganizerPart::OrganizerPart(QWidget *parent, const char *name, const QStringList & )
  : KitchenSync::ManipulatorPart( parent, name )
{
  setInstance(OrganizerPartFactory::instance() );
  m_pixmap = KGlobal::iconLoader()->loadIcon("korganizer", KIcon::Desktop, 48 );
  m_widget=0;
}
QPixmap* OrganizerPart::pixmap()
{
  return &m_pixmap;
}
QWidget* OrganizerPart::widget()
{
  if(m_widget==0 ){
    m_widget = new QWidget();
    m_widget->setBackgroundColor(Qt::green);
  }
  return m_widget;
}
QWidget* OrganizerPart::configWidget()
{
  m_config = new QWidget();
  m_config->setBackgroundColor( Qt::red );
  return m_config;
};

//#include "ksync_organizerpart.moc"
