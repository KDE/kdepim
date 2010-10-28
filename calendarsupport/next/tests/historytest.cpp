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
#include <calendarsupport/next/history.h>
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

class HistoryTest : public QObject
{
  Q_OBJECT
  Collection mCollection;
  History *mHistory;
  CalendarModel *mCalendarModel;
  CalendarSupport::Calendar *mCalendar;

  // List of incidence uids.
  QStringList mPendingInsertsInETM;
  QStringList mPendingUpdatesInETM;
  QStringList mPendingDeletesInETM;

  bool mWaitingForHistorySignals;
  History::ResultCode mExpectedResult;

  private slots:
    void initTestCase()
    {
      mWaitingForHistorySignals = false;
      mExpectedResult = History::ResultCodeSuccess;
      //Control::start(); //TODO: uncomment when using testrunner
      qRegisterMetaType<CalendarSupport::History::ResultCode>("CalendarSupport::History::ResultCode");
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

      IncidenceChanger2 *changer = new IncidenceChanger2();
      changer->setShowDialogsOnError( false );
      mHistory = new History( changer );

      connect( mCalendarModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
               SLOT(rowsInserted(QModelIndex,int,int)) );

      connect( mCalendarModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               SLOT(dataChanged(QModelIndex,QModelIndex)) );

      connect( mCalendarModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
               SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );

      connect( mHistory, SIGNAL(redone(CalendarSupport::History::ResultCode)),
               SLOT(undone(CalendarSupport::History::ResultCode)) );

      // no need for a redone slot, it's the same code
      connect( mHistory, SIGNAL(undone(CalendarSupport::History::ResultCode)),
               SLOT(undone(CalendarSupport::History::ResultCode)) );
    }

    void createIncidence( const QString &uid )
    {
      Event::Ptr event( new Event() );
      event->setUid( uid );
      event->setSummary( "Foo" );

      Item item;
      item.setPayload<Incidence::Ptr>( event );
      item.setMimeType( event->mimeType() );

      ItemCreateJob *job = new ItemCreateJob( item, mCollection );
      AKVERIFYEXEC( job );
    }

    void deleteIncidence( const QString &uid )
    {
      Item item = mCalendar->itemForIncidenceUid( uid );
      QVERIFY( item.isValid() );

      ItemDeleteJob *job = new ItemDeleteJob( item );
      AKVERIFYEXEC( job );
    }

  void testUndoAndRedo()
  {
    QPushButton *undoButton = new QPushButton();
    QPushButton *redoButton = new QPushButton();
    undoButton->setEnabled( true );
    redoButton->setEnabled( true );

    mHistory->registerUndoWidget( undoButton );
    mHistory->registerRedoWidget( redoButton );

    QVERIFY( !undoButton->isEnabled() );
    QVERIFY( !redoButton->isEnabled() );

    QVERIFY( mHistory->clear() );
    QVERIFY( mHistory->lastErrorString().isEmpty() );

    QVERIFY( !undoButton->isEnabled() );
    QVERIFY( !redoButton->isEnabled() );

    // Create.
    const QString uid = "Some uid.";
    mPendingInsertsInETM.append( uid );
    createIncidence( uid );
    waitForETMorSignals();

    Item oldItem;
    Item newItem = mCalendar->itemForIncidenceUid( uid );
    QVERIFY( newItem.isValid() );

    mHistory->recordCreation( newItem );
    QVERIFY( undoButton->isEnabled() );
    QVERIFY( !redoButton->isEnabled() );

    // delete it
    mWaitingForHistorySignals = true;
    mPendingDeletesInETM.append( uid );
    QVERIFY( mHistory->undo() );
    waitForETMorSignals();
    QVERIFY( !undoButton->isEnabled() );
    QVERIFY( redoButton->isEnabled() );

    // redo, lets create it again
    mWaitingForHistorySignals = true;
    mPendingInsertsInETM.append( uid );
    QVERIFY( mHistory->redo() );
    waitForETMorSignals();
    QVERIFY( undoButton->isEnabled() );
    QVERIFY( !redoButton->isEnabled() );

    // It was created again, but the entry has a new akonadi id
    // the old id should have been updated, so this undo should work
    mWaitingForHistorySignals = true;
    mPendingDeletesInETM.append( uid );
    QVERIFY( mHistory->undo() );
    waitForETMorSignals();
    QVERIFY( !undoButton->isEnabled() );
    QVERIFY( redoButton->isEnabled() );

    // Good. Lets add it again, and test editing it.
    mWaitingForHistorySignals = true;
    mPendingInsertsInETM.append( uid );
    QVERIFY( mHistory->redo() );
    waitForETMorSignals();
    QVERIFY( undoButton->isEnabled() );
    QVERIFY( !redoButton->isEnabled() );

    oldItem = mCalendar->itemForIncidenceUid( uid );
    newItem = oldItem;
    QVERIFY( oldItem.isValid() );

    Incidence::Ptr oldPayload = oldItem.payload<Incidence::Ptr>();
    Incidence::Ptr newPayload( oldPayload->clone() );

    const QString summary( QLatin1String( "new summary" ) );
    const QString originalSummary = newPayload->summary();
    newPayload->setSummary( summary );

    newItem.setPayload<Incidence::Ptr>( newPayload );
    mPendingUpdatesInETM.append( uid );
    ItemModifyJob *modifyJob = new ItemModifyJob( newItem );
    AKVERIFYEXEC( modifyJob );
    waitForETMorSignals();

    Item item = mCalendar->itemForIncidenceUid( uid );
    QVERIFY( item.isValid() );
    QCOMPARE( item.payload<Incidence::Ptr>()->summary(), summary );

    mHistory->recordModification( oldItem, item );
    QVERIFY( undoButton->isEnabled() );
    QVERIFY( !redoButton->isEnabled() );

    kDebug() << "undo.";
    mWaitingForHistorySignals = true;
    mPendingUpdatesInETM.append( uid );
    QVERIFY( mHistory->undo() );
    waitForETMorSignals();
    QVERIFY( undoButton->isEnabled() );
    QVERIFY( redoButton->isEnabled() );

    item = mCalendar->itemForIncidenceUid( uid );
    QVERIFY( item.isValid() );
    QCOMPARE( item.payload<Incidence::Ptr>()->summary(), originalSummary );

    mWaitingForHistorySignals = true;
    mPendingUpdatesInETM.append( uid );
    QVERIFY( mHistory->redo() );
    waitForETMorSignals();
    item = mCalendar->itemForIncidenceUid( uid );
    QCOMPARE( item.payload<Incidence::Ptr>()->summary(), summary );

    mWaitingForHistorySignals = true;
    mPendingUpdatesInETM.append( uid );
    QVERIFY( mHistory->undo() );
    waitForETMorSignals();
    item = mCalendar->itemForIncidenceUid( uid );
    QCOMPARE( item.payload<Incidence::Ptr>()->summary(), originalSummary );

    kDebug() << "Deleting.";
    mPendingDeletesInETM.append( uid );
    deleteIncidence( uid );
    waitForETMorSignals();
    mHistory->recordDeletion( item );
    item = mCalendar->itemForIncidenceUid( uid );
    QVERIFY( !item.isValid() );

    kDebug() << "Calling redo on an empty stack.";
    // redo stack should be empty
    QVERIFY( !mHistory->redo() );

    kDebug() << "Recreating. Undoing delete.";
    mPendingInsertsInETM.append( uid );
    mWaitingForHistorySignals = true;
    QVERIFY( mHistory->undo() );
    waitForETMorSignals();
    item = mCalendar->itemForIncidenceUid( uid );
    QVERIFY( item.isValid() );

    kDebug() << "Re-deleting. Redoing delete.";
    mPendingDeletesInETM.append( uid );
    mWaitingForHistorySignals = true;
    QVERIFY( mHistory->redo() );
    waitForETMorSignals();
    item = mCalendar->itemForIncidenceUid( uid );
    QVERIFY( !item.isValid() );

    // Now lets test yo delete something that doesn't exist, maybe deleted by another person,
    // we try to undo an add, but there's
    // nothing to delete, because we didn't really add any incidence
    // to akonadi
    Item item2;
    item2.setId( 3333 );
    Incidence::Ptr payload2( new Event() );
    payload2->setUid( "payload2" );
    item2.setPayload( payload2 );
    item2.setMimeType( "application/x-vnd.akonadi.calendar.event" );
    // Fake create
    mHistory->recordCreation( item2 );
    mWaitingForHistorySignals = true;
    mExpectedResult = History::ResultCodeError;
    QVERIFY( mHistory->undo() );
    waitForETMorSignals();

    kDebug() << "Editing something that doesn't exist";
    mHistory->recordModification( item2, item2  );
    mExpectedResult = History::ResultCodeError;
    mWaitingForHistorySignals = true;
    QVERIFY( mHistory->undo() );
    waitForETMorSignals();

    QVERIFY( undoButton->isEnabled() );
    QVERIFY( !redoButton->isEnabled() );
  }

  public Q_SLOTS:

    void waitForETMorSignals()
    {
      while ( !mPendingInsertsInETM.isEmpty() ||
              !mPendingUpdatesInETM.isEmpty() ||
              !mPendingDeletesInETM.isEmpty() ||
              mWaitingForHistorySignals ) {
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

    void undone( CalendarSupport::History::ResultCode result )
    {
      mWaitingForHistorySignals = false;

      if ( result == History::ResultCodeSuccess ) {
        QVERIFY( mHistory->lastErrorString().isEmpty() );
      } else {
        qDebug() << "last error string is " << mHistory->lastErrorString();
        QVERIFY( !mHistory->lastErrorString().isEmpty() );
      }

      QVERIFY( result == mExpectedResult );
      mExpectedResult = History::ResultCodeSuccess;
    }

};

// For undo/redo buttons
QTEST_AKONADIMAIN( HistoryTest, GUI )

#include "historytest.moc"
