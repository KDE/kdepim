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
    void cloneList(const QPtrList<KCal::Todo>& old,  QPtrList<KCal::Todo> &clone ) {
        KCal::Todo* dummy;
        QPtrListIterator<KCal::Todo> it( old );
        for ( ; it.current(); ++it ) {
            dummy = it.current();
            clone.append( (KCal::Todo*)dummy->clone() );
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
                               KSyncEntry *in,
                               KSyncEntry * out )
{
    m_mode = mode;
    if ( in->type() != QString::fromLatin1("KAlendarSyncEntry") )
        return SyncReturn();
    if ( out->type() != QString::fromLatin1("KAlendarSyncEntry") )
        return SyncReturn();

    KAlendarSyncEntry* entry1= (KAlendarSyncEntry*) in;
    KAlendarSyncEntry* entry2= (KAlendarSyncEntry*) out;

//    kdDebug() << "Entry1 " << entry1->syncMode() << endl;
//    kdDebug() << "Entry2 " << entry2->syncMode() << endl;
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
    KSyncEntry::List synced;
    KSyncEntry::List in2;
    KSyncEntry::List out2;
    synced.append( m_entry );
    SyncReturn ret( synced,  in2, out2 );
    return ret;

}
void SyncKalendar::syncAsync( int mode,
                              KSyncEntry *in,
                              KSyncEntry *out )
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
                blackIds1 << dummy2->uid();
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
                blackIds1 << dummy->uid();
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
    list1.setAutoDelete( TRUE );
    list2.setAutoDelete( TRUE );
    mod1.setAutoDelete( TRUE );
    mod2.setAutoDelete( TRUE );
    rem1.setAutoDelete( TRUE );
    rem2.setAutoDelete( TRUE );
    list1.clear();
    list2.clear();
    mod1.clear();
    mod2.clear();
    rem1.clear();
    rem2.clear();
}
void SyncKalendar::syncMetaTodo( KAlendarSyncEntry* entry1,  KAlendarSyncEntry* entry2 )
{
    blackIds2.clear();
    KCal::Todo* dummy;
    QPtrList<KCal::Todo> dummyList = entry1->calendar()->getTodoList();
    QPtrList<KCal::Todo> list1;
    cloneList( dummyList,  list1 );
    dummyList.clear();

    QPtrList<KCal::Todo> list2;
    dummyList = entry2->calendar()->getTodoList();
    cloneList( dummyList,  list2 );
    dummyList.clear();

    QPtrList<KCal::Todo> added1;
    dummyList = entry1->added()->getTodoList();
    cloneList( dummyList,  added1 );
    dummyList.clear();

    QPtrList<KCal::Todo> added2;
    dummyList = entry2->added()->getTodoList();
    cloneList( dummyList,  added2 );
    dummyList.clear();

    QPtrList<KCal::Todo> mod1;
    dummyList = entry1->modified()->getTodoList();
    cloneList( dummyList,  mod1 );
    dummyList.clear();

    QPtrList<KCal::Todo> mod2;
    dummyList = entry2->modified()->getTodoList();
    cloneList( dummyList,  mod2 );
    dummyList.clear();

    QPtrList<KCal::Todo> rem1;
    dummyList = entry1->removed()->getTodoList();
    cloneList( dummyList,  rem1 );
    dummyList.clear();

    QPtrList<KCal::Todo> rem2;
    dummyList = entry2->removed()->getTodoList();
    cloneList( dummyList,  rem2 );

    // ok same as above
    // first added
    // the modified
    // then fill the black list
    // then add the remaining apps
    syncAdded( added1,  added2 );
    syncAdded( added2,  added1 );
    added1.setAutoDelete( TRUE );
    added2.setAutoDelete( TRUE );
    added1.clear();
    added2.clear();

    // modified and removed
    KCal::Todo *dummy2;
    bool found;
    for ( dummy = mod1.first(); dummy != 0; dummy = mod1.next() ) {
        found  = false;
        for ( dummy2 = mod2.first(); dummy2 != 0; dummy2 = mod2.next() ) {
            if ( dummy2->uid() == dummy->uid() ) { // modified on both conflict resolve
                found = true;
                m_entry->calendar()->addTodo( (KCal::Todo*) dummy->clone() );
                blackIds2 << dummy->uid();
                mod2.remove( dummy2 );
                delete dummy2;
                break;
            }
        }
        for ( dummy2 = rem2.first(); dummy2 != 0; dummy2 = rem2.next() ) {
            if (dummy2->uid() == dummy->uid() ) {
                found = true;
                blackIds2 << dummy->uid();
                m_entry->calendar()->addTodo( (KCal::Todo*) dummy->clone() );
                rem2.remove( dummy2 );
                delete dummy2;
                break;
            }
            if (!found ) {
                m_entry->calendar()->addEvent( (KCal::Event*) dummy->clone() );
                blackIds2 << dummy->uid();
            }
        }
    }
 // ok now the 2nd modified it could be only deleted
    for ( dummy = mod2.first(); dummy != 0; dummy = mod2.next() ) {
        found = false;
        for ( dummy2 = rem1.first(); dummy2 != 0; dummy2 = rem1.next() ) {
            if ( dummy2->uid() == dummy->uid() ) {
                found = true;
                m_entry->calendar()->addTodo( (KCal::Todo*)dummy->clone() );
                blackIds2 << dummy->uid();
                rem1.remove( dummy2 );
                delete dummy2;
                break;
            }
        }
        if (!found ) {
                blackIds2 << dummy->uid();
                m_entry->calendar()->addTodo( (KCal::Todo*) dummy->clone() );
        }
    }
    // removed now
    for (dummy = rem1.first(); dummy != 0; dummy = rem1.next() ) {
        blackIds2 << dummy->uid();
    }
    for (dummy = rem2.first(); dummy!= 0; dummy = rem2.next() ) {
        blackIds2 << dummy->uid();
    }
    // now copy from list1 to m_entry->calendar() if uid is not on blackList
    for ( dummy = list1.first(); dummy != 0; dummy= list1.next() ) {
        if (!blackIds2.contains( dummy->uid() ) )
            m_entry->calendar()->addTodo( (KCal::Todo*) dummy->clone() );
    }
    // now clean up the lists
    list1.setAutoDelete( TRUE );
    list2.setAutoDelete( TRUE );
    mod1.setAutoDelete( TRUE );
    mod2.setAutoDelete( TRUE );
    rem1.setAutoDelete( TRUE );
    rem2.setAutoDelete( TRUE );
    list1.clear();
    list2.clear();
    mod1.clear();
    mod2.clear();
    rem2.clear();
    rem1.clear();
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
    KCal::Event *one=0;
    KCal::Event *two=0;
    QPtrListIterator<KCal::Event> oneIt( eventsOne );
    QPtrListIterator<KCal::Event> twoIt( eventsTwo );
    // first go through entry1 and then through entry2
    for ( ; oneIt.current(); ++oneIt ) {
        one = oneIt.current();
//        kdDebug() << "ADD one " << one->uid() << endl;
        if ( one->uid().startsWith("Konnector-") ) { // really new one
            QString id = KCal::CalFormat::createUniqueId();
//            kdDebug() << "UID " << one->uid() << endl;
            m_entry->insertId( "event", one->uid(),  id );
            //one->setUid( id );
            KCal::Event* clone = (KCal::Event*) one->clone();
            if ( clone == 0 ) ;
//                kdDebug() << "Clone == 0 "<< endl;
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
//                    kdDebug() << "FIXME" << endl;
                    QString text = i18n("Which entry do you want to take precedence?\n");
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
//        kdDebug() << "Adding todo " << one->uid();
        if ( one->uid().startsWith("Konnector-") ) { // really new one
//            kdDebug() << "UID Todo " << one->uid();
            QString id = KCal::CalFormat::createUniqueId();
            m_entry->insertId( "todo", one->uid(),  id );
            //one->setUid( id );
            KCal::Todo* clone = (KCal::Todo*) one->clone();
            if ( clone == 0 ) ;
//                kdDebug() << "Clone == 0 "<< endl;
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
                    m_entry->calendar()->addTodo( (KCal::Todo*) one->clone() );
                    m_entry->calendar()->addTodo( (KCal::Todo*) two->clone() );
                    break;
                case SYNC_INTERACTIVE:
                default: {
                    kdDebug() << "FIXME" << endl;
                    QString text = i18n("Which entry do you want to take precedence?\n");
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
    // All duplicated entries are resolved by now
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
void SyncKalendar::syncAdded( const QPtrList<KCal::Todo> &added,  const QPtrList<KCal::Todo> &/*other*/ ) {
    QPtrListIterator<KCal::Todo> it( added );
    KCal::Todo *dummy;
    KCal::Todo *clone;
    for ( ;it.current(); ++it ) {
        dummy = it.current();
        clone = (KCal::Todo*) dummy->clone();
        if ( dummy->uid().startsWith("Konnector-") ) {
            QString id = KCal::CalFormat::createUniqueId();
            m_entry->insertId("todo",  dummy->uid(),  id );
            clone->setUid( id );
        }
        blackIds2 << dummy->uid();
        m_entry->calendar()->addTodo( clone );
    }
}
