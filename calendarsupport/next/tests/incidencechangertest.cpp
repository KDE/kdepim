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

  bool mWaitingForIncidenceChangerSignals;
  IncidenceChanger2::ResultCode mExpectedResult;
  IncidenceChanger2 *mChanger;

  QSet<int> mKnownChangeIds;

  private slots:
    void initTestCase()
    {
      mWaitingForIncidenceChangerSignals = false;
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

      mChanger = new IncidenceChanger2( mCalendar );
      mChanger->setShowDialogsOnError( false );

      connect( mChanger, SIGNAL(createFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)),
               SLOT(createFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)) );

      connect( mChanger, SIGNAL(deleteFinished(int,QVector<Akonadi::Item::Id>,CalendarSupport::IncidenceChanger2::ResultCode,QString)),
               SLOT(deleteFinished(int,QVector<Akonadi::Item::Id>,CalendarSupport::IncidenceChanger2::ResultCode,QString)) );

      connect( mChanger,SIGNAL(modifyFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)),
               SLOT(modifyFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)) );

      connect( mCalendarModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
               SLOT(rowsInserted(QModelIndex,int,int)) );

      connect( mCalendarModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               SLOT(dataChanged(QModelIndex,QModelIndex)) );

      connect( mCalendarModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
               SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );
    }

    void testCreating()
    {
      int changeId;

      { // Create 5 incidences, wait for the signal.
        for ( int i = 0; i < 5; ++i ) {
          const QString uid( "uid" + QString::number( i ) );
          const QString summary( "summary" + QString::number( i ) );
          Incidence::Ptr incidence( new Event() );
          incidence->setUid( uid );
          incidence->setSummary( summary );
          mPendingInsertsInETM.append( uid );
          changeId = mChanger->createIncidence( incidence,
                                                mCollection );
          QVERIFY( changeId != -1 );
          mKnownChangeIds.insert( changeId );
        }
        waitForSignals();
      }

      { // Invalid parameters
        changeId = mChanger->createIncidence( Incidence::Ptr(), // Invalid payload
                                              mCollection );
        QVERIFY( changeId == -1 );
        mKnownChangeIds.insert( changeId );
      }

    }

    void testDeleting()
    {
      int changeId;
      const Item::List incidences = mCalendar->rawIncidences();

      { // Delete 5 incidences, previously created
        foreach( const Item &item, incidences ) {
          mPendingDeletesInETM.append( item.payload<Incidence::Ptr>()->uid() );
        }
        changeId = mChanger->deleteIncidences( incidences );
        mKnownChangeIds.insert( changeId );
        QVERIFY( changeId != -1 );
        waitForSignals();
      }

      { // Delete something already deleted
        mWaitingForIncidenceChangerSignals = true;
        changeId = mChanger->deleteIncidences( incidences );
        mKnownChangeIds.insert( changeId );
        QVERIFY( changeId != -1 );
        mExpectedResult = IncidenceChanger2::ResultCodeAlreadyDeleted;
        waitForSignals();
      }

      { // If we provide an empty list, a job won't be created
        changeId = mChanger->deleteIncidences( Item::List() );
        mKnownChangeIds.insert( changeId );
        QVERIFY( changeId == -1 );
      }

      { // If we provide a list with at least one invalid item, a job won't be created
        Item::List list;
        list << Item();
        changeId = mChanger->deleteIncidences( list );
        QVERIFY( changeId == -1 );
      }

    }

    void testModifying()
    {
      int changeId;

      // First create an incidence
      const QString uid( "uid");
      const QString summary( "summary");
      Incidence::Ptr incidence( new Event() );
      incidence->setUid( uid );
      incidence->setSummary( summary );
      mPendingInsertsInETM.append( uid );
      changeId = mChanger->createIncidence( incidence,
                                            mCollection );
      QVERIFY( changeId != -1 );
      mKnownChangeIds.insert( changeId );
      waitForSignals();

      { // Just a summary change
        Item item = mCalendar->itemForIncidenceUid( uid );
        QVERIFY( item.isValid() );
        item.payload<Incidence::Ptr>()->setSummary( "summary2" );
        mPendingUpdatesInETM.append( uid );
        changeId = mChanger->modifyIncidence( item );
        QVERIFY( changeId != -1 );
        mKnownChangeIds.insert( changeId );
        waitForSignals();
        item = mCalendar->itemForIncidenceUid( uid );
        QVERIFY( item.isValid() );
        QVERIFY( item.payload<Incidence::Ptr>()->summary() == "summary2" );
      }

      { // Invalid item
        changeId = mChanger->modifyIncidence( Item() );
        QVERIFY( changeId == -1 );
      }

      { // Delete it and try do modify it, should result in error
        Item item = mCalendar->itemForIncidenceUid( uid );
        QVERIFY( item.isValid() );
        mPendingDeletesInETM.append( uid );
        changeId = mChanger->deleteIncidence( item );
        QVERIFY( changeId != -1 );
        mKnownChangeIds.insert( changeId );
        waitForSignals();

        mWaitingForIncidenceChangerSignals = true;
        changeId = mChanger->modifyIncidence( item );
        mKnownChangeIds.insert( changeId );
        mExpectedResult = IncidenceChanger2::ResultCodeAlreadyDeleted;
        QVERIFY( changeId != -1 );
        waitForSignals();

      }
    }


  public Q_SLOTS:

    void waitForSignals()
    {
      while ( !mPendingInsertsInETM.isEmpty() ||
              !mPendingUpdatesInETM.isEmpty() ||
              !mPendingDeletesInETM.isEmpty() ||
              mWaitingForIncidenceChangerSignals ) {
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

    void rowsAboutToBeRemoved( const QModelIndex &index, int start, int end )
    {
      Item::List list = itemsFromModel( mCalendarModel, index, start, end );
      foreach( const Item &item, list ) {
        Incidence::Ptr incidence = CalendarSupport::incidence( item );
        if ( incidence && mPendingDeletesInETM.contains( incidence->uid() ) )
          mPendingDeletesInETM.removeAll( incidence->uid() );
      }
    }

  void deleteFinished( int changeId,
                       const QVector<Akonadi::Item::Id> &deletedIds,
                       CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                       const QString &errorMessage )
  {
    Q_UNUSED( deletedIds );
    QVERIFY( mKnownChangeIds.contains( changeId ) );
    QVERIFY( changeId != -1 );

    if ( resultCode != IncidenceChanger2::ResultCodeSuccess ) {
      kDebug() << "Error string is " << errorMessage;
    } else {
      QVERIFY( !deletedIds.isEmpty() );
      foreach( Akonadi::Item::Id id , deletedIds ) {
        QVERIFY( id != -1 );
      }
    }

    QVERIFY( resultCode == mExpectedResult );
    mExpectedResult = IncidenceChanger2::ResultCodeSuccess;
    mWaitingForIncidenceChangerSignals = false;
  }

  void createFinished( int changeId,
                       const Akonadi::Item &item,
                       CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                       const QString &errorString )
  {
    QVERIFY( mKnownChangeIds.contains( changeId ) );
    QVERIFY( changeId != -1 );

    if ( resultCode == IncidenceChanger2::ResultCodeSuccess ) {
      QVERIFY( item.isValid() );
      QVERIFY( item.parentCollection().isValid() );
    } else {
      kDebug() << "Error string is " << errorString;
    }

    QVERIFY( resultCode == mExpectedResult );
    mExpectedResult = IncidenceChanger2::ResultCodeSuccess;
    mWaitingForIncidenceChangerSignals = false;
  }

  void modifyFinished( int changeId,
                       const Akonadi::Item &item,
                       CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                       const QString &errorString )
  {
    Q_UNUSED( item );
    QVERIFY( mKnownChangeIds.contains( changeId ) );
    QVERIFY( changeId != -1 );

    if ( resultCode == IncidenceChanger2::ResultCodeSuccess )
      QVERIFY( item.isValid() );
    else
      kDebug() << "Error string is " << errorString;

    QVERIFY( resultCode == mExpectedResult );

    mExpectedResult = IncidenceChanger2::ResultCodeSuccess;
    mWaitingForIncidenceChangerSignals = false;
  }
};

QTEST_AKONADIMAIN( IncidenceChangerTest, NoGUI )

#include "incidencechangertest.moc"
