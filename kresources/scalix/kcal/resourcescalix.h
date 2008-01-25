/*
    This file is part of the scalix resource - based on the kolab resource.

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

#ifndef KCAL_RESOURCESCALIX_H
#define KCAL_RESOURCESCALIX_H

#include <qtimer.h>

#include <kdepimmacros.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/resourcecalendar.h>
#include "../shared/resourcescalixbase.h"

namespace KCal {

struct TemporarySilencer;

class KDE_EXPORT ResourceScalix : public KCal::ResourceCalendar,
                      public KCal::IncidenceBase::Observer,
                      public Scalix::ResourceScalixBase
{
  Q_OBJECT
    friend struct TemporarySilencer;

public:
  ResourceScalix( const KConfig* );
  virtual ~ResourceScalix();

  /// Load resource data.
  bool doLoad();

  /// Save resource data.
  bool doSave();

  /// Open the notes resource.
  bool doOpen();
  /// Close the notes resource.
  void doClose();

  // The libkcal functions. See the resource for descriptions
  bool addEvent( KCal::Event* anEvent );
  bool deleteEvent( KCal::Event* );
  KCal::Event* event( const QString &UniqueStr );
  KCal::Event::List rawEvents( EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
  KCal::Event::List rawEventsForDate(
    const QDate& date,
    EventSortField sortField=EventSortUnsorted,
    SortDirection sortDirection=SortDirectionAscending );
  KCal::Event::List rawEventsForDate( const QDateTime& qdt );
  KCal::Event::List rawEvents( const QDate& start, const QDate& end,
                               bool inclusive = false );

  bool addTodo( KCal::Todo* todo );
  bool deleteTodo( KCal::Todo* );
  KCal::Todo* todo( const QString& uid );
  KCal::Todo::List rawTodos( TodoSortField sortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
  KCal::Todo::List rawTodosForDate( const QDate& date );

  bool addJournal( KCal::Journal* );
  bool deleteJournal( KCal::Journal* );
  KCal::Journal* journal( const QString& uid );
  KCal::Journal::List rawJournals( JournalSortField sortField = JournalSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
  KCal::Journal::List rawJournalsForDate( const QDate &date );

  KCal::Alarm::List alarms( const QDateTime& from, const QDateTime& to );
  KCal::Alarm::List alarmsTo( const QDateTime& to );

  void setTimeZoneId( const QString& tzid );

  bool deleteIncidence( KCal::Incidence* i );

  /// The ResourceScalixBase methods called by KMail
  bool fromKMailAddIncidence( const QString& type, const QString& subResource,
                              Q_UINT32 sernum, int format, const QString& data );
  void fromKMailDelIncidence( const QString& type, const QString& subResource,
                              const QString& uid );
  void fromKMailRefresh( const QString& type, const QString& subResource );

  /// Listen to KMail changes in the amount of sub resources
  void fromKMailAddSubresource( const QString& type, const QString& subResource,
                                const QString& label, bool writable );
  void fromKMailDelSubresource( const QString& type, const QString& subResource );

  void fromKMailAsyncLoadResult( const QMap<Q_UINT32, QString>& map,
                                 const QString& type,
                                 const QString& folder );

  /** Return the list of subresources. */
  QStringList subresources() const;

  /** Is this subresource active? */
  bool subresourceActive( const QString& ) const;
  /** (De)activate the subresource */
  virtual void setSubresourceActive( const QString &, bool );

  /** What is the label for this subresource? */
  virtual const QString labelForSubresource( const QString& resource ) const;

  virtual QString subresourceIdentifier( Incidence *incidence );

  KABC::Lock* lock();

signals:
  void useGlobalMode();
protected slots:
   void slotEmitResourceChanged();

private:
  void removeIncidences( const QCString& incidenceType );
  void resolveConflict( KCal::Incidence*, const QString& subresource, Q_UINT32 sernum );

  void addIncidence( const char* mimetype, const QString& xml,
                     const QString& subResource, Q_UINT32 sernum );

  bool addIncidence( KCal::Incidence* i, const QString& subresource,
                     Q_UINT32 sernum );
/*
  void addEvent( const QString& xml, const QString& subresource,
                 Q_UINT32 sernum );
  void addTodo( const QString& xml, const QString& subresource,
                Q_UINT32 sernum );
  void addJournal( const QString& xml, const QString& subresource,
                   Q_UINT32 sernum );
*/

  bool loadAllEvents();
  bool loadAllTodos();
  bool loadAllJournals();

  bool doLoadAll( Scalix::ResourceMap& map, const char* mimetype );

  /// Reimplemented from IncidenceBase::Observer to know when an incidence was changed
  void incidenceUpdated( KCal::IncidenceBase* );

  bool openResource( KConfig& config, const char* contentType,
                     Scalix::ResourceMap& map );
  void loadSubResourceConfig( KConfig& config, const QString& name,
                              const QString& label, bool writable,
                              Scalix::ResourceMap& subResource );
  bool loadSubResource( const QString& subResource, const char* mimetype );

  QString configFile() const {
    return ResourceScalixBase::configFile( "kcal" );
  }

  Scalix::ResourceMap* subResourceMap( const QString& contentsType );

  bool sendKMailUpdate( KCal::IncidenceBase* incidence, const QString& _subresource,
                        Q_UINT32 sernum );


  KCal::CalendarLocal mCalendar;

  // The list of subresources
  Scalix::ResourceMap mEventSubResources, mTodoSubResources, mJournalSubResources;

  bool mOpen; // If the resource is open, this is true
  QDict<KCal::IncidenceBase> mPendingUpdates;
  QTimer mResourceChangedTimer;
  ICalFormat mFormat;

  /**
    This map contains the association between a new added incidence
    and the subresource it belongs to.
    That's needed to return the correct mapping in subresourceIdentifier().

    We can't trust on mUidMap here, because it contains only non-pending uids.
   */
  QMap<QString, QString> mNewIncidencesMap;
  int mProgressDialogIncidenceLimit;
};

struct TemporarySilencer {
 TemporarySilencer( ResourceScalix *_resource )
  {
    resource = _resource;
    oldValue = resource->mSilent;
    resource->mSilent = true;
  }
  ~TemporarySilencer()
  {
    resource->mSilent = oldValue;
  }
  ResourceScalix *resource;
  bool oldValue;
};

}

#endif // KCAL_RESOURCESCALIX_H
