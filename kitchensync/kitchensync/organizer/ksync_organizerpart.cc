
#include <qdir.h>
#include <qobject.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qdatetime.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>
#include <kparts/componentfactory.h>

#include <libkcal/icalformat.h>
#include <libkcal/calendarlocal.h>

#include "organizerbase.h"
#include "ksync_mainwindow.h"
#include "ksync_organizerpart.h"
#include <qptrlist.h>

//#include "ksync_return.h"
//#include "ksync_sync.h"

typedef KParts::GenericFactory< KSync::OrganizerPart> OrganizerPartFactory;
K_EXPORT_COMPONENT_FACTORY( liborganizerpart, OrganizerPartFactory );

using namespace KSync ;

OrganizerPart::OrganizerPart(QWidget *parent, const char *name,
			     QObject *obj, const char *na, const QStringList & )
  : ManipulatorPart( parent, name )
{
    kdDebug() << "Parent " << parent->className() << endl;
    kdDebug() << "Object " << obj->className() << endl;
  setInstance(OrganizerPartFactory::instance() );
  m_pixmap = KGlobal::iconLoader()->loadIcon("korganizer", KIcon::Desktop, 48 );
  m_widget=0;
  m_config=0;
  m_conf = new KConfig( "KitchenSyncOrganizerPart");
  m_configured = false;
}
OrganizerPart::~OrganizerPart()
{
    delete m_conf;
}
QPixmap* OrganizerPart::pixmap()
{
  return &m_pixmap;
}
QWidget* OrganizerPart::widget()
{
  kdDebug(5222) << "widget \n";
  if(m_widget==0 ){
    m_widget = new QWidget();
    m_widget->setBackgroundColor(Qt::green);
  }
  return m_widget;
}
QWidget* OrganizerPart::configWidget()
{
  //  if( m_config == 0 ){ cause of the reparent ;)
    if ( core() != 0 )
        kdDebug() << "Config Widget "<< endl;
    Profile prof = core()->currentProfile();
    if ( !m_configured ) {
        m_conf->setGroup( prof.name() );
        m_path = m_conf->readEntry("Path");
        m_evo = m_conf->readBoolEntry("Evo");
        m_configured = true;
    }
    kdDebug(5222) << "configWidget \n" ;
    m_config = new OrganizerDialogBase();
    m_config->urlReq->setURL( m_path );
    m_config->ckbEvo->setChecked( m_evo );
    //m_config->setBackgroundColor( Qt::green );
    //}
  return (QWidget*) m_config;
};

KAboutData *OrganizerPart::createAboutData()
{
  return new KAboutData("KSyncOrganizerPart", I18N_NOOP("Sync organizer part"), "0.0" );
}

void OrganizerPart::slotConfigOk()
{
    Profile prof = core()->currentProfile();
    m_path = m_config->urlReq->url();
    m_evo = m_config->ckbEvo->isChecked();
    m_conf->setGroup( prof.name() );
    m_conf->writeEntry( "Path",  m_path );
    m_conf->writeEntry( "Evo", m_evo );
}
void OrganizerPart::processEntry( const Syncee::PtrList& in,
                                  Syncee::PtrList& out )
{
}


#include "ksync_organizerpart.moc"
