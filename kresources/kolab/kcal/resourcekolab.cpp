/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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

#include <kabc/locknull.h>

using namespace Kolab;


class KolabFactory : public KRES::PluginFactoryBase
{
public:
  KRES::Resource *resource( const KConfig *config )
  {
    return new ResourceKolab( config );
  }

  KRES::ConfigWidget *configWidget( QWidget* )
  {
    return 0;
  }
};

extern "C"
{
  void *init_kcal_kolab()
  {
    return ( new KolabFactory() );
  }
}

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
                                           const QString& name, bool writable,
                                           ResourceMap& subResource )
{
  KConfigGroup group( &config, name );
  bool active = group.readBoolEntry( "Active", true );
  subResource.insert( name, Kolab::SubResource( active, writable, name ) );
}

bool ResourceKolab::openResource( KConfig& config, const char* contentType,
                                  ResourceMap& map )
{
  // Read the subresource entries from KMail
  QMap<QString, bool> subResources;
  if ( !kmailSubresources( subResources, contentType ) )
    return false;
  map.clear();
  QMap<QString, bool>::ConstIterator it;
  for ( it = subResources.begin(); it != subResources.end(); ++it )
    loadSubResourceConfig( config, it.key(), it.data(), map );
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

  for( QMap<Q_UINT32, QString>::ConstIterator it = lst.begin(); it != lst.end(); ++it )
    addIncidence( mimetype, it.data(), subResource, it.key() );

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

void ResourceKolab::incidenceUpdated( KCal::IncidenceBase* )
{
  kdDebug() << "NYI: " << k_funcinfo << endl;
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
  KCal::Event* event = Event::xmlToEvent( xml );
  Q_ASSERT( event );
  if( event && !mUidMap.contains( event->uid() ) )
    addEvent( event, subresource, sernum );
}

bool ResourceKolab::addEvent( KCal::Event* event, const QString& subresource,
                              Q_UINT32 sernum )
{
  event->registerObserver( this );

  // Find out if this event was previously stored in KMail
  bool newEvent = subresource.isEmpty();
  mCalendar.addEvent( event );

  QString resource =
    newEvent ? findWritableResource( mEventSubResources ) : subresource;
  Q_ASSERT( !resource.isEmpty() );

  if ( !mSilent ) {
    QString xml = Event::eventToXML( event );
    kdDebug() << k_funcinfo << "XML string:\n" << xml << endl;

    if( !kmailUpdate( resource, sernum, xml, eventAttachmentMimeType,
                      event->uid() ) ) {
      kdError(5500) << "Communication problem in ResourceKolab::addEvent()\n";
      return false;
    }
  }

  if ( !resource.isEmpty() && sernum != 0 ) {
    mUidMap[ event->uid() ] = StorageReference( resource, sernum );
    return true;
  }

  return false;
}

void ResourceKolab::deleteEvent( KCal::Event* event )
{
  const QString uid = event->uid();
  if( !mUidMap.contains( uid ) ) return; // Odd
  if ( !mSilent )
    kmailDeleteIncidence( mUidMap[ uid ].resource(),
                          mUidMap[ uid ].serialNumber() );
  mUidMap.remove( uid );
  mCalendar.deleteEvent(event);
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
  KCal::Todo* todo = Task::xmlToTask( xml );
  Q_ASSERT( todo );
  if( todo && !mUidMap.contains( todo->uid() ) )
    addTodo( todo, subresource, sernum );
}

bool ResourceKolab::addTodo( KCal::Todo* todo, const QString& subresource,
                             Q_UINT32 sernum )
{
  todo->registerObserver( this );

  // Find out if this todo was previously stored in KMail
  bool newTodo = subresource.isEmpty();
  mCalendar.addTodo( todo );

  QString resource =
    newTodo ? findWritableResource( mTodoSubResources ) : subresource;
  Q_ASSERT( !resource.isEmpty() );

  if ( !mSilent ) {
    QString xml = Task::taskToXML( todo );
    kdDebug() << k_funcinfo << "XML string:\n" << xml << endl;

    if( !kmailUpdate( resource, sernum, xml, todoAttachmentMimeType,
                      todo->uid() ) ) {
      kdError(5500) << "Communication problem in ResourceKolab::addTodo()\n";
      return false;
    }
  }

  if ( !resource.isEmpty() && sernum != 0 ) {
    mUidMap[ todo->uid() ] = StorageReference( resource, sernum );
    return true;
  }

  return false;
}

void ResourceKolab::deleteTodo( KCal::Todo* todo )
{
  const QString uid = todo->uid();
  if( !mUidMap.contains( uid ) ) return; // Odd
  if ( !mSilent )
    kmailDeleteIncidence( mUidMap[ uid ].resource(),
                          mUidMap[ uid ].serialNumber() );
  mUidMap.remove( uid );
  mCalendar.deleteTodo( todo );
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
  KCal::Journal* journal = Journal::xmlToJournal( xml );
  Q_ASSERT( journal );
  if( journal && !mUidMap.contains( journal->uid() ) )
    addJournal( journal, subresource, sernum );
}

bool ResourceKolab::addJournal( KCal::Journal* journal,
                                const QString& subresource, Q_UINT32 sernum )
{
  journal->registerObserver( this );

  // Find out if this journal was previously stored in KMail
  bool newJournal = subresource.isEmpty();
  mCalendar.addJournal( journal );

  QString resource =
    newJournal ? findWritableResource( mJournalSubResources ) : subresource;
  Q_ASSERT( !resource.isEmpty() );

  if ( !mSilent ) {
    QString xml = Journal::journalToXML( journal );
    kdDebug() << k_funcinfo << "XML string:\n" << xml << endl;

    if( !kmailUpdate( resource, sernum, xml, journalAttachmentMimeType,
                      journal->uid() ) ) {
      kdError(5500) << "Communication problem in ResourceKolab::addJournal()\n";
      return false;
    }
  }

  if ( !resource.isEmpty() && sernum != 0 ) {
    mUidMap[ journal->uid() ] = StorageReference( resource, sernum );
    return true;
  }

  return false;
}

void ResourceKolab::deleteJournal( KCal::Journal* journal )
{
  const QString uid = journal->uid();
  if( !mUidMap.contains( uid ) ) return; // Odd
  if ( !mSilent )
    kmailDeleteIncidence( mUidMap[ uid ].resource(),
                          mUidMap[ uid ].serialNumber() );
  mUidMap.remove( uid );
  mCalendar.deleteJournal( journal );
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
  return rc;
}

void ResourceKolab::fromKMailDelIncidence( const QString& type,
                                           const QString& /*subResource*/,
                                           const QString& uid )
{
  if ( type != kmailCalendarContentsType && type != kmailTodoContentsType
       && type != kmailJournalContentsType )
    // Not ours
    return;

  KCal::Incidence* incidence = mCalendar.incidence( uid );
  if( incidence )
    mCalendar.deleteIncidence( incidence );
  mUidMap.remove( uid );
}

void ResourceKolab::slotRefresh( const QString& type,
                                 const QString& /*subResource*/ )
{
  // TODO: Only load the specified resource
  if ( type == "Calendar" )
    loadAllEvents();
  else if ( type == "Task" )
    loadAllTodos();
  else if ( type == "Journal" )
    loadAllJournals();
}

void ResourceKolab::fromKMailAddSubresource( const QString& type,
                                             const QString& subResource,
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
  (*map)[ subResource ] = Kolab::SubResource( active, writable, subResource );
  loadSubResource( subResource, mimetype );
  emit signalSubresourceAdded( this, type, subResource );
}

void ResourceKolab::fromKMailDelSubresource( const QString& type,
                                             const QString& subResource )
{
  if ( type == kmailCalendarContentsType ) {
    if ( mEventSubResources.contains( subResource ) )
      mEventSubResources.erase( subResource );
    else
      // Not registered
      return;
  } else if ( type == kmailTodoContentsType ) {
    if ( mTodoSubResources.contains( subResource ) )
      mTodoSubResources.erase( subResource );
    else
      // Not registered
      return;
  } else if ( type == kmailJournalContentsType ) {
    if ( mJournalSubResources.contains( subResource ) )
      mJournalSubResources.erase( subResource );
    else
      // Not registered
      return;
  } else
    // Not ours
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
  return mEventSubResources.keys() + mTodoSubResources.keys()
    + mJournalSubResources.keys();
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

KABC::Lock* ResourceKolab::lock()
{
  return new KABC::LockNull( true );
}


#include "resourcekolab.moc"
