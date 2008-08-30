/*
    This file is part of libqopensync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QSYNC_CALLBACKHANDLER_H
#define QSYNC_CALLBACKHANDLER_H

#include <libqopensync/qopensync_export.h>
#include <libqopensync/syncmapping.h>
#include <libqopensync/syncupdates.h>

#include <QtCore/QObject>

class OSyncEngine;
class OSyncMappingEngine;
class OSyncChangeUpdate;
class OSyncMappingUpdate;
class OSyncEngineUpdate;
class OSyncMemberUpdate;

namespace QSync {

class Engine;

class QSYNC_EXPORT CallbackHandler : public QObject
{
  Q_OBJECT

  public:
    CallbackHandler();
    ~CallbackHandler();

    void setEngine( Engine *engine );
    Engine *engine() const;

  Q_SIGNALS:
    void conflict( QSync::SyncMapping mapping );
    void change( const QSync::SyncChangeUpdate &update );
    void mapping( const QSync::SyncMappingUpdate &update );
    void engine( const QSync::SyncEngineUpdate &update );
    void member( const QSync::SyncMemberUpdate &update );

  protected:
    virtual void customEvent( QEvent *event );

  private:
    static void conflict_callback( OSyncEngine*, OSyncMappingEngine*, void* );
    static void change_callback( OSyncChangeUpdate*, void* );
    static void mapping_callback( OSyncMappingUpdate*, void* );
    static void engine_callback( OSyncEngineUpdate*, void* );
    static void member_callback( OSyncMemberUpdate*, void* );

    Engine* mEngine;
};

}

#endif
