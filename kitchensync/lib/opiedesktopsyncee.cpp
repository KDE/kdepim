
// $Id$

/**
 * $Log$
 * Revision 1.2  2002/07/17 15:16:54  zecke
 * API addition to Syncee ( firstSync ) not discussed yet
 * make the Syncee use the new call
 * put plugins back  into the Makefile but removed phone from there
 * cause it wasn't ported yet
 *
 * Revision 1.1  2002/07/15 20:15:19  zecke
 * Ported OpieDesktopSyncEntry
 *
 */


#include "opiedesktopsyncee.h"


using namespace KSync;

OpieDesktopSyncEntry::OpieDesktopSyncEntry( const QStringList& category,
                                            const QString& file,
                                            const QString& name,
                                            const QString& type,
                                            const QString& size )
    : SyncEntry(), mCategory( category ),  mFile( file ),
      mName(name ), mType( type ), mSize( size )
{

}
OpieDesktopSyncEntry::OpieDesktopSyncEntry( const OpieDesktopSyncEntry& opie )
    : SyncEntry( opie )
{
    mName = opie.mName;
    mType = opie.mType;
    mSize = opie.mSize;
    mFile = opie.mFile;
    mCategory = opie.mCategory;
}
OpieDesktopSyncEntry::~OpieDesktopSyncEntry() {
// nothing here
}
QString OpieDesktopSyncEntry::name()  {
    return mName;
}
QString OpieDesktopSyncEntry::file()const {
    return mFile;
}
QString OpieDesktopSyncEntry::fileType()const {
    return mType;
}
QString OpieDesktopSyncEntry::size()const {
    return mSize;
}
QStringList OpieDesktopSyncEntry::category() const {
    return mCategory;
}
QString OpieDesktopSyncEntry::id() {
    return mFile;
}
QString OpieDesktopSyncEntry::type() const {
    return QString::fromLatin1("OpieDesktopSyncEntry");
}
QString OpieDesktopSyncEntry::timestamp(){
    return QString::null;
}
bool OpieDesktopSyncEntry::equals( SyncEntry* entry ) {
    OpieDesktopSyncEntry* opEntry;
    opEntry = dynamic_cast<OpieDesktopSyncEntry*> (entry );
    if (opEntry == 0 )
        return false;
    if ( mFile == opEntry->mFile &&
         mName == opEntry->mName &&
         mType == opEntry->mType &&
         mSize == opEntry->mSize &&
         mCategory == opEntry->mCategory )
        return true;
    else
        return false;

}
SyncEntry* OpieDesktopSyncEntry::clone() {
    return new OpieDesktopSyncEntry( *this );
}
OpieDesktopSyncee::OpieDesktopSyncee()
    : Syncee()
{
    mList.setAutoDelete( true );
}
OpieDesktopSyncee::~OpieDesktopSyncee() {

}
QString OpieDesktopSyncee::type() const {
    return QString::fromLatin1("OpieDesktopSyncee");
}
Syncee* OpieDesktopSyncee::clone() {
    OpieDesktopSyncee* syncee = new OpieDesktopSyncee();
    syncee->setSyncMode( syncMode() );
    syncee->setFirstSync( firstSync() );
    syncee->setSupports( bitArray() );
    syncee->setSource( source() );
    OpieDesktopSyncEntry* entry;
    for ( entry = mList.first(); entry != 0; entry =mList.next() ) {
        syncee->addEntry( entry->clone() );
    }
    return syncee;
}
bool OpieDesktopSyncee::read() {
    return true;
}
bool OpieDesktopSyncee::write() {
    return true;
}
void OpieDesktopSyncee::addEntry( SyncEntry* entry ) {
    OpieDesktopSyncEntry* opEntry;
    opEntry = dynamic_cast<OpieDesktopSyncEntry*> (entry );
    if (opEntry == 0l )
        return;
    opEntry->setSyncee( this);
    mList.append( opEntry );
}
void OpieDesktopSyncee::removeEntry( SyncEntry* entry ) {
    OpieDesktopSyncEntry* opEntry;
    opEntry = dynamic_cast<OpieDesktopSyncEntry*> (entry );
    if ( opEntry == 0l )
        return;
    mList.remove( opEntry ); // is the case useless?
}
SyncEntry* OpieDesktopSyncee::firstEntry() {
    return mList.first();
}
SyncEntry* OpieDesktopSyncee::nextEntry() {
    return mList.next();
}
SyncEntry::PtrList OpieDesktopSyncee::added() {
    return voidi();
}
SyncEntry::PtrList OpieDesktopSyncee::modified() {
    return voidi();
}
SyncEntry::PtrList OpieDesktopSyncee::removed() {
    return voidi();
}
SyncEntry::PtrList OpieDesktopSyncee::voidi() {
    SyncEntry::PtrList list;
    return list;
}
