
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

#include <eventsyncee.h>
#include <todosyncee.h>

#include "organizerbase.h"
#include <ksync_mainwindow.h>
#include <ksync_profile.h>
#include <konnectorprofile.h>
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
//    kdDebug() << "Parent " << parent->className() << endl;
//    kdDebug() << "Object " << obj->className() << endl;
  setInstance(OrganizerPartFactory::instance() );
  m_pixmap = KGlobal::iconLoader()->loadIcon("korganizer", KIcon::Desktop, 48 );
}
OrganizerPart::~OrganizerPart()
{
}
QPixmap* OrganizerPart::pixmap()
{
    return new QPixmap(m_pixmap);
}
QWidget* OrganizerPart::widget()
{
//  kdDebug(5222) << "widget \n";
  if(m_widget==0 ){
    m_widget = new QWidget();
    m_widget->setBackgroundColor(Qt::green);
  }
  return m_widget;
}
QWidget* OrganizerPart::configWidget()
{
    Profile prof = core()->currentProfile();
    QString path = prof.path( "OrganizerPart");

    m_config = new OrganizerDialogBase();

    if ( path == QString::fromLatin1("evolution") )
        m_config->ckbEvo->setChecked( true );
    else
        m_config->urlReq->setURL( m_path );

  return (QWidget*) m_config;
};

KAboutData *OrganizerPart::createAboutData()
{
  return new KAboutData("KSyncOrganizerPart", I18N_NOOP("Sync organizer part"), "0.0" );
}

void OrganizerPart::slotConfigOk()
{
    Profile prof = core()->currentProfile();
    if ( m_config->ckbEvo->isChecked() )
        prof.setPath("OrganizerPart", "evolution");
    else
        prof.setPath("OrganizerPart", m_config->urlReq->url() );

    core()->profileManager()->replaceProfile( prof );
    core()->profileManager()->setCurrentProfile( prof );
}
/* Do it for Events + Todos
 * The 10 Stages to synced data
 * 1. get the currentProfile and currentKonnector
 * 2. get the paths + the information about using meta data
 *    from the Konnector
 * 3. search our Syncees
 * 4. load the File from currentProfile
 * 5. If meta data collect it currentProfile.uid() + currentKonnectorProf.uid()
 *    + name for the MetaInformation filename
 * 6. SYNC using our own algorithm
 * 7. write back meta data if necessary
 * 8. write back file
 * 9. append the Syncee to the out list
 * 10. have a party and get drunk
 */
void OrganizerPart::processEntry( const Syncee::PtrList& in,
                                  Syncee::PtrList& out )
{
/*    /* 1. is fairly easy */
    Profile prof = core()->currentProfile();
    KonnectorProfile kon = core()->konnectorProfile();

    /* 2. too */
    QString path = prof.path( realPath( kon.path("OrganizerPart") ) );
    QString meta = prof.uid() + kon.uid() + "organizer.rc";
    bool met = kon.kapabilities().isMetaSyncingEnabled();

    /* 3. */
    Syncee* syncee;
    EventSyncee* evSyncee = 0l;
    TodoSyncee* toSyncee = 0l;
    for ( syncee = in.first(); syncee; syncee = in.next() ) {
        if ( syncee->type() == QString::fromLatin1("EventSyncee") )
            evSyncee = (EventSyncee*)syncee;
        else if ( syncee->type() == QString::fromLatin1("TodoSyncee") )
            toSyncee = (TodoSyncee*)syncee;
        /* we got both now we can break the loop */
        if ( evSyncee && toSyncee ) break;
    }
    if (!evSyncee && !toSyncee) return; // did not find both of them

    /* 4. load */
    EventSyncee* events =0l;
    TodoSyncee* todos = 0l;
    if (evSyncee )
       events = loadEvents( path );
    if (toSyncee )
        todos = loadTodos( path );
    */
}
//AddressBookSyncee* ANY

#include "ksync_organizerpart.moc"
