#include <qobject.h>
#include <qwidget.h>

#include <kaboutdata.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>
#include <kparts/componentfactory.h>

#include <ksync_mainwindow.h>
#include "overviewwidget.h"
#include "ksync_overviewpart.h"

typedef KParts::GenericFactory< KSync::OverviewPart> OverviewPartFactory;
K_EXPORT_COMPONENT_FACTORY( liboverviewpart, OverviewPartFactory );

using namespace KSync ;

OverviewPart::OverviewPart(QWidget *parent, const char *name,
			   QObject *, const char *,const QStringList & )
  : ManipulatorPart( parent, name ) {
//  setInstance(OverviewPartFactory::instance() );
  m_pixmap = KGlobal::iconLoader()->loadIcon("kcmsystem", KIcon::Desktop, 48 );
  m_widget=0;
//  m_config = 0;
}
KAboutData *OverviewPart::createAboutData() {
    return new KAboutData("KSyncOverviewPart",  I18N_NOOP("Sync Overview Part"), "0.0" );
}

OverviewPart::~OverviewPart() {
    delete m_widget;
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
void OverviewPart::startSync()
{
    m_widget->clearProgress(core()->currentProfile(),  core()->konnector(), core()->currentId() );
};
void OverviewPart::slotProgress(ManipulatorPart* , int syncStatus,  int)
{
    if (syncStatus == SYNC_DONE)
        m_widget->currentDone();
};
void OverviewPart::slotSyncPartActivated( ManipulatorPart* part)
{
    m_widget->addProgress(part);
};


#include "ksync_overviewpart.moc"
