#include <qobject.h>
#include <qwidget.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>

#include "overviewwidget.h"
#include "ksync_overviewpart.h"

//typedef KGenericFactory< KitchenSync::OverviewPart> OverviewPartFactory;
//K_EXPORT_COMPONENT_FACTORY( liboverviewpart, OverviewPartFactory );

using namespace KitchenSync ;

OverviewPart::OverviewPart(QWidget *parent, const char *name, const QStringList & )
  : KitchenSync::ManipulatorPart( parent, name ) {
  // setInstance(OverviewPartFactory::instance() );
  m_pixmap = KGlobal::iconLoader()->loadIcon("kcmsystem", KIcon::Desktop, 48 );
  m_widget=0;
}

OverviewPart::~OverviewPart() {
}

QPixmap* OverviewPart::pixmap() {
  return &m_pixmap;
}

QWidget* OverviewPart::widget() {
  if(m_widget==0 ){
    m_widget = new OverviewWidget();
  }
  return m_widget;
}

QWidget* OverviewPart::configWidget() {

  m_config = new QWidget();
  m_config->setBackgroundColor( Qt::red );

  return m_config;
};

