/* This file is part of the KDE Project
   Copyright (c) 2002 Holger Freyther <freyther@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <qmutex.h>
#include <qmap.h>
#include "kdedsharedfile.h"

#include <kmessagebox.h>

class FileLockObject {
 public:
  FileLockObject(){
    readShares = 0;
    writeLock = false;
  };
  FileLockObject(const QString &fileName ){
    readShares = 0;
    writeLock = false;
    this->fileName = fileName;
  }
  ~FileLockObject(){};
  FileLockObject(const FileLockObject &obj  ){
    readShares = obj.readShares;
    writeLock = obj.writeLock;
    fileName = obj.fileName;
  }
  uint readShares;
  bool writeLock:1;
  QString fileName;
};

class KShareFileModule::KShareFileModulePrivate {
public:
  KShareFileModulePrivate(){
  };
  QMutex mutex;
  QMap<QString, FileLockObject> m_objects;
  QMap<QString, FileLockObject>::Iterator m_it;
};

KShareFileModule::KShareFileModule(const QCString &obj ) : KDEDModule(obj )
{
  d = new KShareFileModulePrivate();
}
KShareFileModule::~KShareFileModule()
{
  delete d;
}
/*
 * We want to manage a new file
 * Check if already have it our cache
 * and if not insert it
 */
void KShareFileModule::interestedIn(const QString &fileName )
{
  KMessageBox::error(0, "", "ksharedfile-kded" );
  d->mutex.lock();
  if( !d->m_objects.contains(fileName) ){
    d->m_objects.insert(fileName, FileLockObject(fileName) );
  }
  d->mutex.unlock();
}
void KShareFileModule::removeInterestIn(const QString &fileName )
{
  KMessageBox::error(0, "", "ksharedfile-kded" );
  d->mutex.lock();
  if( d->m_objects.contains(fileName) ){
    d->m_it = d->m_objects.find(fileName);
    if( !d->m_it.data().writeLock && d->m_it.data().readShares == 0 ){
      d->m_objects.remove(d->m_it );
    }
  }
  d->mutex.unlock();
}
/*
 Look if its inside the module

 */
bool KShareFileModule::readShareFile(const QString &fileName )
{
  KMessageBox::error(0, "", "ksharedfile-kded" );
  static bool val=false;
  d->mutex.lock();
  if(!d->m_objects.contains(fileName ) ){
    d->m_objects.insert(fileName, FileLockObject(fileName ) );
  }
  d->m_it = d->m_objects.find(fileName );
  if( !d->m_it.data().writeLock ){
    val = true;
    d->m_it.data().readShares += 1;
  }else{
    val = false;
  }
  d->mutex.unlock();
  return val;
}
bool KShareFileModule::readUnshareFile(const QString &fileName )
{
    bool val=false;
    d->mutex.lock();
    if(d->m_objects.contains(fileName ) ){
        d->m_it = d->m_objects.find(fileName );
        if(!d->m_it.data().writeLock ){
            if ( d->m_it.data().readShares > 0 )
                d->m_it.data().readShares--;
            val = true;
        }
    }
    d->mutex.unlock();

    return val;
}
bool KShareFileModule::writeLockFile(const QString &fileName )
{
  static bool val=false;
  d->mutex.lock();
  if(!d->m_objects.contains(fileName ) ){
    d->m_objects.insert(fileName, FileLockObject(fileName ) );
  }
  d->m_it = d->m_objects.find(fileName );
  if( !d->m_it.data().writeLock && d->m_it.data().readShares==0 ){
    val = true;
    d->m_it.data().writeLock = true;
  }else {
    val = false;
  }
  d->mutex.unlock();
  return val;
}
bool KShareFileModule::writeUnlockFile(const QString &fileName )
{
  bool val=false;
  d->mutex.lock();
  if(d->m_objects.contains(fileName ) ){
    d->m_it = d->m_objects.find(fileName );
    if(  d->m_it.data().writeLock ){
      //d->m_objects.remove(d->m_it );
      d->m_it.data().writeLock = false;
      val = true;
    }
  }
  d->mutex.unlock();
  return val;
}



extern "C" {
  KDE_EXPORT KDEDModule *create_ksharedfile(const QCString &obj)
  {
    return new KShareFileModule(obj);
  }
}
#include "kdedsharedfile.moc"
