#include <kstaticdeleter.h>

#include <calformat.h>

#include <incidencetemplate.h>
#include "todosyncee.h"


using namespace KSync;
using KCal::Todo;
/* A test for the template
void testIt() {
    TodoSyncee* syncee = new TodoSyncee();
    syncee->setSyncMode( Syncee::FirstSync );
    delete syncee;
}
*/
TodoSyncEntry::TodoSyncEntry( KCal::Todo* todo )
    : SyncEntry(), mTodo( todo )
{
    if (!mTodo )
        mTodo = new KCal::Todo;
}
TodoSyncEntry::TodoSyncEntry( const TodoSyncEntry& entry)
    : SyncEntry( entry )
{
    mTodo = (KCal::Todo*)entry.mTodo->clone();
}
TodoSyncEntry::~TodoSyncEntry() {
    delete mTodo;
}
KCal::Todo* TodoSyncEntry::todo()  {
    return mTodo;
}
QString TodoSyncEntry::type() const {
    return QString::fromLatin1("TodoSyncEntry");
}
QString TodoSyncEntry::name() {
    return mTodo->summary();
}
QString TodoSyncEntry::id() {
    return mTodo->uid();
}
void TodoSyncEntry::setId(const QString& id ) {
    mTodo->setUid( id );
}
QString TodoSyncEntry::timestamp() {
    return mTodo->lastModified().toString();
}
SyncEntry* TodoSyncEntry::clone() {
    return new TodoSyncEntry( *this );
}
bool TodoSyncEntry::equals(SyncEntry* entry ) {
    TodoSyncEntry* todoEntry = dynamic_cast<TodoSyncEntry*> (entry );
    if (!todoEntry )
        return false;

    if (mTodo->uid() != todoEntry->todo()->uid() ) return false;
    if (mTodo->lastModified() != todoEntry->todo()->lastModified() ) return false;
    return true;
}
/* merging hell! */
namespace{
    typedef MergeBase<Todo, TodoSyncee> MergeTodo;
    static MergeTodo* mergeMap = 0l;
    static KStaticDeleter<MergeTodo> deleter;

    void mergeDue( Todo* const, const Todo* const );
    void mergeStart( Todo* const, const Todo* const );
    void mergeComp( Todo* const, const Todo* const );
    void mergePer( Todo* const, const Todo* const );

    MergeTodo* mapTo() {
        if (!mergeMap ) {
            deleter.setObject( mergeMap, new MergeTodo );

            mergeMap->add( TodoSyncee::DtDue, mergeDue );
            mergeMap->add( TodoSyncee::StartDate, mergeStart );
            mergeMap->add( TodoSyncee::Completed, mergeComp );
            mergeMap->add( TodoSyncee::Percent, mergePer );
        }
        return mergeMap;
    }

}
bool TodoSyncEntry::mergeWith( SyncEntry* entry ) {
    if ( entry->name() != name() || !syncee() || !entry->syncee() )
        return false;

    TodoSyncEntry* toEn = static_cast<TodoSyncEntry*>(entry);
    QBitArray da = toEn->syncee()->bitArray();
    QBitArray hier = syncee()->bitArray();
    for ( uint i = 0; i < da.size() && i < hier.size() ; i++ ) {
        if (da[i] && !hier[i] ) {
            mapTo()->invoke(i, mTodo, toEn->mTodo );
        }
    }

    return true;
}

/// Syncee
TodoSyncee::TodoSyncee()
    : SyncTemplate<TodoSyncEntry>(TodoSyncee::Percent+1) { // Percent is the last item
};
QString TodoSyncee::type() const {
    return QString::fromLatin1("TodoSyncee");
}
Syncee* TodoSyncee::clone() {
    TodoSyncee* temp = new TodoSyncee();
    temp->setSyncMode( syncMode() );
    temp->setFirstSync( firstSync() );
    temp->setSupports( bitArray() );
    temp->setSource( source() );
    TodoSyncEntry* entry;
    for ( entry = mList.first(); entry != 0; entry = mList.next() ) {
        temp->addEntry( entry->clone() );
    }
    return temp;
}
QString TodoSyncee::newId() const {
    return KCal::CalFormat::createUniqueId();
}

namespace {
    void mergeDue( Todo* const dest, const Todo* const src) {
        dest->setDtDue( src->dtDue() );
    }
    void mergeStart( Todo* const dest, const Todo* const src) {
        dest->setHasStartDate( src->hasStartDate() );
    }
    void mergeComp( Todo* const dest, const Todo* const src) {
        dest->setCompleted( src->isCompleted() );
    }
    void mergePer( Todo* const dest, const Todo* const src) {
        dest->setPercentComplete( src->percentComplete() );
    }
}
