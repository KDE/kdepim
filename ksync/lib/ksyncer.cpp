/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qregexp.h>

#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "ksyncui.h"

#include "ksyncer.h"

KSyncEntry::KSyncEntry() :
  mSyncee(0)
{
}

KSyncEntry::~KSyncEntry()
{
}

void KSyncEntry::setSyncee(KSyncee *syncee)
{
  mSyncee = syncee;
}

KSyncee *KSyncEntry::syncee()
{
  return mSyncee;
}


KSyncee::KSyncee() :
  mStatusLog(0)
{
}

KSyncee::~KSyncee()
{
  delete mStatusLog;
}

void KSyncee::setFilename(const QString &filename)
{
  mFilename = filename;
}

QString KSyncee::filename()
{
  return mFilename;
}

KSyncEntry *KSyncee::findEntry(const QString &id)
{
  kdDebug() << "KSyncee::findEntry() '" << id << "'" << endl;

  KSyncEntry *entry = firstEntry();
  while (entry) {
    if (entry->id() == id) return entry;
    entry = nextEntry();
  }

  return 0;
}

void KSyncee::replaceEntry(KSyncEntry *oldEntry,KSyncEntry *newEntry)
{
  removeEntry(oldEntry);
  addEntry(newEntry);
}

bool KSyncee::hasChanged(KSyncEntry *entry)
{
  if ( entry->timestamp().isEmpty() ) return true;

  mStatusLog->setGroup(entry->id());
  QString timestamp = mStatusLog->readEntry("Timestamp");

  return (timestamp != entry->timestamp());
}

bool KSyncee::load()
{
  delete mStatusLog;
  mStatusLog = new KSimpleConfig(locateLocal("appdata",statusLogName()));

  return read();
}

bool KSyncee::save()
{
  bool success = write();
  if (success) {
    writeLog();
    return true;
  } else {
    return false;
  }
}

void KSyncee::writeLog()
{  
  for (KSyncEntry *entry = firstEntry();entry;entry = nextEntry()) {
    mStatusLog->setGroup(entry->id());
    mStatusLog->writeEntry("Name",entry->name());
    mStatusLog->writeEntry("Timestamp",entry->timestamp());
  }
  
  mStatusLog->sync();
}

QString KSyncee::statusLogName()
{
  QString name = filename();

  name.replace(QRegExp("/"),"_");
  name.replace(QRegExp(":"),"_");

  name += ".syncee";
  
  return name;
}


KSyncer::KSyncer(KSyncUi *ui)
{
  mSyncees.setAutoDelete(true);
  if (!ui) {
    mUi = new KSyncUi();
  } else {
    mUi = ui;
  }
}

KSyncer::~KSyncer()
{
}

void KSyncer::addSyncee(KSyncee *syncee)
{
  mSyncees.append(syncee);
}

void KSyncer::sync()
{
  KSyncee *target = mSyncees.last();
  KSyncee *syncee = mSyncees.first();
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

void KSyncer::syncAllToTarget(KSyncee *target, bool writeback)
{
  KSyncee *syncee = mSyncees.first();
  while(syncee) {
    syncToTarget(syncee,target);
    syncee = mSyncees.next();
  }

  target->writeLog();

  if (writeback) {
    for (KSyncee *syncee=mSyncees.first();syncee;syncee = mSyncees.next()) {
      syncToTarget(target,syncee,true);
    }
  }
}

void KSyncer::syncToTarget(KSyncee *source, KSyncee *target, bool override)
{
  kdDebug() << "KSyncer::syncToTarget(): from: " << source->filename()
            << " to: " << target->filename() << "  override: "
            << (override ? "true" : "false") << endl;

  KSyncEntry *sourceEntry = source->firstEntry();
  while (sourceEntry) {
    KSyncEntry *targetEntry = target->findEntry(sourceEntry->id());
    if (targetEntry) {
      // Entry already exists in target
      if (sourceEntry->equals(targetEntry)) {
        // Entries are equal, no action required
      } else {
        // Entries are different, resolve conflict
        if (override) {
          // Force override
          target->replaceEntry(targetEntry,sourceEntry);
        } else {
          if (source->hasChanged(sourceEntry) &&
              target->hasChanged(targetEntry)) {
            // Both entries have changed
            KSyncEntry *result = mUi->deconflict(sourceEntry,targetEntry);
            if (result == sourceEntry) {
              target->replaceEntry(targetEntry,sourceEntry);
            }
          } else if (source->hasChanged(sourceEntry) &&
                     !target->hasChanged(targetEntry)) {
            // take source entry
            target->replaceEntry(targetEntry,sourceEntry);
          } else if (!source->hasChanged(sourceEntry) &&
                     target->hasChanged(targetEntry)) {
            // take target entry, no action required
          }
        }
      }
    } else {
      // New entry
      target->addEntry(sourceEntry);
    }

    sourceEntry = source->nextEntry();
  }

  source->writeLog();
}
