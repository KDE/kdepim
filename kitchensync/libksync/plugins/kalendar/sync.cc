#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>

#include <libkcal/calformat.h>

#include "ksync_sync.h"
#include "sync.h"

using namespace  KitchenSync;


namespace {

    void cloneList(const QPtrList<KCal::Event>& old, QPtrList<KCal::Event> &clone  ) {
        KCal::Event* dummy;
        QPtrListIterator<KCal::Event> it(old );
        for ( ; it.current(); ++it ) {
            dummy = it.current();
            clone.append( (KCal::Event*)dummy->clone()  );
        }
    }
};

SyncKalendar::SyncKalendar( QObject *obj, const char *name,  const QStringList & )
    : SyncPlugin( obj,  name )
{

}
SyncKalendar::~SyncKalendar()
{

}
// incomplete hack FIXME
// FIXME only 1 Entry at a time
// FIXME normal mode don't just copy like stupid
SyncReturn SyncKalendar::sync( int mode,
                               const QPtrList<KSyncEntry> &in,
                               const QPtrList<KSyncEntry> &out )
{
    m_mode = mode;
    QPtrListIterator<KSyncEntry> itIn( in );
    QPtrListIterator<KSyncEntry> outIt( out );
    KAlendarSyncEntry* entry1=0;
    KAlendarSyncEntry* entry2=0;
    KSyncEntry* entry;
    for ( ; itIn.current(); ++itIn ) {
        entry = itIn.current();
        if ( entry->type() == QString::fromLatin1("KAlendarSyncEntry") ) {
            entry1 = (KAlendarSyncEntry*) entry;
            break;
        }
    }
    for ( ; outIt.current(); ++outIt ) {
        entry = outIt.current();
        if ( entry->type() == QString::fromLatin1("KAlendarSyncEntry") ) {
            entry2 = (KAlendarSyncEntry*) entry;
            break;
        }
    }
    kdDebug() << "Entry1 " << entry1->syncMode() << endl;
    kdDebug() << "Entry2 " << entry2->syncMode() << endl;
    KCal::CalendarLocal *cal = new KCal::CalendarLocal();
    m_entry = new KAlendarSyncEntry();
    m_entry->setCalendar( cal );
    if ( entry2->syncMode() == KSyncEntry::SYNC_NORMAL && entry1->syncMode() == KSyncEntry::SYNC_NORMAL ) {
        syncNormal( entry1,  entry2 );
        syncTodo( entry1,  entry2 );
    }else if ( entry2->syncMode() == KSyncEntry::SYNC_NORMAL && entry1->syncMode() == KSyncEntry::SYNC_META ) {
        syncNormal( entry1, entry2 );
        syncTodo( entry1, entry2 );
    }else if ( entry2->syncMode() == KSyncEntry::SYNC_META && entry1->syncMode() == KSyncEntry::SYNC_NORMAL ) {
        syncNormal( entry1, entry2 );
        syncTodo( entry1, entry2 );
    }else if ( entry2->syncMode() == KSyncEntry::SYNC_META && entry1->syncMode() == KSyncEntry::SYNC_META ) {
        if ( entry2->firstSync() || entry1->firstSync() ) {
            m_mode = 10;
            syncNormal( entry1, entry2 );
            syncTodo( entry1, entry2 );
        }else {
            syncMetaEvent( entry1, entry2 );
            syncMetaTodo( entry1,  entry2 );
        }
    }
}
void SyncKalendar::syncAsync( int mode,
                              const QPtrList<KSyncEntry> &in,
                              const QPtrList<KSyncEntry> &out )
{

}
void SyncKalendar::syncMetaEvent( KAlendarSyncEntry* entry1,  KAlendarSyncEntry *entry2 )
{
    // clone to be independant from entryX->calendar();
    KCal::Event* dummy;
    blackIds1.clear();
    QPtrList<KCal::Event> dummyList = entry1->calendar()->getAllEvents();
    QPtrList<KCal::Event> list1;
    cloneList( dummyList, list1 );
    dummyList.clear();

    dummyList = entry2->calendar()->getAllEvents();
    QPtrList<KCal::Event> list2;
    cloneList( dummyList,  list2 );
    dummyList.clear();

    dummyList =entry1->added()->getAllEvents();
    QPtrList<KCal::Event> added1;
    cloneList( dummyList,  added1 );
    dummyList.clear();

    dummyList = entry2->added()->getAllEvents();
    QPtrList<KCal::Event> added2;
    cloneList( dummyList,  added2 );
    dummyList.clear();

    dummyList =entry1->modified()->getAllEvents();
    QPtrList<KCal::Event> mod1;
    cloneList( dummyList,  mod1 );
    dummyList.clear();

    dummyList = entry2->modified()->getAllEvents();
    QPtrList<KCal::Event> mod2 ;
    cloneList( dummyList,  mod2 );
    dummyList.clear();

    dummyList = entry1->removed()->getAllEvents();
    QPtrList<KCal::Event> rem1;
    cloneList( dummyList,  rem1 );
    dummyList.clear();

    dummyList =entry2->removed()->getAllEvents();
    QPtrList<KCal::Event> rem2;
    cloneList( dummyList,  rem2 );
    dummyList.clear();

    // we need to remove all modified and added entries from list1 and list2
    // if modified on one we need to remove from list1 and list2
    // /////////////////// added entries
    syncAdded( added1,  added2 );
    syncAdded(added2, added1 );
    added1.setAutoDelete ( TRUE );
    added2.setAutoDelete( TRUE );
    added1.clear();
    added2.clear();
    // //////////////////////// modified and removed //////////////////////
    // ////////// Tactic //////////////////////////////////////////////////
    // modified -> check if it is modified in the other -> deconflict
    //          -> check if removed  -> deconflict
    //          -> apply
    KCal::Event *dummy2;
    KCal::Event *dummy3;
    for ( dummy = mod1.first(); dummy != 0; dummy = mod1.next() ) {
        bool found = false;
        for ( dummy2 = mod2.first(); dummy2 != 0; dummy2 = mod2.next() ) { // both modified
            if ( dummy->uid() == dummy2->uid() ) { // both modified deconflict
                found = true;
                m_entry->calendar()->addEvent( (KCal::Event*) dummy->clone() );
                blackIds1 << dummy->uid();
                mod2.remove( dummy2 );
                delete dummy2;
                break;
                // deconflict
            }
        }
        for (dummy2 = rem2.first(); dummy2 != 0; dummy2 = rem2.next() ) { // modified and deleted
            if ( dummy2->uid() == dummy->uid() ) { // modified on Opie deleted on desktop
                // FIXME deconflict
                found = true;
                m_entry->calendar()->addEvent( (KCal::Event*) dummy->clone() );
                rem2.remove( dummy2 );
                delete dummy2;
                break;
            }
        }
        if (!found ) {
            m_entry->calendar()->addEvent( (KCal::Event*) dummy->clone() );
            blackIds1 << dummy->uid();
        }
    }
    // mod2
    for ( dummy = mod2.first(); dummy != 0; dummy = mod2.next() ) {
        bool found = false;
        for (dummy2 = rem1.first(); dummy2 != 0; dummy2 = rem1.next() ) { // modified and deleted
            if ( dummy2->uid() == dummy->uid() ) {
                // FIXME deconflict
                found = true;
                m_entry->calendar()->addEvent( (KCal::Event*) dummy->clone() );
                rem1.remove( dummy2 );
                delete dummy2;
                blackIds1 << dummy->uid();
                break;
            }
        }
        if (!found ) {
            m_entry->calendar()->addEvent( (KCal::Event*) dummy->clone() );
            blackIds1 << dummy->uid();
        }
    }
    // removed
    for ( dummy = rem1.first(); dummy != 0; dummy = rem1.next() ) {
        blackIds1 << dummy->uid();
    }
    for (dummy = rem2.first(); dummy != 0; dummy = rem2.next() ) {
        blackIds1 << dummy->uid();
    }
    // append the rest
    list1 = entry1->calendar()->getAllEvents();
    for ( dummy = list1.first(); dummy != 0; dummy = list1.next() ) {
        if (!blackIds1.contains( dummy->uid() ) )
            m_entry->calendar()->addEvent( (KCal::Event*) dummy->clone() );
    }
}
void SyncKalendar::syncMetaTodo( KAlendarSyncEntry* entry1,  KAlendarSyncEntry* entry2 )
{

}
// get all events assign new id if necessary
// this is plain stupid but take all Events from entry 1 and all from entry2
// in future it's better to check ids first
// and then search for similiaraties to match entries
// then replace 'new' with old
// Currently we search for the same entry id and try to resolve it
// or we add it
void SyncKalendar::syncNormal( KAlendarSyncEntry* entry1,  KAlendarSyncEntry* entry2 )
{
    QPtrList<KCal::Event> eventsOne = entry1->calendar()->getAllEvents();
    QPtrList<KCal::Event> eventsTwo = entry2->calendar()->getAllEvents();
    KCal::Event *one;
    KCal::Event *two;
    QPtrListIterator<KCal::Event> oneIt( eventsOne );
    QPtrListIterator<KCal::Event> twoIt( eventsTwo );
    // first go through entry1 and then through entry2
    for ( ; oneIt.current(); ++oneIt ) {
        one = oneIt.current();
        if ( one->uid().startsWith("Konnector-") ) { // really new one
            QString id = KCal::CalFormat::createUniqueId();
            m_entry->insertId( "event", one->uid(),  id );
            //one->setUid( id );
            KCal::Event* clone = (KCal::Event*) one->clone();
            clone->setUid( id );
            m_entry->calendar()->addEvent( clone );
            continue;
        }
        // find
        bool found = false;
        QPtrListIterator<KCal::Event> threeIt( eventsTwo );
        for ( ; threeIt.current(); ++threeIt ) {
            two = threeIt.current();
            if ( one->uid() ==  two->uid() ) { // found it
                found = true;
                // so dumb modification check just check mode first
                // then prompt or timestamp
                switch ( m_mode ) {
                case SYNC_FIRSTOVERRIDE:
                    m_entry->calendar()->addEvent( (KCal::Event*) one->clone() );
                    break;
                case SYNC_SECONDOVERRIDE:
                    m_entry->calendar()->addEvent( (KCal::Event*) two->clone() );
                    break;
                case 10:
                    m_entry->calendar()->addEvent( (KCal::Event*) one->clone() );
                    m_entry->calendar()->addEvent( (KCal::Event*) two->clone() );
                    break;
                case SYNC_INTERACTIVE:
                default: {
                    kdDebug() << "FIXME" << endl;
                    QString text = i18n("Which entry fo you want to take precedence?\n");
                    text += i18n("Entry 1: '%1'\n").arg( one->summary() );
                    text += i18n("Entry 2: '%1'\n").arg( two->summary() );
                    int result = KMessageBox::questionYesNo(0l,  text,  i18n("Resolve Conflict"),
                                                            i18n("Entry 1"),  i18n("Entry 2") );
                    if ( result == KMessageBox::Yes ) {
                        m_entry->calendar()->addEvent( (KCal::Event*) one->clone() );
                    }else if ( result == KMessageBox::No ) {
                        m_entry->calendar()->addEvent( (KCal::Event*) two->clone() );
                    }
                    break;
                };
                };
            }
        }
        if (!found )
            m_entry->calendar()->addEvent( (KCal::Event*) one->clone() );
    }
    for (; twoIt.current(); ++twoIt ) {
        bool found = false;
        two = twoIt.current();
        QPtrListIterator<KCal::Event> it( eventsOne );
        for ( ; it.current(); ++it ) {
            one = it.current();
            if ( one->uid() == two->uid() ) {
                found = true;
                break;
            }
        }
        if (!found )
            m_entry->calendar()->addEvent( (KCal::Event*) two->clone() );
    }
}
void SyncKalendar::syncTodo( KAlendarSyncEntry* entry1,  KAlendarSyncEntry* entry2 )
{
    const QPtrList<KCal::Todo> todosOne = entry1->calendar()->getTodoList();
    const QPtrList<KCal::Todo> todosTwo = entry2->calendar()->getTodoList();
    KCal::Todo *one;
    KCal::Todo *two;
    QPtrListIterator<KCal::Todo> oneIt( todosOne );
    QPtrListIterator<KCal::Todo> twoIt( todosTwo );
    // first go through entry1 and then through entry2
    for ( ; oneIt.current(); ++oneIt ) {
        one = oneIt.current();
        if ( one->uid().startsWith("Konnector-") ) { // really new one
            QString id = KCal::CalFormat::createUniqueId();
            m_entry->insertId( "todo", one->uid(),  id );
            //one->setUid( id );
            KCal::Todo* clone = (KCal::Todo*) one->clone();
            clone->setUid( id );
            m_entry->calendar()->addTodo( clone );
            continue;
        }
        // find
        bool found = false;
        QPtrListIterator<KCal::Todo> threeIt( todosTwo );
        for ( ; threeIt.current(); ++threeIt ) {
            two = threeIt.current();
            if ( one->uid() ==  two->uid() ) { // found it
                found = true;
                // so dumb modification check just check mode first
                // then prompt or timestamp
                switch ( m_mode ) {
                case SYNC_FIRSTOVERRIDE:
                    m_entry->calendar()->addTodo( (KCal::Todo*) one->clone() );
                    break;
                case SYNC_SECONDOVERRIDE:
                    m_entry->calendar()->addTodo( (KCal::Todo*) two->clone() );
                    break;
                case 10:
                    m_entry->calendar()->addEvent( (KCal::Event*) one->clone() );
                    m_entry->calendar()->addEvent( (KCal::Event*) two->clone() );
                    break;
                case SYNC_INTERACTIVE:
                default: {
                    kdDebug() << "FIXME" << endl;
                    QString text = i18n("Which entry fo you want to take precedence?\n");
                    text += i18n("Entry 1: '%1'\n").arg( one->description() );
                    text += i18n("Entry 2: '%1'\n").arg( two->description() );
                    int result = KMessageBox::questionYesNo(0l,  text,  i18n("Resolve Conflict"),
                                                            i18n("Entry 1"),  i18n("Entry 2") );
                    if ( result == KMessageBox::Yes ) {
                        m_entry->calendar()->addTodo( (KCal::Todo*) one->clone() );
                    }else if ( result == KMessageBox::No ) {
                        m_entry->calendar()->addTodo( (KCal::Todo*) two->clone() );
                    }
                    break;
                };
                };
            }
        }
        if (!found )
            m_entry->calendar()->addTodo( (KCal::Todo*) one->clone() );
    }
    for (; twoIt.current(); ++twoIt ) {
        bool found = false;
        two = twoIt.current();
        QPtrListIterator<KCal::Todo> it( todosOne );
        for ( ; it.current(); ++it ) {
            one = it.current();
            if ( one->uid() == two->uid() ) {
                found = true;
                break;
            }
        }
        if (!found )
            m_entry->calendar()->addTodo( (KCal::Todo*) two->clone() );
    }
}
// currently we just assign new id + add it to the calendar()
// in future we try to match the entry with other added entries
void SyncKalendar::syncAdded( const QPtrList<KCal::Event> &added,  const QPtrList<KCal::Event> &/*otherAdded*/ )
{
    QPtrListIterator<KCal::Event> it( added );
    KCal::Event *dummy;
    KCal::Event *clone;
    for ( ; it.current(); ++it ) {
        dummy = it.current();
        clone = (KCal::Event*) dummy->clone();
        if ( dummy->uid().startsWith("Konnector-") ) {
            QString id = KCal::CalFormat::createUniqueId();
            m_entry->insertId( "event", dummy->uid(),  id );
            clone->setUid(id);
        }
        blackIds1 << dummy->uid();
        m_entry->calendar()->addEvent( clone );
    }
}
