
#include <qdir.h>
#include <qobject.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qdatetime.h>

#include <kapplication.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>
#include <kparts/componentfactory.h>

#include <libkcal/icalformat.h>
#include <libkcal/calendarlocal.h>

#include <eventsyncee.h>
#include <todosyncee.h>
#include <syncer.h>

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
        m_config->urlReq->setURL( path );

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
    /* 1. is fairly easy */
    Profile prof = core()->currentProfile();
    KonnectorProfile kon = core()->konnectorProfile();

    /* 2. too */
    QString path = prof.path( "OrganizerPart"  );
    QString meta =  kon.uid()+"/"+prof.uid() + "organizer.rc";
    bool met = kon.kapabilities().isMetaSyncingEnabled();

    /* 3. */
    Syncee* syncee=0l;
    EventSyncee* evSyncee = 0l;
    TodoSyncee* toSyncee = 0l;
    QPtrListIterator<Syncee> syncIt( in );
    for ( ; syncIt.current(); ++syncIt ) {
        syncee = syncIt.current();
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

    /* 5. meta data */
    if ( met )
        doMeta( events, todos, meta );
    //else {
    // set the sync MODE FIXME
    //}
    /* 6.  sync */
    Syncer sync( core()->syncUi(), core()->syncAlgorithm() );
    if (evSyncee ) {
        sync.addSyncee(evSyncee);
        sync.addSyncee(events);
        sync.sync();
        sync.clear();
    }
    if (toSyncee ) {
        sync.addSyncee( toSyncee );
        sync.addSyncee( todos );
        sync.sync();
        sync.clear();
    }

    /* 7. write back meta */
    if ( met )
        writeMeta( events, todos, meta );

    /* 8. write back data */
    save( events, todos, path );

    /* 9. */
    out.append( events );
    out.append( todos );
}

//AddressBookSyncee* ANY
TodoSyncee* OrganizerPart::loadTodos( const QString& path ) {
    TodoSyncee* syncee = new TodoSyncee();
    KCal::CalendarLocal cal;
    cal.load(path);
    QPtrList<KCal::Todo> todos = cal.rawTodos();
    if ( todos.isEmpty() ) {
        return syncee;
    }

    KCal::Todo *todo;
    TodoSyncEntry* entry =0 ;
    for ( todo = todos.first(); todo; todo = todos.next() ) {
        entry = new TodoSyncEntry((KCal::Todo*)todo->clone()) ;
        syncee->addEntry( entry );
    }
    return syncee;
}
EventSyncee* OrganizerPart::loadEvents(const QString& path) {
    EventSyncee* syncee = new EventSyncee();
    KCal::CalendarLocal cal;
    cal.load( path );
    QPtrList<KCal::Event> events = cal.rawEvents();
    if ( events.isEmpty() ) {
        return syncee;
    }

    KCal::Event *event;
    EventSyncEntry* entry;
    for ( event = events.first(); event; event = events.next() ) {
        entry = new EventSyncEntry( (KCal::Event*)event->clone() );
        syncee->addEntry( entry );
    }
    return syncee;
}
/*
 * For meta informations we're using a KConfig file
 * we've one Group for each id
 * and we save the timestamp
 * First we try to find modified
 * then added and removed
 * Modified is quite easy to find
 * if there hasGroup(uid) and timestamp differs it was
 * modified.
 * added: if not hasGroup(uid) added
 * removed: if it's inside the Config file but not in
 * the current set of data
 */
void OrganizerPart::doMeta( EventSyncee* evSyncee,
                            TodoSyncee* toSyncee,
                            const QString& path ) {
    QString str = QDir::homeDirPath();
    str += "/.kitchensync/meta/konnector-" + path;
    if (!QFile::exists( str ) ) {
        evSyncee->setFirstSync( true );
        toSyncee->setFirstSync( true );
        evSyncee->setSyncMode( Syncee::MetaMode );
        toSyncee->setSyncMode( Syncee::MetaMode );
        return;
    }
    KSimpleConfig conf( str );
    doMetaIntern(evSyncee, &conf, "events-" );
    doMetaIntern(toSyncee, &conf, "todos-" );
}
void OrganizerPart::doMetaIntern( Syncee* syncee,
                                  KSimpleConfig* conf,
                                  const QString& key) {
    syncee->setSyncMode( Syncee::MetaMode );
    SyncEntry* entry;
    QStringList ids;
    QString timestmp;

    /* mod + added */
    for ( entry = syncee->firstEntry(); entry; entry = syncee->nextEntry() ) {
        ids << entry->id();
        /* has group see if modified */
        if ( conf->hasGroup( key + entry->id() ) ) {
            conf->setGroup( key + entry->id() );
            timestmp = conf->readEntry( "time");

            /* timestamp mismatch */
            if ( timestmp != entry->timestamp() )
                entry->setState(SyncEntry::Modified );

        }else { // added
            entry->setState( SyncEntry::Added );
        }

    }
    /* now let's find the removed items */
    QStringList  groups = conf->groupList();
    QStringList::Iterator it;
    // first find the groups for our key
    for ( it = groups.begin(); it != groups.end(); ++it ) {
        /* if group starts with the key */
        if ( (*it).startsWith(key ) ) { // right group
            QString id = (*it).mid( key.length() );
            kdDebug() << "OrganizerPart Meta Gathering: "
                      << id << endl;

            /* the previous saved list of ids
             *  does not contain this id
             * ->REMOVED
             * items were removed we need to create an item
             * with just a uid......
             * let's find out which item to create or have
             * 2 methods sharing 95% of code
             */
            if (!ids.contains( id )) {
                if ( syncee->type() == "TodoSyncee" ) {
                    KCal::Todo *todo = new KCal::Todo();
                    todo->setUid( id );
                    TodoSyncEntry* entry = new TodoSyncEntry( todo );
                    entry->setState( SyncEntry::Removed );
                    syncee->addEntry( entry );
                }else { // organizer
                    KCal::Event* ev = new KCal::Event();
                    ev->setUid( id );
                    EventSyncEntry* entry = new EventSyncEntry( ev );
                    entry->setState( SyncEntry::Removed );
                    syncee->addEntry( entry );
                }
            }
        }
    }
};
/**
 * let's save it
 */
void OrganizerPart::writeMeta( EventSyncee* evSyncee,
                               TodoSyncee* toSyncee,
                               const QString& path) {
    QString str = QDir::homeDirPath();
    str += "/.kitchensync/meta/konnector-" + path;
    if (!QFile::exists( str ) ) {
        KonnectorProfile kon = core()->konnectorProfile();
        QDir dir;
        dir.mkdir( str + "/.kitchensync");
        dir.mkdir( str + "/.kitchensync/meta");
        dir.mkdir( str + "/.kitchensync/meta/konnector-" + kon.uid() );
    }
    KSimpleConfig conf( str );
    writeMetaIntern( evSyncee, &conf, "events-");
    writeMetaIntern( toSyncee, &conf, "todos-");
}
void OrganizerPart::writeMetaIntern( Syncee* syncee,
                                     KSimpleConfig* conf,
                                     const QString& key ) {
    SyncEntry* entry;
    for (entry = syncee->firstEntry(); entry; entry= syncee->nextEntry() ) {
        conf->setGroup( key + entry->id() );
        conf->writeEntry( "time", entry->timestamp() );
    }
}
void OrganizerPart::save( EventSyncee* evSyncee,
                          TodoSyncee* toSyncee,
                          const QString& path) {
    KCal::CalendarLocal* loc = new KCal::CalendarLocal();
    EventSyncEntry* evEntry=0l;
    TodoSyncEntry* toEntry=0l;
    for ( evEntry = (EventSyncEntry*)evSyncee->firstEntry();
          evEntry;
          evEntry = (EventSyncEntry*)evSyncee->nextEntry() ) {
        loc->addEvent( evEntry->incidence() );
    }
    for ( toEntry = (TodoSyncEntry*)toSyncee->firstEntry();
          toEntry;
          toEntry = (TodoSyncEntry*)toSyncee->nextEntry() ) {
        loc->addTodo( toEntry->todo() );
    }
    loc->save( path );
}
#include "ksync_organizerpart.moc"
