
#ifndef KSYNC_INCEDENCE_TEMPLATE_H
#define KSYNC_INCEDENCE_TEMPLATE_H

#include "syncer.h"

#include <qmap.h>

#include <incidence.h>

/*
 * Internal helper!!!
 */

namespace KSync {
    template <class Entry = KCal::Incidence>
    class IncidenceTemplate : public SyncEntry {
    public:
        typedef QPtrList<Entry> PtrList;
        IncidenceTemplate(Entry* entry)
            : SyncEntry(), mIncidence( entry ) {
            if (!entry )
                mIncidence = new Entry;

        };
        IncidenceTemplate( const IncidenceTemplate& temp )
            : SyncEntry( temp ){
            mIncidence = (Entry*)temp.mIncidence->clone();

        }
        ~IncidenceTemplate() {
            delete mIncidence;
        }
        QString type() const { return mIncidence->type() + "SyncEntry"; }
        QString name() { return mIncidence->summary(); }
        QString id() { return mIncidence->uid(); }
        void setId(const QString& id) { mIncidence->setUid( id ); }
        QString timestamp() { return mIncidence->lastModified().toString(); }
        Entry* incidence() { return mIncidence; };
        bool equals( SyncEntry* entry) {
            IncidenceTemplate* inEntry = dynamic_cast<IncidenceTemplate*> (entry );
            if (!inEntry )
                return false;
            if (mIncidence->uid() != inEntry->incidence()->uid() ) return false;
            if (mIncidence->lastModified() != inEntry->incidence()->lastModified() )
                return false;

            return true;
        }
        SyncEntry* clone() {
            return new IncidenceTemplate<Entry>( *this );
        }
    private:
        Entry* mIncidence;

    };
   /*
    * common support map!!!
    */
    namespace Cal {
    template<class T> void mergeOrg( T* const, const T* const );
    template<class T> void mergeRead( T* const, const T* const );
    template<class T> void mergedtStart( T* const, const T* const );
    template<class T> void mergeDur( T* const, const T* const );
    template<class T> void mergeFloat( T* const, const T* const );
    template<class T> void mergeAttend( T* const, const T* const );
    template<class T> void mergeCreated( T* const, const T* const );
    template<class T> void mergeRev( T* const, const T* const );
    template<class T> void mergeDes( T* const, const T* const );
    template<class T> void mergeSum( T* const, const T* const );
    template<class T> void mergeCat( T* const, const T* const );
    template<class T> void mergeRel( T* const, const T* const );
    template<class T> void mergeExDates( T* const, const T* const );
    template<class T> void mergeAtt( T* const, const T* const );
    template<class T> void mergeSec( T* const,  const T* const );
    template<class T> void mergeRes( T* const,  const T* const );
    template<class T> void mergePrio( T* const, const T* const );
    template<class T> void mergeAlarm( T* const, const T* const );
    template<class T> void mergeRecur( T* const, const T* const );
    template<class T> void mergeLoc( T* const, const T* const );
    }

    template <class T,  class U>
    class MergeBase {
    public:
        MergeBase();
        virtual ~MergeBase();
        typedef void (*merge)(T* const, const T* const );
        typedef QMap<int, merge> MergeMap;
        typedef typename QMap<int, merge>::Iterator Iterator;
        void invoke( int, T* const, const T* const );
        void add( int, merge );
    protected:
        MergeMap map;
    };

    // Implementation

    // add the functions to the map
    template <class T, class U> MergeBase<T, U>::MergeBase() {
        map.insert( U::Organizer,  Cal::mergeOrg<T> );
        map.insert( U::ReadOnly,  Cal::mergeRead<T> );
        map.insert( U::DtStart,  Cal::mergedtStart<T> );
        map.insert( U::Duration,  Cal::mergeDur<T> );
        map.insert( U::Float,  Cal::mergeFloat<T> );
        map.insert( U::Attendee,  Cal::mergeAttend<T> );
        map.insert( U::CreatedDate,   Cal::mergeCreated<T> );
        map.insert( U::Revision,  Cal::mergeRev<T> );
        map.insert( U::Description,  Cal::mergeDes<T> );
        map.insert( U::Summary,  Cal::mergeSum<T> );
        map.insert( U::Category, Cal::mergeCat<T> );
        map.insert( U::Relations,  Cal::mergeRel<T> );
        map.insert( U::ExDates,  Cal::mergeExDates<T> );
        map.insert( U::Attachments,  Cal::mergeAtt<T> );
        map.insert( U::Secrecy,  Cal::mergeSec<T> );
        map.insert( U::Resources,  Cal::mergeRes<T> );
        map.insert( U::Priority,  Cal::mergePrio<T> );
        map.insert( U::Alarms,  Cal::mergeAlarm<T> );
        map.insert( U::Recurrence,  Cal::mergeRecur<T> );
        map.insert( U::Location,  Cal::mergeLoc<T> );
    }
    template <class T,  class U> MergeBase<T, U>::~MergeBase() {
    }
    template <class T,  class U> void MergeBase<T, U>::invoke(int i, T* const dest, const T* const src) {
        Iterator it= map.find( i );
        if ( it != map.end() )
            (*it.data())(dest, src );
    }
    template<class T, class U>
    void MergeBase<T, U>::add(int res, merge mer ) {
        map.insert( res, mer );
    }

    namespace Cal {
    // implementation of the merge functions
    template <class Todo> void mergeOrg( Todo* const dest, const Todo* const src) {
        dest->setOrganizer( src->organizer() );
    }
    template <class Todo> void mergeRead( Todo* const dest, const Todo* const src) {
        dest->setReadOnly( src->isReadOnly() );
    }
    template <class Todo> void mergedtStart( Todo* const dest, const Todo* const src) {
        dest->setDtStart( src->dtStart() );
    }
    template <class Todo> void mergeDur( Todo* const dest, const Todo* const src) {
        dest->setDuration( src->duration() );
    }
    template <class Todo> void mergeFloat( Todo* const dest, const Todo* const src) {
        dest->setFloats( src->doesFloat() );
    }
    template <class Todo>  void mergeAttend( Todo* const dest, const Todo* const src) {
        QPtrList<KCal::Attendee> att = src->attendees();
        KCal::Attendee* at;
        for ( at = att.first(); at; at = att.next() )
            dest->addAttendee( new KCal::Attendee( *at ) );
    }
    template <class Todo>  void mergeCreated( Todo* const dest, const Todo* const src) {
        dest->setCreated( src->created() );
    }
    template <class Todo> void mergeRev( Todo* const dest, const Todo* const src) {
        dest->setRevision( src->revision() );
    }
    template <class Todo> void mergeDes( Todo* const dest, const Todo* const src) {
        dest->setDescription( src->description() );
    }
    template <class Todo> void mergeSum( Todo* const dest, const Todo* const src) {
        dest->setSummary( src->summary() );
    }
    template <class Todo> void mergeCat( Todo* const dest, const Todo* const src) {
        dest->setCategories( src->categories() );
    }
    template <class Todo> void mergeRel( Todo* const dest, const Todo* const src) {
        QPtrList<KCal::Incidence> rel = src->relations();
        KCal::Incidence* in;
        for ( in = rel.first(); in; in = rel.next() ) {
            dest->addRelation( in->clone() );
        }
    }
    template <class Todo> void mergeExDates( Todo* const dest, const Todo* const src) {
        dest->setExDates( src->exDates() );
    }
    template <class Todo> void mergeAtt( Todo* const, const Todo* const ) {
    // FIXME!!!
    }
    template <class Todo> void mergeSec( Todo* const dest,  const Todo* const src) {
        dest->setSecrecy( src->secrecy() );
    }
    template <class Todo> void mergeRes( Todo* const dest,  const Todo* const src) {
        dest->setResources( src->resources() );
    }
    template <class Todo> void mergePrio( Todo* const dest, const Todo* const src) {
        dest->setPriority( src->priority() );
    }
    template <class Todo> void mergeAlarm( Todo* const dest, const Todo* const src ) {
        QPtrList<KCal::Alarm> als = src->alarms();
        KCal::Alarm* al;
        for (al = als.first(); al; al = als.next() ) {
            dest->addAlarm( new KCal::Alarm( (*al) ) );
        }
    }
    template <class Todo> void mergeRecur( Todo* const , const Todo* const ) {
         // not available
    }
    template <class Todo> void mergeLoc( Todo* const dest , const Todo* const src) {
        dest->setLocation( src->location() );
    }
    }
};


#endif
