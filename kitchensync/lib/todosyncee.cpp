#include <calformat.h>
#include "todosyncee.h"


using namespace KSync;

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
/// Syncee
TodoSyncee::TodoSyncee()
    : SyncTemplate<TodoSyncEntry>() {
};
QString TodoSyncee::type() const {
    return QString::fromLatin1("TodoSyncee");
}
Syncee* TodoSyncee::clone() {
    TodoSyncee* temp = new TodoSyncee();
    temp->setSyncMode( syncMode() );
    temp->setFirstSync( firstSync() );
    TodoSyncEntry* entry;
    for ( entry = mList.first(); entry != 0; entry = mList.next() ) {
        temp->addEntry( entry->clone() );
    }
    return temp;
}
QString TodoSyncee::newId() const {
return KCal::CalFormat::createUniqueId();
}
