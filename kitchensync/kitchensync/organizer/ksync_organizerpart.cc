
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
#include <kalendarsyncentry.h>
#include <qptrlist.h>

#include "ksync_return.h"
#include "ksync_sync.h"

typedef KParts::GenericFactory< KitchenSync::OrganizerPart> OrganizerPartFactory;
K_EXPORT_COMPONENT_FACTORY( liborganizerpart, OrganizerPartFactory );

using namespace KitchenSync ;

namespace {
    void record( KCal::CalendarLocal* local ) {
        if ( local ==0 )
            return;
        const QPtrList<KCal::Todo> todos = local->getTodoList();
        QPtrListIterator<KCal::Todo> it( todos );
        QPtrList<KCal::Event> events = local->getAllEvents();
        KCal::Event* event;
        KCal::Todo* todo;
        for ( ; it.current(); ++it ) {
            todo = it.current();
            kdDebug() << "Todo: " << todo->description() << endl;
        }
        for ( event = events.first(); event != 0; event = events.next() ) {
            kdDebug() << "Event:" << event->summary() << endl;
        }
    }
    // dumps all meta information
    void dump( const QString &prefix,  KAlendarSyncEntry* entry ) {
        if ( entry->syncMode() == KSyncEntry::SYNC_META && !entry->firstSync() ) {
            kdDebug() << prefix << " Meta Informations" << endl;
            kdDebug() << prefix << "Modified" << endl;
            record( entry->modified() );
            kdDebug() << prefix << "Removed" << endl;
            record( entry->removed() );
            kdDebug() << prefix << "Added" << endl;
            record( entry->added() );
        }else{
            kdDebug() << prefix << "can not sync " << endl;
        }
    };

};

OrganizerPart::OrganizerPart(QWidget *parent, const char *name,
			     QObject *obj, const char *na, const QStringList & )
  : KitchenSync::ManipulatorPart( parent, name )
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
void OrganizerPart::processEntry( const KSyncEntry::List& in,
                                  KSyncEntry::List& out )
{
    QPtrList<KAlendarSyncEntry> our;
    QPtrListIterator<KSyncEntry> it( in );
    KSyncEntry *entry;
    KAlendarSyncEntry* entry2 = 0;
    while ( (entry = it.current() ) != 0 ) {
        ++it;
        kdDebug() << entry->type() << endl;
        if ( entry->type() == QString::fromLatin1("KAlendarSyncEntry") ) {
            kdDebug() << "Found our type" << endl;
            our.append(  (KAlendarSyncEntry*) entry );
            //out.append(  entry->clone() );
            entry2 = (KAlendarSyncEntry*) entry->clone() ; //not cloning
            break;
        }
    }
    if (entry2 == 0 )
        return;
    // now load
    KAlendarSyncEntry* met = meta();

    // sync
    SyncManager manager( this,  "SyncManager");
    KSyncEntry::List one;
    one.append( entry2 );

    KSyncEntry::List two;
    two.append( met );
    SyncReturn ret =  manager.sync( SYNC_INTERACTIVE,  one, two );
    delete entry2;
    delete met;
    QDateTime time = QDateTime::currentDateTime();
    // write back if meta
    two = ret.synced();
    KAlendarSyncEntry *calendar=0;

    for ( entry = two.first(); entry != 0; entry = two.next() ) {
        if ( entry->type() == QString::fromLatin1("KalendarSyncEntry") ) {
            calendar = (KAlendarSyncEntry*)entry;
            break;
        }
    }
    if ( calendar == 0 )
        return;
    // store date time
    Profile prof = core()->currentProfile();
    Kapabilities cap = prof.caps();
    KCal::ICalFormat *form = new KCal::ICalFormat(calendar->calendar() );
    if ( cap.isMetaSyncingEnabled() ) {
        m_conf->setGroup( prof.name() );
        m_conf->writeEntry("time", time );
        QString metaPath = m_conf->readEntry("metaPath" );
        calendar->calendar()->save( QDir::homeDirPath() + "/.kitchensync/meta/"+ metaPath + "/cal.vcf",  form);
        kdDebug() << "Dumping Opie " << endl;
        kdDebug() << "Sync mode == " << entry2->syncMode() << " first sync = " << entry2->firstSync() << endl;
        dump("Opie",  entry2);
        dump("KDE",  met );
    }
    KURL url(m_path );
    QString newPath;
    if ( !url.isLocalFile() ) {
        ;
    }else{
        newPath = url.path();
    }
    calendar->calendar()->save( newPath, form );
    delete form;
    out.append( calendar );
}

KAlendarSyncEntry* OrganizerPart::meta()
{
    KAlendarSyncEntry* entry = new KAlendarSyncEntry();
    KCal::CalendarLocal* cal = new KCal::CalendarLocal();
    if ( m_path.isEmpty() ) {
        KConfig conf("korganizerrc");
        conf.setGroup("General");
        m_path = conf.readEntry("Active Calendar");
    }
    KURL url(m_path );
    QString newPath;
    if ( !url.isLocalFile() ) {
        ;
    }else{
        newPath = url.path();
    }
    kdDebug() << "Path " << newPath << endl;
    cal->load( newPath );
    entry->setCalendar( cal );

    Profile prof = core()->currentProfile();
    Kapabilities cap = prof.caps();
    // meta data
    if ( cap.isMetaSyncingEnabled() ) {
        entry->setSyncMode( KSyncEntry::SYNC_META );
        m_conf->setGroup( prof.name() );
        QString metaPath = m_conf->readEntry("metaPath");
        if ( metaPath.isEmpty() ) {
            entry->setFirstSync( true );
            QString meta = kapp->randomString( 6 );
            meta.append("orgpart");
            m_conf->writeEntry("metaPath",  meta );
            m_conf->sync();
            metaPath = meta;
            QDir dir;
            dir.mkdir(QDir::homeDirPath() + "/.kitchensync/");
            dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta");
            dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta/"+ metaPath);
        }else{
            entry->setFirstSync( false );
            QDateTime date = m_conf->readDateTimeEntry("time" );
            KCal::CalendarLocal *local = new KCal::CalendarLocal();
            local->load( QDir::homeDirPath() + "/.kitchensync/meta/"+ metaPath + "/cal.vcf");
            // now let's gather meta data ( added, modified, removed )
            QPtrList<KCal::Event> newE =   cal->getAllEvents();
            QPtrList<KCal::Event> oldE = local->getAllEvents();
            QPtrList<KCal::Event> addE;
            QPtrList<KCal::Event> modE;
            QPtrList<KCal::Event> delE;
            KCal::Event *Event1;
            KCal::Event *Event2;
            bool found;
            for ( Event1 = newE.first(); Event1 != 0; Event1 = newE.next() ) {
                found = false;
                for ( Event2 = oldE.first(); Event2 != 0; Event2 = oldE.next() ) {
                    if ( Event2->uid() == Event1->uid() ) { // it's there look if it was modified
                        if ( date < Event1->lastModified() )
                            modE.append( (KCal::Event*) Event1->clone() );
                        found = true;
                        break;
                    }
                }
                if (!found ) { // added
                    addE.append( (KCal::Event*)Event1->clone() );
                }
            } // Done with Event1 -> Event2
            // Do Event2->Event1
            for ( Event2 = oldE.first(); Event2 != 0; Event2 = oldE.next() ) {
                found = false;
                for (Event1 = newE.first(); Event1 != 0; Event1 = newE.next() ) {
                    if ( Event1->uid() == Event2->uid() ) {
                        found = true;
                        break;
                    }
                }
                if (!found )
                    delE.append( (KCal::Event*)Event2->clone() );
            }
            // ////////////////////////////////////////////////////////////////////////////////////
            // /////////////////////////////////// ToDo //////////////////////////////////////////
            const QPtrList<KCal::Todo> newT =   cal->getTodoList();
            const QPtrList<KCal::Todo> oldT = local->getTodoList();
            QPtrList<KCal::Todo> addT;
            QPtrList<KCal::Todo> modT;
            QPtrList<KCal::Todo> delT;
            KCal::Todo* Todo1;
            KCal::Todo* Todo2;
            QPtrListIterator<KCal::Todo> newIt( newT );
            QPtrListIterator<KCal::Todo> oldIt( oldT );
            for ( ; newIt.current(); ++newIt ) {
                found = false;
                Todo1 = newIt.current();
                oldIt.toFirst();
                for ( ; oldIt.current(); ++oldIt ) {
                    Todo2 = oldIt.current();
                    if ( Todo1->uid() == Todo2->uid() ) {
                        found = true;
                        if ( date < Todo1->lastModified() )
                            modT.append( (KCal::Todo*)Todo1->clone() );
                        break;
                    }
                }
                if (!found )
                    addT.append( (KCal::Todo*)Todo1->clone() );
            }
            newIt.toFirst();
            oldIt.toFirst();
            for ( ; oldIt.current(); ++oldIt ) {
                found = false;
                Todo2 = oldIt.current();
                for ( ; newIt.current(); ++newIt ) {
                    Todo1 = newIt.current();
                    if (Todo2->uid() == Todo1->uid() ) {
                        found  = true;
                        break;
                    }
                }
                if (!found )
                    delT.append( (KCal::Todo*) Todo2->clone() );
            }
            delete local;
            // gathered all meta data now set it
            // added
            KCal::CalendarLocal *added = new KCal::CalendarLocal();
            for ( Todo1 = addT.first(); Todo1 != 0; Todo1 = addT.next() ) {
                added->addTodo( Todo1 );
            }
            for ( Event1 = addE.first(); Event1 != 0; Event1 = addE.next() ) {
                added->addEvent( Event1 );
            }
            entry->setAdded( added );

            // modified
            KCal::CalendarLocal *modified = new  KCal::CalendarLocal();
            for ( Todo1 = modT.first(); Todo1 != 0; Todo1 = modT.next() ) {
                modified->addTodo( Todo1 );
            }
            for ( Event1 = modE.first(); Event1 != 0; Event1 = modE.next() ) {
                modified->addEvent( Event1 );
            }
            entry->setModified( modified );

            // deleted
            KCal::CalendarLocal *removed = new KCal::CalendarLocal();
            for ( Todo1 = delT.first(); Todo1 != 0; Todo1 = delT.next() ) {
                removed->addTodo( Todo1 );
            }
            for ( Event1 = delE.first(); Event1 != 0; Event1 = delE.next() ) {
                removed->addEvent( Event1 );
            }
            entry->setRemoved( removed );
        }
    }else
        entry->setSyncMode(KSyncEntry::SYNC_NORMAL);
    return entry;
}
#include "ksync_organizerpart.moc"
