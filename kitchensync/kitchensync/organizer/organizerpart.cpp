
#include <qcheckbox.h>

#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kparts/genericfactory.h>

#include <libkcal/icalformat.h>
#include <libkcal/calendarlocal.h>

#include <eventsyncee.h>
#include <todosyncee.h>
#include <syncer.h>

#include <mainwindow.h>

#include "organizerpart.h"

//#include "return.h"
//#include "sync.h"

typedef KParts::GenericFactory< KSync::OrganizerPart> OrganizerPartFactory;
K_EXPORT_COMPONENT_FACTORY( liborganizerpart, OrganizerPartFactory );

using namespace KSync ;

OrganizerPart::OrganizerPart(QWidget *parent, const char *name,
			     QObject *, const char *, const QStringList & )
  : ManipulatorPart( parent, name )
{
  setInstance(OrganizerPartFactory::instance() );
  m_pixmap = KGlobal::iconLoader()->loadIcon("korganizer", KIcon::Desktop, 48 );

  kdDebug() << "OragnizerPart" << endl;
}
OrganizerPart::~OrganizerPart()
{
}
QString OrganizerPart::iconName()const {
    return QString::fromLatin1("korganizer");
}
QPixmap* OrganizerPart::pixmap()
{
    return &m_pixmap;
}
QString OrganizerPart::type()const {
    return QString::fromLatin1("Organizer");
}
QString OrganizerPart::name()const {
    return i18n("Organizer");
}
QString OrganizerPart::description()const {
    return i18n("This part is responsible for syncing your Calendar.");
}
bool OrganizerPart::partIsVisible()const {
    return false;
}
bool OrganizerPart::configIsVisible()const {
    return true;
}
QWidget* OrganizerPart::configWidget()
{
    m_config = new OrganizerDialogBase();

    if ( isEvolutionSync() )
        m_config->ckbEvo->setChecked( true );
    else{
        m_config->urlReq->setURL( core()->currentProfile().path("OrganizerPart") );
    }

  return (QWidget*) m_config;
};
bool OrganizerPart::isEvolutionSync()const {
    QString path;
    path = core()->currentProfile().path( "OrganizerPart" );

    return ( path == QString::fromLatin1("evolution" ) ) ;
}

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
 * 0. get the time zone id
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
void OrganizerPart::sync( const SynceeList &in,
                          SynceeList &out )
{
    /* 0. */
    KConfig conf("korganizerrc");
    conf.setGroup("Time & Date");
    QString timeZoneId = conf.readEntry("TimeZoneId", QString::fromLatin1("UTC") );
    kdDebug(5222) << " Time & Date " << timeZoneId << endl;

    /* 1. is fairly easy */
    Profile prof = core()->currentProfile();
    KonnectorProfile kon = core()->currentKonnectorProfile();

    /* 2. too */
    QString path = prof.path( "OrganizerPart"  );
    QString meta =  kon.uid()+"/"+prof.uid() + "organizer.rc";
    bool met = kon.kapabilities().isMetaSyncingEnabled();

    /* 3. */
    EventSyncee *evSyncee = in.eventSyncee();
    TodoSyncee *toSyncee = in.todoSyncee();
    if (!evSyncee && !toSyncee) {
        done();
        return; // did not find both of them
    }

    /* 4. load */
    EventSyncee* events =0l;
    TodoSyncee* todos = 0l;
    if (evSyncee )
       events = loadEvents( path, timeZoneId );
    if (toSyncee )
        todos = loadTodos( path, timeZoneId );

    /* 5. meta data */
    if ( met )
        doMeta( events, todos, meta );
    //else {
    // set the sync MODE FIXME
    //}
    /* 6.  sync */
    Syncer sync( core()->syncUi(), core()->syncAlgorithm() );
    if (evSyncee ) {
        events->setSource( i18n("Organizer") );
        sync.addSyncee(evSyncee);
        sync.addSyncee(events);
        sync.sync();
        sync.clear();
    }
    if (toSyncee ) {
        todos->setSource( i18n("Todolist") );
        sync.addSyncee( toSyncee );
        sync.addSyncee( todos );
        sync.sync();
        sync.clear();
    }

    if ( confirmBeforeWriting() ) {
        switch ( KMessageBox::questionYesNo(0, i18n("Do you want to write back todolist and calendar?"), i18n("Save"),
                                            KStdGuiItem::save(), KStdGuiItem::dontSave() ) ) {
        case KMessageBox::No:{
            delete todos;
            delete events;
            done();
            return;

            break;
        }
        default:
            break;
        }
    }

    /* 7. write back meta */
    if ( met )
        writeMeta( events, todos, meta );

    /* 8. write back data */
    save( events, todos, path, timeZoneId );

    /* 8.1 take care of the IdHelpers.... */

    /* 9. */
    if (events)
      out.append( events );

    if (todos)
      out.append( todos );
    done();
}

//AddressBookSyncee* ANY
TodoSyncee* OrganizerPart::loadTodos( const QString& pa, const QString& timeZoneId ) {
    TodoSyncee* syncee = new TodoSyncee();
    KCal::CalendarLocal cal(timeZoneId);
    cal.load(path( Todo, pa ) );
    KCal::Todo::List todos = cal.rawTodos();
    if ( todos.isEmpty() ) {
        return syncee;
    }

    KCal::Todo::List::ConstIterator it;
    TodoSyncEntry* entry =0 ;
    for ( it = todos.begin(); it != todos.end(); ++it ) {
        entry = new TodoSyncEntry( (*it)->clone() ) ;
        syncee->addEntry( entry );
    }
    return syncee;
}
EventSyncee* OrganizerPart::loadEvents(const QString& pa,  const QString& timeZoneId) {
    EventSyncee* syncee = new EventSyncee();
    KCal::CalendarLocal cal(timeZoneId);
    cal.load( path(Calendar, pa) );
    KCal::Event::List events = cal.rawEvents();
    if ( events.isEmpty() ) {
        return syncee;
    }

    KCal::Event::List::ConstIterator it;
    EventSyncEntry* entry;
    for ( it = events.begin(); it != events.end(); ++it ) {
        entry = new EventSyncEntry( (*it)->clone() );
        syncee->addEntry( entry );
        kdDebug(5222) << "Start Date of loaded " <<  entry->incidence()->dtStart().toString() << " " << entry->incidence()->uid() << endl;
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
        if (evSyncee ) {
            evSyncee->setFirstSync( true );
            evSyncee->setSyncMode( Syncee::MetaMode );
        }
        if (toSyncee) {
            toSyncee->setFirstSync( true );
            toSyncee->setSyncMode( Syncee::MetaMode );
        }
        return;
    }
    KSimpleConfig conf( str );

    /* check if event and or todosyncee are valid */
    if (evSyncee )
        doMetaIntern(evSyncee, &conf, "events-" );

    if (toSyncee )
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
            kdDebug(5222) << "OrganizerPart Meta Gathering: "
                      << id << endl;

            /*
             * the previous saved list of ids
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
                    syncee->addEntry( entry );
                    entry->setState( SyncEntry::Removed );

                }else { // organizer
                    KCal::Event* ev = new KCal::Event();
                    ev->setUid( id );
                    EventSyncEntry* entry = new EventSyncEntry( ev );
                    syncee->addEntry( entry );
                    entry->setState( SyncEntry::Removed );
                    kdDebug(5222) << "Removed " << entry->id() << endl;
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
        KonnectorProfile kon = core()->currentKonnectorProfile();
        QDir dir;
        dir.mkdir( QDir::homeDirPath() + "/.kitchensync");
        dir.mkdir( QDir::homeDirPath() + "/.kitchensync/meta");
        dir.mkdir( QDir::homeDirPath() + "/.kitchensync/meta/konnector-" + kon.uid() );
    }
    KSimpleConfig conf( str );
    /* clear config before */
    QStringList groups = conf.groupList();
    QStringList::Iterator it;
    for (it = groups.begin(); it != groups.end(); ++it ) {
        conf.deleteGroup( (*it) );
    }
    if (evSyncee )
        writeMetaIntern( evSyncee, &conf, "events-");
    if (toSyncee)
        writeMetaIntern( toSyncee, &conf, "todos-");
}
void OrganizerPart::writeMetaIntern( Syncee* syncee,
                                     KSimpleConfig* conf,
                                     const QString& key ) {
    SyncEntry* entry;
    for (entry = syncee->firstEntry(); entry; entry= syncee->nextEntry() ) {
        if (entry->state() == SyncEntry::Removed )
            continue;

        conf->setGroup( key + entry->id() );
        conf->writeEntry( "time", entry->timestamp() );
    }
}
void OrganizerPart::save( EventSyncee* evSyncee,
                          TodoSyncee* toSyncee,
                          const QString& pa,
                          const QString& timeZoneId) {
    KCal::CalendarLocal* loc = new KCal::CalendarLocal(timeZoneId);
    EventSyncEntry* evEntry=0l;
    TodoSyncEntry* toEntry=0l;
    if (evSyncee) {
      for ( evEntry = (EventSyncEntry*)evSyncee->firstEntry();
            evEntry;
            evEntry = (EventSyncEntry*)evSyncee->nextEntry() )
      {
          if (evEntry->state() != SyncEntry::Removed )
              loc->addEvent((KCal::Event*)evEntry->incidence()->clone()  );
      }
    }

    /*
     * For Evolution we will save here
     */
    if (isEvolutionSync() ) {
        loc->save( path( Calendar, pa ) );
        delete loc;
        loc = new KCal::CalendarLocal( timeZoneId );
    }

    if (toSyncee){
      for ( toEntry = (TodoSyncEntry*)toSyncee->firstEntry();
            toEntry;
            toEntry = (TodoSyncEntry*)toSyncee->nextEntry() )
      {
          if (toEntry->state() != SyncEntry::Removed )
              loc->addTodo((KCal::Todo*)toEntry->todo()->clone() );
      }
    }

    loc->save( path( Todo, pa)  );

    /* WE NEED TO LEAK HERE CAUSE CLONE SEAMS TO FAIL
     * I'll INVESTIGATE THIS FURTHER
     * FIXME let the libkonnector be more verbose
     * let us tell us if it synced successfully or not
     * there we could also delete the calendar then....
     * More Info now:
     * We add these incidence to the calendar and it want to take ownership
     * the CalendarLocals internal QPtrList is set to AutoDelete
     * which will delete even our todos..
     * so we need to clone before adding it.
     * FIXED
     */
    delete loc;
}

/*
 * we will return the right path
 * for the right calendar system!
 */
QString OrganizerPart::path( Data d, const QString& path ) {
    if ( !isEvolutionSync() ) {
        kdDebug(5222) << "Syncing with KDE and not Evolution " << endl;
        return path;
    }

    QString str;
    switch(d) {
    case Calendar:
        str = QDir::homeDirPath() + "/evolution/local/Calendar/calendar.ics";
        break;
    case Todo:
    default:
        str = QDir::homeDirPath()+ "/evolution/local/Tasks/tasks.ics";
        break;
    }
    return str;
}

#include "organizerpart.moc"
