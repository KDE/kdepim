/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>
  Copyright (C) 2010 Casey Link <casey@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "editorfreebusy.h"
#include "../editorconfig.h"
#include "freebusyurldialog.h"

#include "attendeedata.h"

#include <calendarsupport/freebusymanager.h>

#include <kdgantt2/kdganttgraphicsview.h>
#include <kdgantt2/kdganttdatetimegrid.h>
#include <kdgantt2/kdganttabstractrowcontroller.h>

#include <Akonadi/Contact/ContactGroupExpandJob>
#include <Akonadi/Contact/ContactGroupSearchJob>

#include <KPIMUtils/Email>

#include <kcalutils/stringify.h>

#include <KComboBox>
#include <KLocale>
#include <KMessageBox>
#include <KSystemTimeZone>

#include <QBoxLayout>
#include <QDateTime>
#include <QFrame>
#include <QLabel>
#include <KMenu>
#include <QPushButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QSplitter>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QScrollBar>

using namespace IncidenceEditorsNG;

class RowController : public KDGantt::AbstractRowController {
private:
    static const int ROW_HEIGHT ;
    QPointer<QAbstractItemModel> m_model;

public:
    RowController()
    {
      mRowHeight = 20;
    }

    void setModel( QAbstractItemModel* model )
    {
        m_model = model;
    }

    /*reimp*/int headerHeight() const { return 2*mRowHeight + 10; }

    /*reimp*/ bool isRowVisible( const QModelIndex& ) const { return true;}
    /*reimp*/ bool isRowExpanded( const QModelIndex& ) const { return false; }
    /*reimp*/ KDGantt::Span rowGeometry( const QModelIndex& idx ) const
    {
        return KDGantt::Span( idx.row()*mRowHeight, mRowHeight );
    }
    /*reimp*/ int maximumItemHeight() const {
        return mRowHeight/2;
    }
    /*reimp*/int totalHeight() const {
        return m_model->rowCount()* mRowHeight;
    }

    /*reimp*/ QModelIndex indexAt( int height ) const {
        return m_model->index( height/mRowHeight, 0 );
    }

    /*reimp*/ QModelIndex indexBelow( const QModelIndex& idx ) const {
        if ( !idx.isValid() )return QModelIndex();
        return idx.model()->index( idx.row()+1, idx.column(), idx.parent() );
    }
    /*reimp*/ QModelIndex indexAbove( const QModelIndex& idx ) const {
        if ( !idx.isValid() )return QModelIndex();
        return idx.model()->index( idx.row()-1, idx.column(), idx.parent() );
    }

    void setRowHeight( int height ) { mRowHeight = height; }

private:
    int mRowHeight;

};

class GanttHeaderView : public QHeaderView {
public:
    explicit GanttHeaderView( QWidget* parent=0 ) : QHeaderView( Qt::Horizontal, parent ) {
    }

    QSize sizeHint() const { QSize s = QHeaderView::sizeHint(); s.rheight() *= 2; return s; }
};

// The FreeBusyItem is the whole line for a given attendee.
// Individual "busy" periods are created as sub-items of this item.
//
// We can't use the CustomListViewItem base class, since we need a
// different inheritance hierarchy for supporting the Gantt view.
class FreeBusyItem
{
  public:
    FreeBusyItem( AttendeeData::Ptr attendee, QStandardItemModel *model,
                  EditorFreeBusy *parentWidget, QTreeWidget *listWidget ) :
      mAttendee( attendee ), mTimerID( 0 ), mIsDownloading( false ),
      mModel( model ), mParentWidget( parentWidget ), mListWidget( listWidget )
    {
      QStandardItem * dummyItem = new QStandardItem;
      dummyItem->setData( KDGantt::TypeTask, KDGantt::ItemTypeRole );
      mModel->appendRow( dummyItem );
      Q_ASSERT( attendee );
      updateItem();
      setFreeBusyPeriods( KCalCore::FreeBusy::Ptr() );
#if 0
      setDisplaySubitemsAsGroup( true );
      if ( listView () ) {
        listView ()->setRootIsDecorated( false );
      }
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif

    }
    ~FreeBusyItem() {}

    void updateItem();

    AttendeeData::Ptr attendee() const { return mAttendee; }
  void setFreeBusy( const KCalCore::FreeBusy::Ptr &fb ) { mFreeBusy = fb; }
    KCalCore::FreeBusy::Ptr freeBusy() const
    {
      return mFreeBusy;
    }

  void setFreeBusyPeriods( const KCalCore::FreeBusy::Ptr &fb );

    QString email() const { return mAttendee->email(); }
    void setUpdateTimerID( int id ) { mTimerID = id; }
    int updateTimerID() const { return mTimerID; }

    void startDownload( bool forceDownload ) {
      mIsDownloading = true;
      CalendarSupport::FreeBusyManager *m = CalendarSupport::FreeBusyManager::self();
      if ( !m->retrieveFreeBusy( attendee()->email(), forceDownload,
                                 mParentWidget ) ) {
        mIsDownloading = false;
      }
    }
    void setIsDownloading( bool d ) { mIsDownloading = d; }
    bool isDownloading() const { return mIsDownloading; }

  private:
    AttendeeData::Ptr mAttendee;
    KCalCore::FreeBusy::Ptr mFreeBusy;

    // This is used for the update timer
    int mTimerID;

    // Only run one download job at a time
    bool mIsDownloading;

    QStandardItemModel* mModel;
    EditorFreeBusy *mParentWidget;
    QTreeWidget *mListWidget;
};

void FreeBusyItem::updateItem()
{
  QString text = mAttendee->name() + " <" + mAttendee->email() + '>';
  QTreeWidgetItem* item = 0;
  int index = mParentWidget->indexOfItem( this );
  if ( index == -1 ||  mListWidget->topLevelItemCount() <= index ) {
    item = new QTreeWidgetItem( QStringList() << text );
    mListWidget->addTopLevelItem( item );
  } else {
    item = mListWidget->topLevelItem( index );
    item->setText( 0, text );
  }

  switch ( mAttendee->status() ) {
    case AttendeeData::Accepted:
      item->setIcon( 0, SmallIcon( "dialog-ok-apply" ) );
      break;
    case AttendeeData::Declined:
      item->setIcon( 0, SmallIcon( "dialog-cancel" ) );
      break;
    case AttendeeData::NeedsAction:
    case AttendeeData::InProcess:
      item->setIcon( 0, SmallIcon( "help-about" ) );
      break;
    case AttendeeData::Tentative:
      item->setIcon( 0, SmallIcon( "dialog-ok" ) );
      break;
    case AttendeeData::Delegated:
      item->setIcon( 0, SmallIcon( "mail-forward" ) );
      break;
    default:
      item->setIcon( 0, QPixmap() );
  }
}

// Set the free/busy periods for this attendee
void FreeBusyItem::setFreeBusyPeriods( const KCalCore::FreeBusy::Ptr &fb )
{
  if ( fb ) {
    KDateTime::Spec timeSpec = KSystemTimeZones::local();

    QList<QStandardItem *> newItems;
    // Evaluate free/busy information
    KCalCore::FreeBusyPeriod::List busyPeriods = fb->fullBusyPeriods();
    for ( KCalCore::FreeBusyPeriod::List::Iterator it = busyPeriods.begin();
          it != busyPeriods.end(); ++it ) {
      KCalCore::FreeBusyPeriod per = *it;

      QStandardItem* newItem = new QStandardItem;
      newItem->setData( KDGantt::TypeTask, KDGantt::ItemTypeRole );
      newItem->setData( per.start().toTimeSpec( timeSpec ).dateTime(), KDGantt::StartTimeRole );
      newItem->setData( per.end().toTimeSpec( timeSpec ).dateTime(), KDGantt::EndTimeRole );
      newItem->setData( Qt::red, Qt::BackgroundRole ); //TODO: doesn't work, probably we need a delegate

      QString toolTip = "<qt>";
      toolTip += "<b>" + i18nc( "@info:tooltip", "Free/Busy Period" ) + "</b>";
      toolTip += "<hr>";
      if ( !per.summary().isEmpty() ) {
        toolTip += "<i>" + i18nc( "@info:tooltip", "Summary:" ) + "</i>" + "&nbsp;";
        toolTip += per.summary();
        toolTip += "<br>";
      }
      if ( !per.location().isEmpty() ) {
        toolTip += "<i>" + i18nc( "@info:tooltip", "Location:" ) + "</i>" + "&nbsp;";
        toolTip += per.location();
        toolTip += "<br>";
      }
      toolTip += "<i>" + i18nc( "@info:tooltip period start time", "Start:" ) + "</i>" + "&nbsp;";
      toolTip += KGlobal::locale()->formatDateTime( per.start().toTimeSpec( timeSpec ).dateTime() );
      toolTip += "<br>";
      toolTip += "<i>" + i18nc( "@info:tooltip period end time", "End:" ) + "</i>" + "&nbsp;";
      toolTip += KGlobal::locale()->formatDateTime( per.end().toTimeSpec( timeSpec ).dateTime() );
      toolTip += "<br>";
      toolTip += "</qt>";
      newItem->setData( toolTip, Qt::ToolTipRole );
      newItems.append( newItem );
    }
    int index = mParentWidget->indexOfItem( this );
    mModel->removeRow( index );
    mModel->insertRow( index, newItems );
    setFreeBusy( fb );
  } else {
    setFreeBusy( KCalCore::FreeBusy::Ptr() );
  }

  // We are no longer downloading
  mIsDownloading = false;

}

////

EditorFreeBusy::EditorFreeBusy( int spacing, QWidget *parent )
  : QDialog( parent ),
    mGanttGrid( 0 ),
    mScaleCombo( 0 )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  // Label for status summary information
  // Uses the tooltip palette to highlight it
  mIsOrganizer = false; // Will be set later. This is just valgrind silencing
  mStatusSummaryLabel = new QLabel( this );
  mStatusSummaryLabel->setPalette( QToolTip::palette() );
  mStatusSummaryLabel->setFrameStyle( QFrame::Plain | QFrame::Box );
  mStatusSummaryLabel->setLineWidth( 1 );
  mStatusSummaryLabel->hide(); // Will be unhidden later if you are organizer
  topLayout->addWidget( mStatusSummaryLabel );

  // The control panel for the gantt widget
  QBoxLayout *controlLayout = new QHBoxLayout();
  controlLayout->setSpacing( topLayout->spacing() );
  topLayout->addItem( controlLayout );

  QLabel *label = new QLabel( i18nc( "@label", "Scale: " ), this );
  controlLayout->addWidget( label );

  mScaleCombo = new KComboBox( this );
  mScaleCombo->setToolTip(
    i18nc( "@info:tooltip", "Set the Gantt chart zoom level" ) );
  mScaleCombo->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select the Gantt chart zoom level from one of the following:<nl/>"
           "'Hour' shows a range of several hours,<nl/>"
           "'Day' shows a range of a few days,<nl/>"
           "'Week' shows a range of a few months,<nl/>"
           "and 'Month' shows a range of a few years,<nl/>"
           "while 'Automatic' selects the range most "
           "appropriate for the current event or to-do." ) );
  mScaleCombo->addItem( i18nc( "@item:inlistbox range in hours", "Hour" ),
                        QVariant::fromValue<int>( KDGantt::DateTimeGrid::ScaleHour ) );
  mScaleCombo->addItem( i18nc( "@item:inlistbox range in days", "Day" ),
                        QVariant::fromValue<int>( KDGantt::DateTimeGrid::ScaleDay ) );
  mScaleCombo->addItem( i18nc( "@item:inlistbox range in weeks", "Week" ),
                        QVariant::fromValue<int>( KDGantt::DateTimeGrid::ScaleWeek ) );
  mScaleCombo->addItem( i18nc( "@item:inlistbox range in months", "Month" ),
                        QVariant::fromValue<int>( KDGantt::DateTimeGrid::ScaleMonth ) );
  mScaleCombo->addItem( i18nc( "@item:inlistbox range is computed automatically", "Automatic" ),
                        QVariant::fromValue<int>( KDGantt::DateTimeGrid::ScaleAuto ) );
  mScaleCombo->setCurrentIndex( 0 ); // start with "hour"
  connect( mScaleCombo, SIGNAL( activated( int ) ), SLOT( slotScaleChanged( int ) ) );
  controlLayout->addWidget( mScaleCombo );

  QPushButton *button = new QPushButton( i18nc( "@action:button", "Center on Start" ), this );
  button->setToolTip(
    i18nc( "@info:tooltip",
           "Center the Gantt chart on the event start date and time" ) );
  button->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Click this button to center the Gantt chart on the start "
           "time and day of this event." ) );
  connect( button, SIGNAL(clicked()), SLOT(slotCenterOnStart()) );
  controlLayout->addWidget( button );

  controlLayout->addStretch( 1 );

  button = new QPushButton( i18nc( "@action:button", "Pick Date" ), this );
  button->setToolTip(
    i18nc( "@info:tooltip",
           "Move the event to a date and time when all "
           "attendees are available" ) );
  button->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Click this button to move the event to a date "
           "and time when all the attendees have time "
           "available in their Free/Busy lists." ) );
  connect( button, SIGNAL(clicked()), SLOT(slotPickDate()) );
  controlLayout->addWidget( button );

  controlLayout->addStretch( 1 );

  button = new QPushButton( i18nc( "@action:button reload freebusy data", "Reload" ), this );
  button->setToolTip(
    i18nc( "@info:tooltip",
           "Reload Free/Busy data for all attendees" ) );
  button->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Pressing this button will cause the Free/Busy data for all "
           "attendees to be reloaded from their corresponding servers." ) );
  controlLayout->addWidget( button );
  connect( button, SIGNAL(clicked()), SLOT(manualReload()) );

  QSplitter *splitter = new QSplitter( Qt::Horizontal, this );
  connect( splitter, SIGNAL( splitterMoved(int,int) ), SLOT( splitterMoved() ) );
  mLeftView = new QTreeWidget;
  mLeftView->setHeader( new GanttHeaderView );
  mLeftView->setHeaderLabel( i18nc( "@label:listbox A list showing attendees", "Attendee") );
  mLeftView->setRootIsDecorated( false );
  mLeftView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mLeftView->setContextMenuPolicy( Qt::CustomContextMenu );

  mGanttView = new KDGantt::GraphicsView();
  mGanttView->setObjectName( "mGanttView" );
  mGanttView->setToolTip(
    i18nc( "@info:tooltip",
           "Shows the Free/Busy status of all attendees" ) );
  mGanttView->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Shows the Free/Busy status of all attendees. "
           "Double-clicking on an attendee's entry in the "
           "list will allow you to enter the location of "
           "their Free/Busy Information." ) );
  QStandardItemModel *model = new QStandardItemModel( this );

  mRowController = new RowController;
  mRowController->setRowHeight( fontMetrics().height() ); //TODO: detect

  mRowController->setModel( model );
  mGanttView->setRowController( mRowController );

  mGanttGrid = new KDGantt::DateTimeGrid;
  mGanttGrid->setScale( KDGantt::DateTimeGrid::ScaleHour );
  mGanttGrid->setDayWidth( 800 );
  mGanttGrid->setRowSeparators( true );
  mGanttView->setGrid( mGanttGrid );
  mGanttView->setModel( model );
  mGanttView->viewport()->setFixedWidth( 800 * 30 );

  //TODO: an item delegate might be needed, see kotimelineview.cpp from korganizer

  splitter->addWidget( mLeftView );
  splitter->addWidget( mGanttView );

  topLayout->addWidget( splitter );
  topLayout->setStretchFactor( splitter, 100 );

  // Initially, show 15 days back and forth
  // set start to even hours, i.e. to 12:AM 0 Min 0 Sec
  QDateTime horizonStart =
    QDateTime( QDateTime::currentDateTime().addDays( -15 ).date() );
  QDateTime horizonEnd = QDateTime::currentDateTime().addDays( 15 );
  mGanttGrid->setStartDateTime( horizonStart );

#if 0
  // mEventRectangle is the colored rectangle representing the event being modified
  mEventRectangle = new KDIntervalColorRectangle( mGanttView );
  mEventRectangle->setColor( Qt::magenta );
  mGanttView->addIntervalBackgroundColor( mEventRectangle );
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif

  connect( mGanttView, SIGNAL(timeIntervalSelected(const QDateTime &,const QDateTime &)),
           mGanttView, SLOT(zoomToSelection(const QDateTime &,const  QDateTime &)) );
  connect( mGanttView, SIGNAL(doubleClicked(QModelIndex)),
           SLOT(editFreeBusyUrl(QModelIndex)) );
  connect( mGanttView, SIGNAL(intervalColorRectangleMoved(const QDateTime &,const QDateTime &)),
           this, SLOT(slotIntervalColorRectangleMoved(const QDateTime &,const QDateTime &)) );

  connect( mLeftView, SIGNAL(customContextMenuRequested(QPoint)),
           this, SLOT(showAttendeeStatusMenu()) );

  CalendarSupport::FreeBusyManager *m = CalendarSupport::FreeBusyManager::self();
  connect( m, SIGNAL(freeBusyRetrieved(KCalCore::FreeBusy *,const QString &)),
           SLOT(slotInsertFreeBusy(KCalCore::FreeBusy *,const QString &)) );


  connect( &mReloadTimer, SIGNAL(timeout()), SLOT(autoReload()) );
  mReloadTimer.setSingleShot( true );

#if 0
  //suppress the buggy consequences of clicks on the time header widget
  mGanttView->timeHeaderWidget()->installEventFilter( this );
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif
}

EditorFreeBusy::~EditorFreeBusy()
{
}

void EditorFreeBusy::removeAttendee( AttendeeData::Ptr attendee )
{
  FreeBusyItem *anItem = 0;
  for ( int i = 0; i < mFreeBusyItems.count(); i++ ) {
    anItem = mFreeBusyItems[i];
    if ( anItem->attendee() == attendee ) {
      if ( anItem->updateTimerID() != 0 ) {
        killTimer( anItem->updateTimerID() );
      }
      delete anItem;
      mFreeBusyItems.removeAt( i );
      mLeftView->removeItemWidget( mLeftView->topLevelItem( i ), 0 );
      updateStatusSummary();
      break;
    }
  }
}

void EditorFreeBusy::insertAttendee( AttendeeData::Ptr attendee, bool readFBList )
{
  FreeBusyItem *item = new FreeBusyItem( attendee, static_cast<QStandardItemModel*>( mGanttView->model() ), this , mLeftView );
  mFreeBusyItems.append( item );
  if ( readFBList ) {
    updateFreeBusyData( item );
  } else {
    clearSelection();
    mLeftView->topLevelItem( mLeftView->topLevelItemCount() - 1 )->setSelected( true );
  }
  updateStatusSummary();
}

void EditorFreeBusy::clearAttendees()
{
 static_cast<QStandardItemModel*>( mGanttView->model() )->clear();
 qDeleteAll( mFreeBusyItems );
 mFreeBusyItems.clear();
 mLeftView->clear();
}

void EditorFreeBusy::setUpdateEnabled( bool enabled )
{
#if 0
 mGanttView->setUpdateEnabled( enabled );
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif

}

bool EditorFreeBusy::updateEnabled() const
{
#if 0
  return mGanttView->getUpdateEnabled();
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif
return false; //TODO-NGPORT temp value, make compile
}

void EditorFreeBusy::slotIntervalColorRectangleMoved( const QDateTime &start,
                                                        const QDateTime &end )
{
  mDtStart = start;
  mDtEnd = end;
  emit dateTimesChanged( start, end );
}

void EditorFreeBusy::setDateTimes( const KDateTime &start, const KDateTime &end )
{
  kDebug() << "start: " << start.dateTime();
  slotUpdateGanttView( start.dateTime(), end.dateTime() );
}

void EditorFreeBusy::slotScaleChanged( int newScale )
{
  const QVariant var = mScaleCombo->itemData( newScale );
  Q_ASSERT( var.isValid() );

  int value = var.value<int>();

  mGanttGrid->setScale( (KDGantt::DateTimeGrid::Scale) value );
}

void EditorFreeBusy::slotCenterOnStart()
{
 KDGantt::DateTimeGrid *grid = static_cast<KDGantt::DateTimeGrid*>( mGanttView->grid() );
 int daysTo = grid->startDateTime().daysTo( mDtStart );
 mGanttView->horizontalScrollBar()->setValue( daysTo * 800 );
}

void EditorFreeBusy::slotZoomToTime()
{
#if 0
  mGanttView->zoomToFit();
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif
}

void EditorFreeBusy::updateFreeBusyData( FreeBusyItem *item )
{
  if ( item->isDownloading() ) {
    // This item is already in the process of fetching the FB list
    return;
  }

  if ( item->updateTimerID() != 0 ) {
    // An update timer is already running. Reset it
    killTimer( item->updateTimerID() );
  }

  // This item does not have a download running, and no timer is set
  // Do the download in five seconds
  item->setUpdateTimerID( startTimer( 5000 ) );
}

void EditorFreeBusy::timerEvent( QTimerEvent *event )
{
  killTimer( event->timerId() );
  Q_FOREACH ( FreeBusyItem * item, mFreeBusyItems ) {
    if ( item->updateTimerID() == event->timerId() ) {
      item->setUpdateTimerID( 0 );
      item->startDownload( mForceDownload );
      return;
    }
  }
}

// Set the Free Busy list for everyone having this email address
// If fb == 0, this disabled the free busy list for them
void EditorFreeBusy::slotInsertFreeBusy( const KCalCore::FreeBusy::Ptr &fb,
                                         const QString &email )
{
  if ( fb ) {
    fb->sortList();
  }
#if 0
  bool block = mGanttView->getUpdateEnabled();
  mGanttView->setUpdateEnabled( false );
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif
  Q_FOREACH ( FreeBusyItem *item, mFreeBusyItems ) {
    if ( item->email() == email ) {
      item->setFreeBusyPeriods( fb );
    }
  }
#if 0
  mGanttView->setUpdateEnabled( block );
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif
}

/*!
  Centers the Gantt view to the date/time passed in.
*/

void EditorFreeBusy::slotUpdateGanttView( const QDateTime &dtFrom, const QDateTime &dtTo )
{
  mDtStart = dtFrom;
  mDtEnd = dtTo;
  QDateTime horizonStart = QDateTime( dtFrom.addDays( -15 ).date() );
  KDGantt::DateTimeGrid *grid = static_cast<KDGantt::DateTimeGrid*>( mGanttView->grid() );
  grid->setStartDateTime( horizonStart );
  slotCenterOnStart();
#if 0
  bool block = mGanttView->getUpdateEnabled( );
  mGanttView->setUpdateEnabled( false );
  mGanttView->setHorizonStart( horizonStart );
  mGanttView->setHorizonEnd( dtTo.addDays( 15 ) );
  mEventRectangle->setDateTimes( dtFrom, dtTo );
  mGanttView->setUpdateEnabled( block );
  mGanttView->centerTimelineAfterShow( dtFrom );
#else
 mGanttGrid->setStartDateTime( horizonStart );
 kDebug() << "Disabled code, port to KDGantt2";
#endif
}

/*!
  This slot is called when the user clicks the "Pick a date" button.
*/
void EditorFreeBusy::slotPickDate()
{
  KDateTime::Spec timeSpec = KSystemTimeZones::local();
  KDateTime dtStart( mDtStart, timeSpec );
  KDateTime dtEnd( mDtEnd, timeSpec );
  KDateTime start = dtStart;
  KDateTime end = dtEnd;
  bool success = findFreeSlot( start, end );

  if ( success ) {
    if ( start == dtStart && end == dtEnd ) {
      KMessageBox::information(
        this,
        i18nc( "@info", "The meeting already has suitable start/end times." ),
        QString(),
        "MeetingTimeOKFreeBusy" );
    } else {
      if ( KMessageBox::questionYesNo(
             this,
             i18nc( "@info",
                    "The next available time slot for the meeting is:<nl/>"
                    "Start: %1<nl/>End: %2<nl/>"
                    "Would you like to move the meeting to this time slot?",
                    start.dateTime().toString(), end.dateTime().toString() ), QString(),
             KStandardGuiItem::yes(), KStandardGuiItem::no(),
             "MeetingMovedFreeBusy" ) == KMessageBox::Yes ) {
        emit dateTimesChanged( start.dateTime(), end.dateTime() );
        slotUpdateGanttView( start.dateTime(), end.dateTime() );
      }
    }
  } else {
    KMessageBox::sorry( this, i18nc( "@info", "No suitable date found." ) );
  }
}

/*!
  Finds a free slot in the future which has at least the same size as
  the initial slot.
*/
bool EditorFreeBusy::findFreeSlot( KDateTime &dtFrom, KDateTime &dtTo )
{
  if ( tryDate( dtFrom, dtTo ) ) {
    // Current time is acceptable
    return true;
  }

  KDateTime tryFrom = dtFrom;
  KDateTime tryTo = dtTo;

  // Make sure that we never suggest a date in the past, even if the
  // user originally scheduled the meeting to be in the past.
  KDateTime now = KDateTime::currentUtcDateTime();
  if ( tryFrom < now ) {
    // The slot to look for is at least partially in the past.
    int secs = tryFrom.secsTo( tryTo );
    tryFrom = now;
    tryTo = tryFrom.addSecs( secs );
  }

  bool found = false;
  while ( !found ) {
    found = tryDate( tryFrom, tryTo );
    // PENDING(kalle) Make the interval configurable
    if ( !found && dtFrom.daysTo( tryFrom ) > 365 ) {
      break; // don't look more than one year in the future
    }
  }

  dtFrom = tryFrom;
  dtTo = tryTo;

  return found;
}

/*!
  Checks whether the slot specified by (tryFrom, tryTo) is free
  for all participants. If yes, return true. If at least one
  participant is found for which this slot is occupied, this method
  returns false, and (tryFrom, tryTo) contain the next free slot for
  that participant. In other words, the returned slot does not have to
  be free for everybody else.
*/
bool EditorFreeBusy::tryDate( KDateTime &tryFrom, KDateTime &tryTo )
{
  Q_FOREACH( FreeBusyItem * currentItem, mFreeBusyItems ) {
    if ( !tryDate( currentItem, tryFrom, tryTo ) ) {
      return false;
    }
  }
  return true;
}

/*!
  Checks whether the slot specified by (tryFrom, tryTo) is available
  for the participant specified with attendee. If yes, return true. If
  not, return false and change (tryFrom, tryTo) to contain the next
  possible slot for this participant (not necessarily a slot that is
  available for all participants).
*/
bool EditorFreeBusy::tryDate( FreeBusyItem *attendee,
                                KDateTime &tryFrom, KDateTime &tryTo )
{
  // If we don't have any free/busy information, assume the
  // participant is free. Otherwise a participant without available
  // information would block the whole allocation.
  KCalCore::FreeBusy::Ptr fb = attendee->freeBusy();
  if ( !fb ) {
    return true;
  }

  KCalCore::Period::List busyPeriods = fb->busyPeriods();
  for ( KCalCore::Period::List::Iterator it = busyPeriods.begin();
       it != busyPeriods.end(); ++it ) {
    if ( (*it).end() <= tryFrom || // busy period ends before try period
         (*it).start() >= tryTo ) { // busy period starts after try period
      continue;
    } else {
      // the current busy period blocks the try period, try
      // after the end of the current busy period
      int secsDuration = tryFrom.secsTo( tryTo );
      tryFrom = (*it).end();
      tryTo = tryFrom.addSecs( secsDuration );
      // try again with the new try period
      tryDate( attendee, tryFrom, tryTo );
      // we had to change the date at least once
      return false;
    }
  }

  return true;
}

void EditorFreeBusy::updateStatusSummary()
{
  int total = 0;
  int accepted = 0;
  int tentative = 0;
  int declined = 0;
  Q_FOREACH ( FreeBusyItem * aItem, mFreeBusyItems ) {
    ++total;
    switch( aItem->attendee()->status() ) {
    case AttendeeData::Accepted:
      ++accepted;
      break;
    case AttendeeData::Tentative:
      ++tentative;
      break;
    case AttendeeData::Declined:
      ++declined;
      break;
    case AttendeeData::NeedsAction:
    case AttendeeData::Delegated:
    case AttendeeData::Completed:
    case AttendeeData::InProcess:
    case AttendeeData::None:
      /* just to shut up the compiler */
      break;
    }
  }
  if ( total > 1 && mIsOrganizer ) {
    mStatusSummaryLabel->show();
    mStatusSummaryLabel->setText( i18nc( "@label",
                                         "Of the %1 participants, "
                                         "%2 have accepted, "
                                         "%3 have tentatively accepted, and "
                                         "%4 have declined.",
                                         total, accepted, tentative, declined ) );
  } else {
    mStatusSummaryLabel->hide();
  }
  mStatusSummaryLabel->adjustSize();
}

void EditorFreeBusy::triggerReload()
{
  mReloadTimer.start( 1000 );
}

void EditorFreeBusy::cancelReload()
{
  mReloadTimer.stop();
}

void EditorFreeBusy::manualReload()
{
  mForceDownload = true;
  reload();
}

void EditorFreeBusy::autoReload()
{
  mForceDownload = false;
  reload();
}

void EditorFreeBusy::reload()
{
  Q_FOREACH( FreeBusyItem * item, mFreeBusyItems ) {
    if ( mForceDownload ) {
      item->startDownload( mForceDownload );
    } else {
      updateFreeBusyData( item );
    }
  }
}


void EditorFreeBusy::editFreeBusyUrl( const QModelIndex & index )
{
  if ( index.row() < 0 || index.row() >= mFreeBusyItems.count() ) {
    return;
  }

FreeBusyItem *item = mFreeBusyItems[ index.row() ];

  AttendeeData::Ptr attendee = item->attendee();
  QPointer<FreeBusyUrlDialog> dialog = new FreeBusyUrlDialog( attendee, this );
  dialog->exec();
  delete dialog;
}

FreeBusyItem* EditorFreeBusy::selectedItem() const
{
  if ( mLeftView->selectedItems().isEmpty() )
    return 0;
  int index = mLeftView->indexOfTopLevelItem( mLeftView->selectedItems().at(0) );
  if ( index == -1 )
    return 0;
  return mFreeBusyItems[index];
}

AttendeeData::Ptr EditorFreeBusy::currentAttendee() const
{
  FreeBusyItem *aItem = selectedItem();
  if ( !aItem ) {
    return AttendeeData::Ptr();
  }
  return aItem->attendee();
}

void EditorFreeBusy::updateCurrentItem()
{
  FreeBusyItem *item = selectedItem();
  if ( item ) {
    item->updateItem();
    updateFreeBusyData( item );
    updateStatusSummary();
  }
}

void EditorFreeBusy::removeAttendee()
{
  FreeBusyItem *item = selectedItem();
  if ( !item ) {
    return;
  }

  FreeBusyItem *nextSelectedItem = 0;
  if( mFreeBusyItems.count() == 1 ) {
    nextSelectedItem = 0;
  }
  if( mFreeBusyItems.count() > 1 && item == mFreeBusyItems.last() ) {
    nextSelectedItem = mFreeBusyItems.first();
  } else {
    nextSelectedItem = mFreeBusyItems[ mFreeBusyItems.indexOf( item ) + 1 ];
  }


  AttendeeData::Ptr delA ( new AttendeeData( item->attendee()->name(), item->attendee()->email(),
                                 item->attendee()->RSVP(), item->attendee()->status(),
                                 item->attendee()->role(), item->attendee()->uid() ) );
  // TODO-NGPORT mDelAttendees.append( delA );
  delete item;

  updateStatusSummary();
  if( nextSelectedItem ) {
      mLeftView->topLevelItem( mFreeBusyItems.indexOf( nextSelectedItem ) )->setSelected( true );
  }
}

void EditorFreeBusy::clearSelection() const
{
  mLeftView->clearSelection();
  mGanttView->repaint();
}

void EditorFreeBusy::changeStatusForMe( AttendeeData::PartStat status )
{
  const QStringList myEmails = IncidenceEditors::EditorConfig::instance()->allEmails();
  Q_FOREACH ( FreeBusyItem *item, mFreeBusyItems ) {
    for ( QStringList::ConstIterator it2( myEmails.begin() ), end( myEmails.end() );
          it2 != end; ++it2 ) {
      if ( item->attendee()->email() == *it2 ) {
        item->attendee()->setStatus( status );
        item->updateItem();
      }
    }
  }
}

void EditorFreeBusy::showAttendeeStatusMenu()
{
#if 0
//TODO: see the hasExampleAttendee() note
  if( !currentAttendee() || selectedItem() == hasExampleAttendee() ) {
     return;
  }
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif

  KMenu *menu = new KMenu( 0 );

  QAction *needsaction =
    menu->addAction( SmallIcon( "help-about" ),
                     KCalUtils::Stringify::attendeeStatus( AttendeeData::NeedsAction ) );
  QAction *accepted =
    menu->addAction( SmallIcon( "dialog-ok-apply" ),
                     KCalUtils::Stringify::attendeeStatus( AttendeeData::Accepted ) );
  QAction *declined =
    menu->addAction( SmallIcon( "dialog-cancel" ),
                     KCalUtils::Stringify::attendeeStatus( AttendeeData::Declined ) );
  QAction *tentative =
    menu->addAction( SmallIcon( "dialog-ok" ),
                     KCalUtils::Stringify::attendeeStatus( AttendeeData::Tentative ) );
  QAction *delegated =
    menu->addAction( SmallIcon( "mail-forward" ),
                     KCalUtils::Stringify::attendeeStatus( AttendeeData::Delegated ) );
  QAction *completed =
    menu->addAction( SmallIcon( "mail-mark-read" ),
                     KCalUtils::Stringify::attendeeStatus( AttendeeData::Completed ) );
  QAction *inprocess =
    menu->addAction( SmallIcon( "help-about" ),
                     KCalUtils::Stringify::attendeeStatus( AttendeeData::InProcess ) );
  QAction *ret = menu->exec( QCursor::pos() );
  delete menu;
  if ( ret == needsaction ) {
    currentAttendee()->setStatus( AttendeeData::NeedsAction );
  } else if ( ret == accepted ) {
    currentAttendee()->setStatus( AttendeeData::Accepted );
  } else if ( ret == declined ) {
    currentAttendee()->setStatus( AttendeeData::Declined );
  } else if ( ret == tentative ) {
    currentAttendee()->setStatus( AttendeeData::Tentative );
  } else if ( ret == delegated ) {
    currentAttendee()->setStatus( AttendeeData::Delegated );
  } else if ( ret == completed ) {
    currentAttendee()->setStatus( AttendeeData::Completed );
  } else if ( ret == inprocess ) {
    currentAttendee()->setStatus( AttendeeData::InProcess );
  } else {
    return;
  }

  updateCurrentItem();
}

void EditorFreeBusy::slotOrganizerChanged( const QString &newOrganizer )
{
  if ( newOrganizer == mCurrentOrganizer ) {
    return;
  }

  QString name;
  QString email;
  bool success = KPIMUtils::extractEmailAddressAndName( newOrganizer, email, name );

  if ( !success ) {
    return;
  }

  AttendeeData::Ptr currentOrganizerAttendee;
  AttendeeData::Ptr newOrganizerAttendee;

  Q_FOREACH( FreeBusyItem* item, mFreeBusyItems ) {
    AttendeeData::Ptr attendee = item->attendee();
    if( attendee->fullName() == mCurrentOrganizer ) {
      currentOrganizerAttendee = attendee;
    }

    if( attendee->fullName() == newOrganizer ) {
      newOrganizerAttendee = attendee;
    }

  }


  int answer = KMessageBox::No;
  if ( currentOrganizerAttendee ) {
    answer = KMessageBox::questionYesNo(
      this,
      i18nc( "@option",
             "You are changing the organizer of this event. "
             "Since the organizer is also attending this event, would you "
             "like to change the corresponding attendee as well?" ) );
  } else {
    answer = KMessageBox::Yes;
  }

  if ( answer == KMessageBox::Yes ) {
    if ( currentOrganizerAttendee ) {
      removeAttendee( currentOrganizerAttendee );
    }

    if ( !newOrganizerAttendee ) {
      AttendeeData::Ptr a( new AttendeeData( name, email, true ) );
      insertAttendee( a, true );
     // TODO-NGPORT mNewAttendees.append( a );
      // TODO-NGPORT updateAttendee();
    }
  }

  mCurrentOrganizer = newOrganizer;
}

// Q3ListViewItem *EditorFreeBusy::hasExampleAttendee() const
// {
// #if 0
// //TODO: finish porting. the upper class deals with Q3ListViewItem, and it should deal with something else. Here FreeBusyItem.
//   Q_FOREACH ( FreeBusyItem *item, mFreeBusyItems ) {
//     AttendeeData::Ptr attendee = item->attendee();
//     Q_ASSERT( attendee );
//     if ( isExampleAttendee( attendee ) ) {
//         return item;
//     }
//   }
// #else
//  kDebug() << "Disabled code, port to KDGantt2";
// #endif
//
//   return 0; //TODO-NGPORT
// }

bool EditorFreeBusy::eventFilter( QObject *watched, QEvent *event )
{
#if 0
  if ( watched == mGanttView->timeHeaderWidget() &&
       event->type() >= QEvent::MouseButtonPress && event->type() <= QEvent::MouseMove ) {
    return true;
  } else {
    return AttendeeEditor::eventFilter( watched, event );
  }
#else
// kDebug() << "Disabled code, port to KDGantt2";
#endif
    return false;
}

int EditorFreeBusy::indexOfItem(FreeBusyItem* item)
{
  if ( mFreeBusyItems.isEmpty() )
    return -1;
  return mFreeBusyItems.indexOf( item );
}

void EditorFreeBusy::splitterMoved()
{
  mLeftView->setColumnWidth( 0, mLeftView->width() );
}


#include "editorfreebusy.moc"
