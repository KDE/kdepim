// $Id$

#include <qregexp.h>

#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "syncalgorithm.h"
#include "standardsync.h"
#include "syncui.h"

#include "syncer.h"

using namespace KSync;

SyncEntry::SyncEntry( Syncee *sync) :
  mSyncee(sync)
{
    mState = Undefined;
    mSyncState = Undefined;
}
SyncEntry::SyncEntry( const SyncEntry& ent) {
    kdDebug(5230) << "SyncEntry copy c'tor " << endl;
    mState = ent.mState;
    mSyncee = ent.mSyncee;
    mSyncState = ent.mSyncState;
}
SyncEntry::~SyncEntry()
{
}

void SyncEntry::setSyncee(Syncee *syncee)
{
  mSyncee = syncee;
}
int SyncEntry::match( SyncEntry* /*entry*/ ) {
    return -2;
}
int SyncEntry::compareTo(SyncEntry* /*entry*/ ) {
    return -2;
}
int SyncEntry::state() const {
    return mState;
}
bool SyncEntry::wasAdded() const {
    return ( mState == Added );
}
bool SyncEntry::wasModified() const {
    return ( mState == Modified );
}
bool SyncEntry::wasRemoved() const {
    return ( mState == Removed );
}
void SyncEntry::setState( int state ) {
    kdDebug(5230) << "State is " << state << endl;
    mState = state;
}
void SyncEntry::setSyncState( int state ) {
    mSyncState = state;
}
int SyncEntry::syncState()const {
    return mSyncState;
}
Syncee *SyncEntry::syncee()
{
  return mSyncee;
}
/* not implemented here */
void SyncEntry::setId( const QString& ) {

}
bool SyncEntry::mergeWith( SyncEntry* ) {
    return false;
}
///// Syncee ////////////////
Syncee::Syncee(uint size) :
  mStatusLog(0), mSupport( size )
{
    mSyncMode = MetaLess;
    mFirstSync = false;
    mSupport.fill(true);
    kdDebug(5230) << "Size is " << size << " " << mSupport.size() << endl;
}

Syncee::~Syncee()
{
  delete mStatusLog;
}

void Syncee::setFilename(const QString &filename)
{
  mFilename = filename;
}

QString Syncee::filename()
{
  return mFilename;
}

SyncEntry *Syncee::findEntry(const QString &id)
{
  kdDebug(5231) << "Syncee::findEntry() '" << id << "'" << endl;

  SyncEntry *entry = firstEntry();
  while (entry) {
    if (entry->id() == id) return entry;
    entry = nextEntry();
  }

  return 0;
}

void Syncee::replaceEntry(SyncEntry *oldEntry, SyncEntry *newEntry)
{
  removeEntry(oldEntry);
  addEntry(newEntry);
}

bool Syncee::hasChanged(SyncEntry *entry)
{
  if ( entry->state() != SyncEntry::Undefined ) return true;
  if ( entry->timestamp().isEmpty() ) return false; // sure -zecke

  if (!mStatusLog ) return false;
  mStatusLog->setGroup(entry->id());
  QString timestamp = mStatusLog->readEntry("Timestamp");

  return (timestamp != entry->timestamp());
}

bool Syncee::load()
{
  delete mStatusLog;
  mStatusLog = new KSimpleConfig(locateLocal("appdata",statusLogName()));

  return read();
}

bool Syncee::save()
{
  bool success = write();
  if (success) {
    writeLog();
    return true;
  } else {
    return false;
  }
}

void Syncee::writeLog()
{
  if (!mStatusLog ) return;
  for (SyncEntry *entry = firstEntry();entry;entry = nextEntry()) {
    mStatusLog->setGroup(entry->id());
    mStatusLog->writeEntry("Name",entry->name());
    mStatusLog->writeEntry("Timestamp",entry->timestamp());
  }

  mStatusLog->sync();
}

QString Syncee::statusLogName()
{
  QString name = filename();

  name.replace(QRegExp("/"),"_");
  name.replace(QRegExp(":"),"_");

  name += ".syncee";

  return name;
}
int Syncee::modificationState( SyncEntry* entry ) const {
    return entry->state();
}
int Syncee::syncMode() const {
    return mSyncMode;
}
void Syncee::setSyncMode( int mode ) {
    mSyncMode = mode;
}
bool Syncee::firstSync() const {
    return mFirstSync;
}
void Syncee::setFirstSync( bool first ) {
    mFirstSync = first;
}
void Syncee::insertId( const QString &type,
                          const QString &konnectorId,
                          const QString &kdeId )
{
    QMap<QString,  Kontainer::ValueList>::Iterator it;
    it = mMaps.find( type );
    if ( it == mMaps.end() ) { // not inserted yet anything
        Kontainer::ValueList list;
        list.append( Kontainer(konnectorId,  kdeId) );
        mMaps.replace( type, list);
    }else {
        it.data().append(Kontainer( konnectorId,  kdeId) );
    }
}
Kontainer::ValueList Syncee::ids(const QString &type )const
{
    Kontainer::ValueList id;
    QMap<QString,  Kontainer::ValueList >::ConstIterator it;
    it = mMaps.find( type );
    if ( it != mMaps.end() )
        id = it.data();
    return id;
}
QMap<QString, Kontainer::ValueList> Syncee::ids() const {
    return mMaps;
}
bool Syncee::trustIdsOnFirstSync()const {
    return false;
}
QString Syncee::newId() const {
    return QString::null;
}
void Syncee::setSupports( const QBitArray& ar) {
    mSupport = ar;
    mSupport.detach();
    kdDebug(5230) << "setSupports count is " << ar.size() << endl;
}
QBitArray Syncee::bitArray()const {
    return mSupport;
}
bool Syncee::isSupported( uint attr )const {
    if ( attr >= mSupport.size() )
        return false;
    return mSupport.testBit( attr );
}
////////////// Syncer //////////////////////
Syncer::Syncer(SyncUi *ui,  SyncAlgorithm *iface)
{
//  mSyncees.setAutoDelete(true); this leads to crashes
  if (!ui) {
    mUi = new SyncUi();
  } else {
    mUi = ui;
  }
  if (iface == 0 )
      mInterface = new StandardSync( mUi );
  else
      mInterface = iface;

}

Syncer::~Syncer()
{
    delete mUi;
    delete mInterface;
}

void Syncer::addSyncee(Syncee *syncee)
{
  mSyncees.append(syncee);
}

void Syncer::clear() {
    mSyncees.clear();
}

void Syncer::sync()
{
  Syncee *target = mSyncees.last();
  Syncee *syncee = mSyncees.first();
  while (syncee != target) {
    syncToTarget(syncee,target);
    syncee = mSyncees.next();
  }
  target->save();
  syncee = mSyncees.first();
  while (syncee != target) {
    syncToTarget(target,syncee,true);
    syncee->save();
    syncee = mSyncees.next();
  }
}

void Syncer::syncAllToTarget(Syncee *target, bool writeback)
{
  Syncee *syncee = mSyncees.first();
  while(syncee) {
    syncToTarget(syncee,target);
    syncee = mSyncees.next();
  }

  target->writeLog();

  if (writeback) {
    for (Syncee *syncee=mSyncees.first();syncee;syncee = mSyncees.next()) {
      syncToTarget(target,syncee,true);
    }
  }
}

void Syncer::syncToTarget(Syncee *source, Syncee *target, bool override)
{
    mInterface->syncToTarget( source, target, override );
}

void Syncer::setSyncAlgorithm( SyncAlgorithm* iface ) {
    if ( iface == 0 )
        return;
    delete mInterface;
    mInterface = iface;
}
