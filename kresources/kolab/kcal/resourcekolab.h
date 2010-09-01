/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>
                  2004 Till Adam <till@klaralvdalens-datakonsult.se>

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

#ifndef KCAL_RESOURCEKOLAB_H
#define KCAL_RESOURCEKOLAB_H

#include <tqtimer.h>

#include <kdepimmacros.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/resourcecalendar.h>
#include "../shared/resourcekolabbase.h"

namespace KCal {

struct TemporarySilencer;

class KDE_EXPORT ResourceKolab : public KCal::ResourceCalendar,
                      public KCal::IncidenceBase::Observer,
                      public Kolab::ResourceKolabBase
{
  Q_OBJECT
    friend struct TemporarySilencer;

public:
  ResourceKolab( const KConfig* );
  virtual ~ResourceKolab();

  /// Load resource data.
  bool doLoad();

  /// Save resource data.
  bool doSave();

  /// Open the notes resource.
  bool doOpen();
  /// Close the notes resource.
  void doClose();

  // The libkcal functions. See the resource for descriptions
  KDE_DEPRECATED bool addEvent( KCal::Event *event );
  bool addEvent( KCal::Event *event, const TQString &subResource );
  bool deleteEvent( KCal::Event * );
  KCal::Event* event( const TQString &UniqueStr );
  KCal::Event::List rawEvents( EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
  KCal::Event::List rawEventsForDate(
    const TQDate& date,
    EventSortField sortField=EventSortUnsorted,
    SortDirection sortDirection=SortDirectionAscending );
  KCal::Event::List rawEventsForDate( const TQDateTime& qdt );
  KCal::Event::List rawEvents( const TQDate& start, const TQDate& end,
                               bool inclusive = false );

  KDE_DEPRECATED bool addTodo( KCal::Todo * todo );
  bool addTodo( KCal::Todo *todo, const TQString &subResource );
  bool deleteTodo( KCal::Todo * );
  KCal::Todo* todo( const TQString &uid );
  KCal::Todo::List rawTodos( TodoSortField sortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
  KCal::Todo::List rawTodosForDate( const TQDate& date );

  KDE_DEPRECATED bool addJournal( KCal::Journal * );
  bool addJournal( KCal::Journal *, const TQString &subResource );
  bool deleteJournal( KCal::Journal * );
  KCal::Journal* journal( const TQString &uid );
  KCal::Journal::List rawJournals( JournalSortField sortField = JournalSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
  KCal::Journal::List rawJournalsForDate( const TQDate &date );

  KCal::Alarm::List alarms( const TQDateTime& from, const TQDateTime& to );
  KCal::Alarm::List alarmsTo( const TQDateTime& to );

  void setTimeZoneId( const TQString& tzid );

  bool deleteIncidence( KCal::Incidence* i );

  /// The ResourceKolabBase methods called by KMail
  bool fromKMailAddIncidence( const TQString& type, const TQString& subResource,
                              Q_UINT32 sernum, int format, const TQString& data );
  void fromKMailDelIncidence( const TQString& type, const TQString& subResource,
                              const TQString& uid );
  void fromKMailRefresh( const TQString& type, const TQString& subResource );

  /// Listen to KMail changes in the amount of sub resources
  void fromKMailAddSubresource( const TQString& type, const TQString& subResource,
                                const TQString& label, bool writable,
                                bool alarmRelevant );
  void fromKMailDelSubresource( const TQString& type, const TQString& subResource );

  void fromKMailAsyncLoadResult( const TQMap<Q_UINT32, TQString>& map,
                                 const TQString& type,
                                 const TQString& folder );

  /** Return the list of subresources. */
  TQStringList subresources() const;

  bool canHaveSubresources() const { return true; }

  /** Is this subresource active? */
  bool subresourceActive( const TQString& ) const;
  /** (De)activate the subresource */
  virtual void setSubresourceActive( const TQString &, bool );

  /** Is this subresource writable? */
  bool subresourceWritable( const TQString& ) const;

  /** What is the label for this subresource? */
  virtual const TQString labelForSubresource( const TQString& resource ) const;

  virtual TQString subresourceIdentifier( Incidence *incidence );

  virtual bool addSubresource( const TQString& resource, const TQString& parent );
  virtual bool removeSubresource( const TQString& resource );

  virtual TQString subresourceType( const TQString &resource );

  KABC::Lock* lock();

  void beginAddingIncidences();

  void endAddingIncidences();

signals:
  void useGlobalMode();
protected slots:
  void slotEmitResourceChanged();
  void writeConfig();

protected:
  /**
   * Return list of alarms which are relevant for the current user. These
   * are the ones coming from folders which the user has "Administer" rights
   * for, as per ACL */
  KCal::Alarm::List relevantAlarms( const KCal::Alarm::List &alarms );

private:
  void removeIncidences( const TQCString& incidenceType );
  void resolveConflict( KCal::Incidence*, const TQString& subresource, Q_UINT32 sernum );
  void addIncidence( const char* mimetype, const TQString& xml,
                     const TQString& subResource, Q_UINT32 sernum );


  /**
     Caller guarantees i is not null.
   */
  bool addIncidence( KCal::Incidence *i, const TQString& subresource,
                     Q_UINT32 sernum );

  void addEvent( const TQString& xml, const TQString& subresource,
                 Q_UINT32 sernum );
  void addTodo( const TQString& xml, const TQString& subresource,
                Q_UINT32 sernum );
  void addJournal( const TQString& xml, const TQString& subresource,
                   Q_UINT32 sernum );


  bool loadAllEvents();
  bool loadAllTodos();
  bool loadAllJournals();

  bool doLoadAll( Kolab::ResourceMap& map, const char* mimetype );

  /// Reimplemented from IncidenceBase::Observer to know when an incidence was changed
  void incidenceUpdated( KCal::IncidenceBase* );
  void incidenceUpdatedSilent( KCal::IncidenceBase* incidencebase);

  bool openResource( KConfig& config, const char* contentType,
                     Kolab::ResourceMap& map );
  void loadSubResourceConfig( KConfig& config, const TQString& name,
                              const TQString& label, bool writable,
                              bool alarmRelevant, Kolab::ResourceMap& subResource );
  bool loadSubResource( const TQString& subResource, const char* mimetype );
  bool unloadSubResource( const TQString& subResource );

  TQString configFile() const {
    return ResourceKolabBase::configFile( "kcal" );
  }

  Kolab::ResourceMap* subResourceMap( const TQString& contentsType );

  bool sendKMailUpdate( KCal::IncidenceBase* incidence, const TQString& _subresource,
                        Q_UINT32 sernum );


  KCal::CalendarLocal mCalendar;

  // The list of subresources
  Kolab::ResourceMap mEventSubResources, mTodoSubResources, mJournalSubResources;

  bool mOpen; // If the resource is open, this is true
  TQDict<KCal::IncidenceBase> mPendingUpdates;
  TQTimer mResourceChangedTimer;
  ICalFormat mFormat;

  /**
    This map contains the association between a new added incidence
    and the subresource it belongs to.
    That's needed to return the correct mapping in subresourceIdentifier().

    We can't trust on mUidMap here, because it contains only non-pending uids.
   */
  TQMap<TQString, TQString> mNewIncidencesMap;
  int mProgressDialogIncidenceLimit;

  /**
   * If a user has a subresource for viewing another user's folder then it can happen
   * that addIncidence(...) adds an incidence with an already existing UID.
   *
   * When this happens, addIncidence(...) sets a new random UID and stores the
   * original UID using incidence->setSchedulingID(uid) because KCal doesn't
   * allow two incidences to have the same UID.
   *
   * This map keeps track of the generated UIDs (which are local) so we can delete the
   * right incidence inside fromKMailDelIncidence(...) whenever we sync.
   *
   * The key is originalUID,subResource and the value is the fake UID.
   */
  TQMap< QPair<TQString, TQString>, TQString > mOriginalUID2fakeUID;

  bool mBatchAddingInProgress;
  TQMap<Kolab::ResourceType,TQString> mLastUsedResources;

  /**
     Indexed by uid, it holds the last known revision of an incidence.
     If we receive an update where the incidence still has the same
     revision as the last known, we ignore it and don't send it to kmail,
     because shortly after, IncidenceChanger will increment the revision
     and that will trigger another update.

     If we didn't discard the first update, kmail would have been updated twice.
  */
  TQMap<TQString,int> mLastKnownRevisions;

};

struct TemporarySilencer {
 TemporarySilencer( ResourceKolab *_resource )
  {
    resource = _resource;
    oldValue = resource->mSilent;
    resource->mSilent = true;
  }
  ~TemporarySilencer()
  {
    resource->mSilent = oldValue;
  }
  ResourceKolab *resource;
  bool oldValue;
};

}

#endif // KCAL_RESOURCEKOLAB_H
