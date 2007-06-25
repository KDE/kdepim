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

#include <QTimer>
#include <QHash>

#include <kcal/calendarlocal.h>
#include <kcal/icalformat.h>
#include <kcal/resourcecalendar.h>
#include "kcalkolab_export.h"
#include "../shared/resourcekolabbase.h"

namespace KCal {

struct TemporarySilencer;

class KCALKOLAB_EXPORT ResourceKolab : public KCal::ResourceCalendar,
                      public KCal::IncidenceBase::IncidenceObserver,
                      public Kolab::ResourceKolabBase
{
  Q_OBJECT
    friend struct TemporarySilencer;

public:
  ResourceKolab( const KConfigGroup& );
  ResourceKolab();

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
  bool addEvent( KCal::Event* anEvent );
  bool deleteEvent( KCal::Event* );
  KCal::Event* event( const QString &UniqueStr );
  KCal::Event::List rawEvents( EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
  KCal::Event::List rawEventsForDate(
    const QDate& date,
    EventSortField sortField=EventSortUnsorted,
    SortDirection sortDirection=SortDirectionAscending );
  KCal::Event::List rawEventsForDate( const KDateTime& dt );
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

  KCal::Alarm::List alarms( const KDateTime& from, const KDateTime& to );
  KCal::Alarm::List alarmsTo( const KDateTime& to );

  void setTimeZoneId( const QString& tzid );

  bool deleteIncidence( KCal::Incidence* i );

  /// The ResourceKolabBase methods called by KMail
  bool fromKMailAddIncidence( const QString& type, const QString& subResource,
                              quint32 sernum, int format, const QString& data );
  void fromKMailDelIncidence( const QString& type, const QString& subResource,
                              const QString& uid );
  void fromKMailRefresh( const QString& type, const QString& subResource );

  /// Listen to KMail changes in the amount of sub resources
  void fromKMailAddSubresource( const QString& type, const QString& subResource,
                                const QString& label, bool writable,
                                bool alarmRelevant );
  void fromKMailDelSubresource( const QString& type, const QString& subResource );

  void fromKMailAsyncLoadResult( const QMap<quint32, QString>& map,
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

  //TODO need to implement them
  virtual void deleteAllEvents() {}
  virtual void deleteAllTodos(){}
  virtual void deleteAllJournals(){}
  virtual void setTimeSpec(const KDateTime::Spec&){}
  virtual KDateTime::Spec timeSpec() const{ return KDateTime::UTC; }
  virtual QString timeZoneId() const { return QString(); }
  virtual void shiftTimes(const KDateTime::Spec&, const KDateTime::Spec&){}

  KABC::Lock* lock();

signals:
  void useGlobalMode();
protected slots:
   void slotEmitResourceChanged();
protected:
  /** 
   * Return list of alarms which are relevant for the current user. These
   * are the ones coming from folders which the user has "Administer" rights
   * for, as per ACL */
  KCal::Alarm::List relevantAlarms( const KCal::Alarm::List &alarms );

private:
  void removeIncidences( const QByteArray& incidenceType );
  void resolveConflict( KCal::Incidence*, const QString& subresource, quint32 sernum );
  void addIncidence( const char* mimetype, const QString& xml,
                     const QString& subResource, quint32 sernum );

  bool addIncidence( KCal::Incidence* i, const QString& subresource,
                     quint32 sernum );

  void addEvent( const QString& xml, const QString& subresource,
                 quint32 sernum );
  void addTodo( const QString& xml, const QString& subresource,
                quint32 sernum );
  void addJournal( const QString& xml, const QString& subresource,
                   quint32 sernum );


  bool loadAllEvents();
  bool loadAllTodos();
  bool loadAllJournals();

  bool doLoadAll( Kolab::ResourceMap& map, const char* mimetype );

  /// Reimplemented from IncidenceBase::Observer to know when an incidence was changed
  void incidenceUpdated( KCal::IncidenceBase* );

  bool openResource( KConfig& config, const char* contentType,
                     Kolab::ResourceMap& map );
  void loadSubResourceConfig( KConfig& config, const QString& name,
                              const QString& label, bool writable,
                              bool alarmRelevant, Kolab::ResourceMap& subResource );
  bool loadSubResource( const QString& subResource, const char* mimetype );

  QString configFile() const {
    return ResourceKolabBase::configFile( "kcal" );
  }

  Kolab::ResourceMap* subResourceMap( const QString& contentsType );

  bool sendKMailUpdate( KCal::IncidenceBase* incidence, const QString& _subresource,
                        quint32 sernum );


  KCal::CalendarLocal mCalendar;

  // The list of subresources
  Kolab::ResourceMap mEventSubResources, mTodoSubResources, mJournalSubResources;

  bool mOpen; // If the resource is open, this is true
  QHash<QString, KCal::IncidenceBase*> mPendingUpdates;
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
