/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Mathias Froehlich <Mathias.Froehlich@web.de>

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

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>

#include <kapabilities.h>
#include <konnectorinfo.h>
#include <konnectorplugin.h>

#include "clientmanager.h"

#include "konnector.h"

using namespace KSync;
using namespace Threaded;

typedef KGenericFactory<ThreadedPlugin, QObject> ThreadedKonnectorPlugin;

K_EXPORT_COMPONENT_FACTORY(libthreadedkonnector, ThreadedKonnectorPlugin);

ThreadedPlugin::ThreadedPlugin( QObject* obj, const char* name, const QStringList )
  : Konnector( obj, name ) {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;

//   connect( &mClientManager, SIGNAL(signalThreadTerminated()),
// 	   SLOT(slotThreadTerminated()) );
  connect( &mClientManager, SIGNAL(signalProgress(const KSync::Progress&)),
	   SLOT(slotProgress(const KSync::Progress&)) );
  connect( &mClientManager, SIGNAL(signalError(const KSync::Error&)),
	   SLOT(slotError(const KSync::Error&)) );
  connect( &mClientManager, SIGNAL(signalFinished()),
	   SLOT(slotFinished()) );
}

ThreadedPlugin::~ThreadedPlugin() {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  mClientManager.terminate();
  mClientManager.wait();
}

Kapabilities ThreadedPlugin::capabilities() {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  return Kapabilities();
}

void ThreadedPlugin::setCapabilities( const KSync::Kapabilities& ) {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
}

bool ThreadedPlugin::readSyncees() {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  return mClientManager.readSyncees();
}

bool ThreadedPlugin::writeSyncees() {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  return mClientManager.writeSyncees();
}

bool ThreadedPlugin::startBackup( const QString& file ) { 
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  return mClientManager.startBackup( file );
}

bool ThreadedPlugin::startRestore( const QString& file ) {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  return mClientManager.startRestore( file );
}

bool ThreadedPlugin::connectDevice() {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  return mClientManager.connectDevice();
}

bool ThreadedPlugin::disconnectDevice() {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  return mClientManager.disconnectDevice();
}

KSync::KonnectorInfo ThreadedPlugin::info() const {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  return KSync::KonnectorInfo();
}

void ThreadedPlugin::download( const QString& ) {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
}

ConfigWidget* ThreadedPlugin::configWidget( const Kapabilities& cap, QWidget* parent, const char* name ) {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  return 0;
}

ConfigWidget* ThreadedPlugin::configWidget( QWidget* parent, const char* name ) {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  return 0;
}

void ThreadedPlugin::slotFinished() {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  progress( KSync::StdProgress::done() );
}

void ThreadedPlugin::slotError( const KSync::Error& e ) {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  error( e );
}

void ThreadedPlugin::slotProgress( const KSync::Progress& p ) {
  kdDebug() << __PRETTY_FUNCTION__ << " this = " << this << endl;
  progress( p );
}

#include "konnector.moc"
