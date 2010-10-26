/*
    Copyright (c) 2010 Sérgio Martins <iamsergio@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include <calendarsupport/utils.h>
#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/next/incidencechanger2.h>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/Session>
#include <Akonadi/ItemFetchScope>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>
#include <akonadi/collection.h>
#include <akonadi/collectionstatistics.h>
#include <akonadi/control.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/qtest_akonadi.h>

#include <Akonadi/Item>
#include <Akonadi/Collection>

#include <KCalCore/Event>
#include <KCalCore/Journal>
#include <KCalCore/Todo>

#include <KSystemTimeZone>

#include <QtCore/QObject>
#include <QPushButton>

using namespace Akonadi;
using namespace KCalCore;
using namespace CalendarSupport;

class IncidenceChangerTest : public QObject
{
  Q_OBJECT
  Collection mCollection;
  CalendarModel *mCalendarModel;
  CalendarSupport::Calendar *mCalendar;

  // List of incidence uids.
  QStringList mPendingInsertsInETM;
  QStringList mPendingUpdatesInETM;
  QStringList mPendingDeletesInETM;

  bool mWaitingForSignals;
  IncidenceChanger2::ResultCode mExpectedResult;

  private slots:
    void initTestCase()
    {
      mWaitingForSignals = false;
      mExpectedResult = IncidenceChanger2::ResultCodeSuccess;
      //Control::start(); //TODO: uncomment when using testrunner
      qRegisterMetaType<CalendarSupport::IncidenceChanger2::ResultCode>("CalendarSupport::IncidenceChanger2::ResultCode");
      CollectionFetchJob *job = new CollectionFetchJob( Collection::root(),
                                                        CollectionFetchJob::Recursive,
                                                        this );
      // Get list of collections
      job->fetchScope().setContentMimeTypes( QStringList() << "application/x-vnd.akonadi.calendar.event" );
      AKVERIFYEXEC( job );

      // Find our collection
      Collection::List collections = job->collections();
      foreach( Collection collection, collections ) {
        if ( collection.name() == QLatin1String( "akonadi_ical_resource_0" ) ) {
          mCollection = collection;
          break;
        }
      }

      QVERIFY( mCollection.isValid() );

      // Setup our ETM
      Akonadi::Session *session = new Akonadi::Session( "KOrganizerETM", this );
      Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( this );
      Akonadi::ItemFetchScope scope;
      scope.fetchFullPayload( true );
      scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();
      monitor->setSession( session );
      monitor->setCollectionMonitored( Akonadi::Collection::root() );
      monitor->fetchCollection( true );
      monitor->setItemFetchScope( scope );
      monitor->setMimeTypeMonitored( "text/calendar", true );
      monitor->setMimeTypeMonitored( KCalCore::Event::eventMimeType(), true );
      monitor->setMimeTypeMonitored( KCalCore::Todo::todoMimeType(), true );
      monitor->setMimeTypeMonitored( KCalCore::Journal::journalMimeType(), true );
      mCalendarModel = new CalendarSupport::CalendarModel( monitor, this );

      mCalendar = new CalendarSupport::Calendar( mCalendarModel,
                                                 mCalendarModel,
                                                 KSystemTimeZones::local() );

      IncidenceChanger2 *changer = new IncidenceChanger2( mCalendar );
      changer->setShowDialogsOnError( false );

      connect( mCalendarModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
               SLOT(rowsInserted(QModelIndex,int,int)) );

      connect( mCalendarModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               SLOT(dataChanged(QModelIndex,QModelIndex)) );

      connect( mCalendarModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
               SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );

    }

    void testCreating()
    {
    }

    void testModifying()
    {
    }

    void testDeleting()
    {
    }

  public Q_SLOTS:

    void waitForETMorSignals()
    {
      while ( !mPendingInsertsInETM.isEmpty() ||
              !mPendingUpdatesInETM.isEmpty() ||
              !mPendingDeletesInETM.isEmpty() ||
              mWaitingForSignals ) {
        QTest::qWait( 1000 );
      }
    }

    void rowsInserted( const QModelIndex &index, int start, int end )
    {
      Item::List list = itemsFromModel( mCalendarModel, index, start, end );

      foreach( const Item &item, list ) {
        Incidence::Ptr incidence = CalendarSupport::incidence( item );
        if ( incidence && mPendingInsertsInETM.contains( incidence->uid() ) )
          mPendingInsertsInETM.removeAll( incidence->uid() );
      }
    }

    void dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
    {
      Q_ASSERT( topLeft.row() <= bottomRight.row() );
      const int endRow = bottomRight.row();
      QModelIndex i( topLeft );
      int row = i.row();
      while ( row <= endRow ) {
        const Akonadi::Item item = itemFromIndex( i );
        if ( item.isValid() ) {
          Incidence::Ptr incidence = CalendarSupport::incidence( item );
          if ( incidence && mPendingUpdatesInETM.contains( incidence->uid() ) )
            mPendingUpdatesInETM.removeAll( incidence->uid() );
        }
        ++row;
        i = i.sibling( row, topLeft.column() );
      }
    }
};

// For undo/redo buttons
QTEST_AKONADIMAIN( IncidenceChangerTest, GUI )

#include "incidencechangertest.moc"
