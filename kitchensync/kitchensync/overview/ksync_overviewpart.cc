#include <qobject.h>
#include <qwidget.h>

#include <kaboutdata.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>
#include <kparts/componentfactory.h>

#include <ksync_mainwindow.h>
#include "ksync_overviewpart.h"

typedef KParts::GenericFactory< KSync::OverviewPart> OverviewPartFactory;
K_EXPORT_COMPONENT_FACTORY( liboverviewpart, OverviewPartFactory );

using namespace KSync ;

OverviewPart::OverviewPart(QWidget *parent, const char *name,
			   QObject *, const char *,const QStringList & )
  : ManipulatorPart( parent, name ) {
  m_pixmap = KGlobal::iconLoader()->loadIcon("kcmsystem", KIcon::Desktop, 48 );
  m_widget=0;
}
KAboutData *OverviewPart::createAboutData() {
    return new KAboutData("KSyncOverviewPart",  I18N_NOOP("Sync Overview Part"), "0.0" );
}

OverviewPart::~OverviewPart() {
    delete m_widget;
}
QString OverviewPart::type()const {
    return QString::fromLatin1("Overview");
}
QString OverviewPart::name()const {
    return i18n("Overview" );
}
QString OverviewPart::description()const {
    return i18n("This part is the main widget of KitchenSync");
}
QPixmap* OverviewPart::pixmap() {
  return &m_pixmap;
}
QString OverviewPart::iconName()const {
    return QString::fromLatin1("kcmsystem");
}
bool OverviewPart::partIsVisible()const{
    return true;
}

QWidget* OverviewPart::widget() {
  if(!m_widget){
    m_widget = new QWidget();
  }
  return m_widget;
}

#include "ksync_overviewpart.moc"
