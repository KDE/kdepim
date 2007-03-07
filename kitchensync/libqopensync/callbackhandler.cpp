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

#include <osengine/engine.h>

#include <libqopensync/engine.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>

#include "callbackhandler.h"

using namespace QSync;

class CallbackHandler::ConflictEvent : public QEvent
{
  public:
    enum { Type = QEvent::User + 1 };

    ConflictEvent( const SyncMapping& mapping )
      : QEvent( (QEvent::Type)(QEvent::User + 1) ), mMapping( mapping )
    {
    }

    SyncMapping mapping() const { return mMapping; }

  private:
    SyncMapping mMapping;
};

class CallbackHandler::ChangeEvent : public QEvent
{
  public:
    enum { Type = QEvent::User + 2 };

    ChangeEvent( const SyncChangeUpdate& change )
      : QEvent( (QEvent::Type)(QEvent::User + 2) ), mChange( change )
    {
    }

    SyncChangeUpdate change() const { return mChange; }

  private:
    SyncChangeUpdate mChange;
};

class CallbackHandler::MappingEvent : public QEvent
{
  public:
    enum { Type = QEvent::User + 3 };

    MappingEvent( const SyncMappingUpdate& mapping )
      : QEvent( (QEvent::Type)(QEvent::User + 3) ), mMapping( mapping )
    {
    }

    SyncMappingUpdate mapping() const { return mMapping; }

  private:
    SyncMappingUpdate mMapping;
};

class CallbackHandler::EngineEvent : public QEvent
{
  public:
    enum { Type = QEvent::User + 4 };

    EngineEvent( const SyncEngineUpdate& engine )
      : QEvent( (QEvent::Type)(QEvent::User + 4) ), mEngine( engine )
    {
    }

    SyncEngineUpdate engine() const { return mEngine; }

  private:
    SyncEngineUpdate mEngine;
};

class CallbackHandler::MemberEvent : public QEvent
{
  public:
    enum { Type = QEvent::User + 5 };

    MemberEvent( const SyncMemberUpdate& member )
      : QEvent( (QEvent::Type)(QEvent::User + 5) ), mMember( member )
    {
    }

    SyncMemberUpdate member() const { return mMember; }

  private:
    SyncMemberUpdate mMember;
};

CallbackHandler::CallbackHandler()
{
}

CallbackHandler::~CallbackHandler()
{
}

void CallbackHandler::setEngine( Engine *engine )
{
  mEngine = engine;

  osengine_set_conflict_callback( engine->mEngine, &conflict_callback, this );
  osengine_set_changestatus_callback( engine->mEngine, &change_callback, this );
  osengine_set_mappingstatus_callback( engine->mEngine, &mapping_callback, this );
  osengine_set_enginestatus_callback( engine->mEngine, &engine_callback, this );
  osengine_set_memberstatus_callback( engine->mEngine, &member_callback, this );
}

Engine* CallbackHandler::engine() const
{
  return mEngine;
}

void CallbackHandler::customEvent( QEvent *event )
{
  if ( event->type() == ConflictEvent::Type ) {
    ConflictEvent *conflictEvent = static_cast<ConflictEvent*>( event );
    emit conflict( conflictEvent->mapping() );
  } else if ( event->type() == ChangeEvent::Type ) {
    ChangeEvent *changeEvent = static_cast<ChangeEvent*>( event );
    emit change( changeEvent->change() );
  } else if ( event->type() == MappingEvent::Type ) {
    MappingEvent *mappingEvent = static_cast<MappingEvent*>( event );
    emit mapping( mappingEvent->mapping() );
  } else if ( event->type() == EngineEvent::Type ) {
    EngineEvent *engineEvent = static_cast<EngineEvent*>( event );
    emit engine( engineEvent->engine() );
  } else if ( event->type() == MemberEvent::Type ) {
    MemberEvent *memberEvent = static_cast<MemberEvent*>( event );
    emit member( memberEvent->member() );
  }
}

void CallbackHandler::conflict_callback( OSyncEngine *engine, OSyncMapping *omapping, void *data )
{
  SyncMapping mapping( omapping, engine );

  CallbackHandler *handler = static_cast<CallbackHandler*>( data );

  QCoreApplication::postEvent( handler, new ConflictEvent( mapping ) );
}

void CallbackHandler::change_callback( OSyncEngine*, OSyncChangeUpdate *update, void *data )
{
  SyncChangeUpdate change( update );

  CallbackHandler *handler = static_cast<CallbackHandler*>( data );

  QCoreApplication::postEvent( handler, new ChangeEvent( change ) );
}

void CallbackHandler::mapping_callback( OSyncMappingUpdate *update, void *data )
{
  CallbackHandler *handler = static_cast<CallbackHandler*>( data );

  SyncMappingUpdate mapping( update, handler->engine()->mEngine );

  QCoreApplication::postEvent( handler, new MappingEvent( mapping ) );
}

void CallbackHandler::engine_callback( OSyncEngine*, OSyncEngineUpdate *update, void *data )
{
  SyncEngineUpdate engine( update );

  CallbackHandler *handler = static_cast<CallbackHandler*>( data );

  QCoreApplication::postEvent( handler, new EngineEvent( engine ) );
}

void CallbackHandler::member_callback( OSyncMemberUpdate *update, void *data )
{
  SyncMemberUpdate member( update );

  CallbackHandler *handler = static_cast<CallbackHandler*>( data );

  QCoreApplication::postEvent( handler, new MemberEvent( member ) );
}

#include "callbackhandler.moc"
