/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "resourcekolab.h"
#include "event.h"
#include "task.h"
#include "journal.h"
#include <qobject.h>

#include <kabc/locknull.h>
//#include <libkdepim/kincidencechooser.h>

using namespace KCal;
using namespace Kolab;

static const char* kmailCalendarContentsType = "Calendar";
static const char* kmailTodoContentsType = "Task";
static const char* kmailJournalContentsType = "Journal";
static const char* eventAttachmentMimeType = "application/x-vnd.kolab.event";
static const char* todoAttachmentMimeType = "application/x-vnd.kolab.task";
static const char* journalAttachmentMimeType = "application/x-vnd.kolab.journal";

ResourceKolab::ResourceKolab( const KConfig *config )
  : ResourceCalendar( config ), ResourceKolabBase( "ResourceKolab-libkcal" ),
    mOpen( false )
{
  setType( "kolab" );
  connect( &mResourceChangedTimer, SIGNAL( timeout() ),
           this, SLOT( slotEmitResourceChanged() ) );
}

ResourceKolab::~ResourceKolab()
{
  // The resource is deleted on exit (StdAddressBook's KStaticDeleter),
  // and it wasn't closed before that, so close here to save the config.
  if ( mOpen ) {
    close();
  }
}

void ResourceKolab::loadSubResourceConfig( KConfig& config,
                                           const QString& name,
                                           const QString& label,
                                           bool writable,
                                           ResourceMap& subResource )
{
  KConfigGroup group( &config, name );
  bool active = group.readBoolEntry( "Active", true );
  subResource.insert( name, Kolab::SubResource( active, writable, label ) );
}

bool ResourceKolab::openResource( KConfig& config, const char* contentType,
                                  ResourceMap& map )
{
  // Read the subresource entries from KMail
  QValueList<KMailICalIface::SubResource> subResources;
  if ( !kmailSubresources( subResources, contentType ) )
    return false;
  map.clear();
  QValueList<KMailICalIface::SubResource>::ConstIterator it;
  for ( it = subResources.begin(); it != subResources.end(); ++it )
    loadSubResourceConfig( config, (*it).location, (*it).label, (*it).writable, map );
  return true;
}

bool ResourceKolab::doOpen()
{
  if ( mOpen )
    // Already open
    return true;
  mOpen = true;

  KConfig config( configFile() );

  return openResource( config, kmailCalendarContentsType, mEventSubResources )
    && openResource( config, kmailTodoContentsType, mTodoSubResources )
    && openResource( config, kmailJournalContentsType, mJournalSubResources );
}

static void closeResource( KConfig& config, ResourceMap& map )
{
  ResourceMap::ConstIterator it;
  for ( it = map.begin(); it != map.end(); ++it ) {
    config.setGroup( it.key() );
    config.writeEntry( "Active", it.data().active() );
  }
}

void ResourceKolab::doClose()
{
  if ( !mOpen )
    // Not open
    return;
  mOpen = false;

  KConfig config( configFile() );
  closeResource( config, mEventSubResources );
  closeResource( config, mTodoSubResources );
  closeResource( config, mJournalSubResources );
}

bool ResourceKolab::loadSubResource( const QString& subResource,
                                     const char* mimetype )
{
  QMap<Q_UINT32, QString> lst;
  if ( !kmailIncidences( lst, mimetype, subResource ) ) {
    kdError() << "Communication problem in ResourceKolab::load()\n";
    return false;
  }

  kdDebug() << "Kolab resource: got " << lst.count() << " in "
                << subResource << " of type " << mimetype << endl;

  const bool silent = mSilent;
  mSilent = true;
  for( QMap<Q_UINT32, QString>::ConstIterator it = lst.begin(); it != lst.end(); ++it )
    addIncidence( mimetype, it.data(), subResource, it.key() );
  mSilent = silent;

  return true;
}

bool ResourceKolab::doLoad()
{
  mUidMap.clear();

  return loadAllEvents() & loadAllTodos() & loadAllJournals();
}

bool ResourceKolab::doLoadAll( ResourceMap& map, const char* mimetype )
{
  bool rc = true;
  for ( ResourceMap::ConstIterator it = map.begin(); it != map.end(); ++it ) {
    if ( !it.data().active() )
      // This resource is disabled
      continue;

    rc &= loadSubResource( it.key(), mimetype );
  }
  return rc;
}

bool ResourceKolab::loadAllEvents()
{
  mCalendar.deleteAllEvents();
  return doLoadAll( mEventSubResources, eventAttachmentMimeType );
}

bool ResourceKolab::loadAllTodos()
{
  mCalendar.deleteAllTodos();
  return doLoadAll( mTodoSubResources, todoAttachmentMimeType );
}

bool ResourceKolab::loadAllJournals()
{
  mCalendar.deleteAllJournals();
  return doLoadAll( mJournalSubResources, journalAttachmentMimeType );
}

bool ResourceKolab::doSave()
{
  return true;
}

void ResourceKolab::incidenceUpdated( KCal::IncidenceBase* incidencebase )
{
  QString type = incidencebase->type();
  if ( type == "Event" ) type = "Calendar";
  else if ( type == "Todo" ) type = "Task";
  else if ( type != "Journal" ) return;

  incidencebase->setSyncStatus( KCal::Event::SYNCMOD );
  incidencebase->setLastModified( QDateTime::currentDateTime() );
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

  const QString uid = incidencebase->uid();

  QString subResource;
  Q_UINT32 sernum;
  if ( mUidMap.contains( uid ) ) {
    subResource = mUidMap[ uid ].resource();
    sernum = mUidMap[ uid ].serialNumber();
  } else {
    /* Either this is a bogus update or we are currently processing
     * this event ( removing and readding it ). If so, ignore this
     * update. Keep the last of these around and process once
     * we hear back from KMail on this event. */
    mPendingUpdates.replace( uid, incidencebase );
    return;
  }

  const char* mimetype = 0;
  QString xml;
  if ( type == "Calendar" ) {
    mimetype = eventAttachmentMimeType;
    xml = Kolab::Event::eventToXML( static_cast<KCal::Event *>(incidencebase),
                                    mCalendar.timeZoneId() );
  } else if ( type == "Task" ) {
    mimetype = todoAttachmentMimeType;
    xml = Kolab::Task::taskToXML( static_cast<KCal::Todo *>(incidencebase),
                                  mCalendar.timeZoneId() );
  } else if ( type == "Journal" ) {
    mimetype = journalAttachmentMimeType;
    xml = Kolab::Journal::journalToXML( static_cast<KCal::Journal *>(incidencebase ),
                                        mCalendar.timeZoneId() );
  } else {
    kdWarning(5006) << "Can't happen: unhandled type=" << type << endl;
  }

  kdDebug() << k_funcinfo << "XML string:\n" << xml << endl;

  /* Remove from the local calendar and the uidmap. The update will add the
   * event to a folder which will result in the signals telling us about it.
   * This way the other instances of this resource are notified as well, and
   * we know which uids are currently being processed. */
  if ( mUidMap.contains( uid ) ) {
    mUidMap.remove( uid );
    mCalendar.deleteIncidence( static_cast<KCal::Incidence*>( incidencebase ) );
  }

  if( !kmailUpdate( subResource, sernum, xml, mimetype, uid ) ) {
    kdError(5500) << "Communication problem in ResourceKolab::incidenceUpdated()\n";
    return;
  }
}
void ResourceKolab::resolveConflict( KCal::Incidence* inc, const QString& subresource, Q_UINT32 sernum )
{
#if 0
    if ( ! inc )
        return;
    if ( ! mResolveConflict ) {
        // we should do no conflict resolution
        delete inc;
        return;
    }
    Incidence* local = mCalendar.incidence( inc->uid() );
    Incidence* localIncidence = 0;
    Incidence* addedIncidence = 0;
    if ( local ) {
        KIncidenceChooser* ch = new KIncidenceChooser();
        ch->setIncidence( local ,inc );
        if ( KIncidenceChooser::chooseMode == KIncidenceChooser::ask ) {
            connect ( this, SIGNAL( useGlobalMode() ), ch, SLOT (  useGlobalMode() ) );
            if ( ch->exec() )
                if ( KIncidenceChooser::chooseMode != KIncidenceChooser::ask )
                    emit useGlobalMode() ;
        }
        Incidence* result = ch->getIncidence();
      delete ch;
      if ( result == local ) {
          localIncidence = local->clone();
          delete inc;
      } else  if ( result == inc ) {
          addedIncidence = inc;
      } else if ( result == 0 ) { // take both
          localIncidence = local->clone();
          addedIncidence = inc;
      }
      if ( localIncidence )
          localIncidence->recreate();
      if ( addedIncidence )
          addedIncidence->recreate();
      bool silent = mSilent;
      mSilent = false;
      if ( local->type() == "Event" ) {
          deleteEvent( (Event*)local );
          kmailDeleteIncidence( subresource,sernum);
          if ( localIncidence ) addEvent( (Event*)localIncidence, subresource, 0  );
          if ( addedIncidence  ) addEvent( (Event*)addedIncidence, subresource, 0  );
      } else if (local->type() == "Todo" ) {
          deleteTodo((Todo*)local);
          kmailDeleteIncidence( subresource,sernum);
          if ( localIncidence ) addTodo( (Todo*)localIncidence, subresource, 0  );
          if ( addedIncidence  ) addTodo( (Todo*)addedIncidence, subresource, 0  );
      } else if ( local->type() == "Journal" ) {
          deleteJournal((Journal*)local );
          kmailDeleteIncidence( subresource,sernum);
          if ( localIncidence ) addJournal( (Journal*)localIncidence, subresource, 0  );
          if ( addedIncidence  ) addJournal( (Journal*)addedIncidence, subresource, 0  );
      }
      mSilent = silent;


  }
#endif
}
void ResourceKolab::addIncidence( const char* mimetype, const QString& xml,
                                  const QString& subResource, Q_UINT32 sernum )
{
  // This uses pointer comparison, so it only works if we use the static
  // objects defined in the top of the file
  if ( mimetype == eventAttachmentMimeType )
    addEvent( xml, subResource, sernum );
  else if ( mimetype == todoAttachmentMimeType )
    addTodo( xml, subResource, sernum );
  else if ( mimetype == journalAttachmentMimeType )
    addJournal( xml, subResource, sernum );
}

bool ResourceKolab::addEvent( KCal::Event* event )
{
  return addEvent( event, QString::null, 0 );
}

void ResourceKolab::addEvent( const QString& xml, const QString& subresource,
                              Q_UINT32 sernum )
{
  KCal::Event* event = Kolab::Event::xmlToEvent( xml, mCalendar.timeZoneId() );
  Q_ASSERT( event );
  if( event ) {
    if ( !mUidMap.contains( event->uid() ) )
      addEvent( event, subresource, sernum );
    else
      resolveConflict( event, subresource, sernum );
  }
}

bool ResourceKolab::addEvent( KCal::Event* event, const QString& _subresource,
                              Q_UINT32 sernum )
{
  // Find out if this event was previously stored in KMail
  bool newEvent = _subresource.isEmpty();

  QString subResource =
    newEvent ? findWritableResource( mEventSubResources ) : _subresource;
  if ( subResource.isEmpty() )
    return false;

  /* check the noop case */
  if ( mUidMap.contains( event->uid() ) && mUidMap[ event->uid() ].resource() == subResource )
    return false;

  if ( !mSilent ) {
    /* We got this one from the user, tell KMail. */
    QString xml = Kolab::Event::eventToXML( event, mCalendar.timeZoneId() );
    kdDebug() << k_funcinfo << "XML string:\n" << xml << endl;

    if( !kmailUpdate( subResource, sernum, xml, eventAttachmentMimeType,
                      event->uid() ) ) {
      kdError(5500) << "Communication problem in ResourceKolab::addEvent()\n";
      return false;
    }
  } else {
    /* KMail got back to us, add to the cache unless there are pending updates. */
    mCalendar.addEvent( event );
    event->registerObserver( this );
    if ( !subResource.isEmpty() && sernum != 0 ) {
      mUidMap[ event->uid() ] = StorageReference( subResource, sernum );
      event->setReadOnly( !mEventSubResources[ subResource ].writable() );
    }
    if ( KCal::IncidenceBase *update = mPendingUpdates.find( event->uid() ) ) {
      mSilent = false; // we do want to tell KMail
      mPendingUpdates.remove( event->uid() );
      incidenceUpdated( update );
    }
  }
  return true;
}

void ResourceKolab::deleteEvent( KCal::Event* event )
{
  const QString uid = event->uid();
  if( !mUidMap.contains( uid ) ) return; // Odd
  /* The user told us to delete, tell KMail */
  if ( !mSilent ) {
    kmailDeleteIncidence( mUidMap[ uid ].resource(),
                          mUidMap[ uid ].serialNumber() );
  } else {
    /* KMail told us it went away. Cope. */
    mUidMap.remove( uid );
    mCalendar.deleteEvent(event);
  }
}

KCal::Event* ResourceKolab::event( const QString& uid )
{
  return mCalendar.event(uid);
}

KCal::Event::List ResourceKolab::rawEvents()
{
  return mCalendar.rawEvents();
}

KCal::Event::List ResourceKolab::rawEventsForDate( const QDate& date,
                                                   bool sorted )
{
  return mCalendar.rawEventsForDate( date, sorted );
}

KCal::Event::List ResourceKolab::rawEventsForDate( const QDateTime& qdt )
{
  return mCalendar.rawEventsForDate( qdt );
}

KCal::Event::List ResourceKolab::rawEvents( const QDate& start,
                                            const QDate& end,
                                            bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

bool ResourceKolab::addTodo( KCal::Todo* todo )
{
  return addTodo( todo, QString::null, 0 );
}

void ResourceKolab::addTodo( const QString& xml, const QString& subresource,
                             Q_UINT32 sernum )
{
  KCal::Todo* todo = Kolab::Task::xmlToTask( xml, mCalendar.timeZoneId() );
  Q_ASSERT( todo );
  if( todo )
    if ( !mUidMap.contains( todo->uid() ) )
      addTodo( todo, subresource, sernum );
    else
      resolveConflict( todo, subresource, sernum );
}

bool ResourceKolab::addTodo( KCal::Todo* todo, const QString& _subresource,
                             Q_UINT32 sernum )
{
  // Find out if this todo was previously stored in KMail
  bool newTodo = _subresource.isEmpty();

  QString subResource =
    newTodo ? findWritableResource( mTodoSubResources ) : _subresource;
  if ( subResource.isEmpty() )
    return false;

 /* check the noop case */
  if ( mUidMap.contains( todo->uid() ) && mUidMap[ todo->uid() ].resource() == subResource )
    return false;

  if ( !mSilent ) {
    QString xml = Kolab::Task::taskToXML( todo, mCalendar.timeZoneId() );
    kdDebug() << k_funcinfo << "XML string:\n" << xml << endl;

    if( !kmailUpdate( subResource, sernum, xml, todoAttachmentMimeType,
                      todo->uid() ) ) {
      kdError(5500) << "Communication problem in ResourceKolab::addTodo()\n";
      return false;
    }
  } else {
    /* KMail got back to us, add to the cache unless there are pending updates. */
    mCalendar.addTodo( todo );
    todo->registerObserver( this );
    if ( !subResource.isEmpty() && sernum != 0 ) {
      mUidMap[ todo->uid() ] = StorageReference( subResource, sernum );
      todo->setReadOnly( !mTodoSubResources[ subResource ].writable() );
    }
    if ( KCal::IncidenceBase *update = mPendingUpdates.find( todo->uid() ) ) {
      mSilent = false; // we do want to tell KMail
      mPendingUpdates.remove( todo->uid() );
      incidenceUpdated( update );
    }
  }
  return true;
}

void ResourceKolab::deleteTodo( KCal::Todo* todo )
{
  const QString uid = todo->uid();
  if( !mUidMap.contains( uid ) ) return; // Odd
  if ( !mSilent ) {
    kmailDeleteIncidence( mUidMap[ uid ].resource(),
                          mUidMap[ uid ].serialNumber() );
  } else {
    /* KMail told us it went away. Cope. */
    mUidMap.remove( uid );
    mCalendar.deleteTodo( todo );
  }
}

KCal::Todo* ResourceKolab::todo( const QString& uid )
{
  return mCalendar.todo( uid );
}

KCal::Todo::List ResourceKolab::rawTodos()
{
  return mCalendar.rawTodos();
}

KCal::Todo::List ResourceKolab::rawTodosForDate( const QDate& date )
{
  return mCalendar.rawTodosForDate( date );
}

bool ResourceKolab::addJournal( KCal::Journal* journal )
{
  return addJournal( journal, QString::null, 0 );
}

void ResourceKolab::addJournal( const QString& xml, const QString& subresource,
                                Q_UINT32 sernum )
{
  KCal::Journal* journal =
    Kolab::Journal::xmlToJournal( xml, mCalendar.timeZoneId() );
  Q_ASSERT( journal );
  if( journal ) {
    if ( !mUidMap.contains( journal->uid() ) )
      addJournal( journal, subresource, sernum );
    else
      resolveConflict( journal, subresource, sernum );
  }
}

bool ResourceKolab::addJournal( KCal::Journal* journal,
                                const QString& _subresource, Q_UINT32 sernum )
{
  // Find out if this journal was previously stored in KMail
  bool newJournal = _subresource.isEmpty();

  QString subResource =
    newJournal ? findWritableResource( mJournalSubResources ) : _subresource;
  if ( subResource.isEmpty() )
    return false;

  /* check the noop case */
  if ( mUidMap.contains( journal->uid() ) && mUidMap[ journal->uid() ].resource() == subResource )
    return false;

  if ( !mSilent ) {
    /* We got this one from the user, tell KMail. */
    QString xml = Kolab::Journal::journalToXML( journal,
                                                mCalendar.timeZoneId() );
    kdDebug() << k_funcinfo << "XML string:\n" << xml << endl;

    if( !kmailUpdate( subResource, sernum, xml, journalAttachmentMimeType,
                      journal->uid() ) ) {
      kdError(5500) << "Communication problem in ResourceKolab::addJournal()\n";
      return false;
    }
  } else {
    /* KMail got back to us, add to the cache unless there are pending updates. */
    mCalendar.addJournal( journal );
    journal->registerObserver( this );
    if ( !subResource.isEmpty() && sernum != 0 ) {
      mUidMap[ journal->uid() ] = StorageReference( subResource, sernum );
      journal->setReadOnly( !mJournalSubResources[ subResource ].writable() );
    }
    if ( KCal::IncidenceBase *update = mPendingUpdates.find( journal->uid() ) ) {
      mSilent = false; // we do want to tell KMail
      mPendingUpdates.remove( journal->uid() );
      incidenceUpdated( update );
    }
  }
  return true;
}

void ResourceKolab::deleteJournal( KCal::Journal* journal )
{
  const QString uid = journal->uid();
  if( !mUidMap.contains( uid ) ) return; // Odd
  /* The user told us to delete, tell KMail */
  if ( !mSilent ) {
    kmailDeleteIncidence( mUidMap[ uid ].resource(),
                          mUidMap[ uid ].serialNumber() );
  } else {
    /* KMail told us it went away. Cope. */
    mUidMap.remove( uid );
    mCalendar.deleteJournal( journal );
  }
}

KCal::Journal* ResourceKolab::journal( const QDate& date )
{
  return mCalendar.journal( date );
}

KCal::Journal* ResourceKolab::journal( const QString& uid )
{
  return mCalendar.journal(uid);
}

KCal::Journal::List ResourceKolab::journals()
{
  return mCalendar.journals();
}

KCal::Alarm::List ResourceKolab::alarms( const QDateTime& from,
                                         const QDateTime& to )
{
  return mCalendar.alarms( from, to );
}

KCal::Alarm::List ResourceKolab::alarmsTo( const QDateTime& to )
{
  return mCalendar.alarmsTo(to);
}

void ResourceKolab::setTimeZoneId( const QString& tzid )
{
  mCalendar.setTimeZoneId( tzid );
}

#include <qtimer.h>
bool ResourceKolab::fromKMailAddIncidence( const QString& type,
                                           const QString& subResource,
                                           Q_UINT32 sernum,
                                           const QString& xml )
{
  bool rc = true;
  const bool silent = mSilent;
  mSilent = true;

  // If this xml file is one of ours, load it here
  if ( type == kmailCalendarContentsType )
    addEvent( xml, subResource, sernum );
  else if ( type == kmailTodoContentsType )
    addTodo( xml, subResource, sernum );
  else if ( type == kmailJournalContentsType )
    addJournal( xml, subResource, sernum );
  else
    rc = false;

  mSilent = silent;
  mResourceChangedTimer.changeInterval( 100 );
  return rc;
}

void ResourceKolab::fromKMailDelIncidence( const QString& type,
                                           const QString& subResource,
                                           const QString& uid )
{
  if ( type != kmailCalendarContentsType && type != kmailTodoContentsType
       && type != kmailJournalContentsType )
    // Not ours
    return;

  if ( mUidMap.contains( uid ) && mUidMap[ uid ].resource() != subResource )
    return; // it was already moved elsewhere

  KCal::Incidence* incidence = mCalendar.incidence( uid );
  if( incidence )
    mCalendar.deleteIncidence( incidence );
  mUidMap.remove( uid );
  mResourceChangedTimer.changeInterval( 100 );
}

void ResourceKolab::fromKMailRefresh( const QString& type,
                                      const QString& /*subResource*/ )
{
  // TODO: Only load the specified subResource
  if ( type == "Calendar" )
    loadAllEvents();
  else if ( type == "Task" )
    loadAllTodos();
  else if ( type == "Journal" )
    loadAllJournals();
  mResourceChangedTimer.changeInterval( 100 );
}

void ResourceKolab::fromKMailAddSubresource( const QString& type,
                                             const QString& subResource,
                                             const QString& label,
                                             bool writable )
{
  ResourceMap* map = 0;
  const char* mimetype = 0;
  if ( type == kmailCalendarContentsType ) {
    map = &mEventSubResources;
    mimetype = eventAttachmentMimeType;
  } else if ( type == kmailTodoContentsType ) {
    map = &mTodoSubResources;
    mimetype = todoAttachmentMimeType;
  } else if ( type == kmailJournalContentsType ) {
    map = &mJournalSubResources;
    mimetype = journalAttachmentMimeType;
  } else
    // Not ours
    return;

  if ( map->contains( subResource ) )
    // Already registered
    return;

  KConfig config( configFile() );
  config.setGroup( subResource );

  bool active = config.readBoolEntry( subResource, true );
  (*map)[ subResource ] = Kolab::SubResource( active, writable, label );
  loadSubResource( subResource, mimetype );
  emit signalSubresourceAdded( this, type, subResource, label );
}

void ResourceKolab::fromKMailDelSubresource( const QString& type,
                                             const QString& subResource )
{
  ResourceMap* map = subResourceMap( type );
  if ( !map ) // not ours
    return;
  if ( map->contains( subResource ) )
    map->erase( subResource );
  else
    // Not registered
    return;

  // Delete from the config file
  KConfig config( configFile() );
  config.deleteGroup( subResource );
  config.sync();

  // Make a list of all uids to remove
  Kolab::UidMap::ConstIterator mapIt;
  QStringList uids;
  for ( mapIt = mUidMap.begin(); mapIt != mUidMap.end(); ++mapIt )
    if ( mapIt.data().resource() == subResource )
      // We have a match
      uids << mapIt.key();

  // Finally delete all the incidences
  if ( !uids.isEmpty() ) {
    const bool silent = mSilent;
    mSilent = true;
    QStringList::ConstIterator it;
    for ( it = uids.begin(); it != uids.end(); ++it ) {
      KCal::Incidence* incidence = mCalendar.incidence( *it );
      if( incidence )
        mCalendar.deleteIncidence( incidence );
      mUidMap.remove( *it );
    }
    mSilent = silent;
  }

  emit signalSubresourceRemoved( this, type, subResource );
}

QStringList ResourceKolab::subresources() const
{
  // Workaround: The ResourceView in KOrganizer wants to know this
  // before it opens the resource :-( Make sure we are open
  const_cast<ResourceKolab*>( this )->doOpen();
  return ( mEventSubResources.keys()
         + mTodoSubResources.keys()
         + mJournalSubResources.keys() );
}

const QString
ResourceKolab::labelForSubresource( const QString& subresource ) const
{
  if ( mEventSubResources.contains( subresource ) )
    return mEventSubResources[ subresource ].label();
  if ( mTodoSubResources.contains( subresource ) )
    return mTodoSubResources[ subresource ].label();
  if ( mJournalSubResources.contains( subresource ) )
    return mJournalSubResources[ subresource ].label();
  return subresource;
}

void ResourceKolab::fromKMailAsyncLoadResult( const QMap<Q_UINT32, QString>& map,
                                              const QString& type,
                                              const QString& folder )
{
  const bool silent = mSilent;
  mSilent = true;
  for( QMap<Q_UINT32, QString>::ConstIterator it = map.begin(); it != map.end(); ++it )
    addIncidence( type.latin1(), it.data(), folder, it.key() );
  mSilent = silent;
}

bool ResourceKolab::subresourceActive( const QString& subresource ) const
{
  // Workaround: The ResourceView in KOrganizer wants to know this
  // before it opens the resource :-( Make sure we are open
  const_cast<ResourceKolab*>( this )->doOpen();

  if ( mEventSubResources.contains( subresource ) )
    return mEventSubResources[ subresource ].active();
  if ( mTodoSubResources.contains( subresource ) )
    return mTodoSubResources[ subresource ].active();
  if ( mJournalSubResources.contains( subresource ) )
    return mJournalSubResources[ subresource ].active();

  // Safe default bet:
  kdDebug(5650) << "subresourceActive( " << subresource << " ): Safe bet\n";

  return true;
}

void ResourceKolab::setSubresourceActive( const QString &subresource, bool v )
{
  ResourceMap *map = 0;

  if ( mEventSubResources.contains( subresource ) )
     map = &mEventSubResources;
  if ( mTodoSubResources.contains( subresource ) )
     map = &mTodoSubResources;
  if ( mJournalSubResources.contains( subresource ) )
     map = &mJournalSubResources;

  if ( map && ( ( *map )[ subresource ].active() != v ) ) {
    ( *map )[ subresource ].setActive( v );
    doLoad();                     // refresh the mCalendar cache
    mResourceChangedTimer.changeInterval( 100 );
  }
}

void ResourceKolab::slotEmitResourceChanged()
{
   emit resourceChanged( this );
   mResourceChangedTimer.stop();
}

KABC::Lock* ResourceKolab::lock()
{
  return new KABC::LockNull( true );
}


Kolab::ResourceMap* ResourceKolab::subResourceMap( const QString& contentsType )
{
  if ( contentsType == kmailCalendarContentsType ) {
    return &mEventSubResources;
  } else if ( contentsType == kmailTodoContentsType ) {
    return &mTodoSubResources;
  } else if ( contentsType == kmailJournalContentsType ) {
    return &mJournalSubResources;
  }
  // Not ours
  return 0;
}

#include "resourcekolab.moc"
