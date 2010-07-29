/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include "visualfreebusywidget.h"

#include "conflictresolver.h"
#include "freebusyitem.h"

#include <kdgantt2/kdganttgraphicsview.h>
#include <kdgantt2/kdganttdatetimegrid.h>
#include <kdgantt2/kdganttabstractrowcontroller.h>

#include <KSystemTimeZones>
#include <KComboBox>
#include <KLocale>
#include <KMenu>
#include <KCal/Attendee>
#include <KCal/FreeBusy>
#include <KMessageBox>
#include <KDebug>

#include <QTreeWidget>
#include <QAction>
#include <QBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QSplitter>
#include <QPushButton>
#include <QHeaderView>
#include <QScrollBar>

using namespace IncidenceEditorsNG;

class RowController : public KDGantt::AbstractRowController
{
private:
    static const int ROW_HEIGHT ;
    QPointer<QAbstractItemModel> m_model;

public:
    RowController() {
        mRowHeight = 20;
    }

    void setModel( QAbstractItemModel* model ) {
        m_model = model;
    }

    /*reimp*/int headerHeight() const { return 2*mRowHeight + 10; }

    /*reimp*/ bool isRowVisible( const QModelIndex& ) const { return true;}
    /*reimp*/ bool isRowExpanded( const QModelIndex& ) const { return false; }
    /*reimp*/ KDGantt::Span rowGeometry( const QModelIndex& idx ) const {
        return KDGantt::Span( idx.row()*mRowHeight, mRowHeight );
    }
    /*reimp*/ int maximumItemHeight() const {
        return mRowHeight / 2;
    }
    /*reimp*/int totalHeight() const {
        return m_model->rowCount()* mRowHeight;
    }

    /*reimp*/ QModelIndex indexAt( int height ) const {
        return m_model->index( height / mRowHeight, 0 );
    }

    /*reimp*/ QModelIndex indexBelow( const QModelIndex& idx ) const {
        if ( !idx.isValid() )return QModelIndex();
        return idx.model()->index( idx.row() + 1, idx.column(), idx.parent() );
    }
    /*reimp*/ QModelIndex indexAbove( const QModelIndex& idx ) const {
        if ( !idx.isValid() )return QModelIndex();
        return idx.model()->index( idx.row() - 1, idx.column(), idx.parent() );
    }

    void setRowHeight( int height ) { mRowHeight = height; }

private:
    int mRowHeight;

};

class GanttHeaderView : public QHeaderView
{
public:
    explicit GanttHeaderView( QWidget* parent = 0 ) : QHeaderView( Qt::Horizontal, parent ) {
    }

    QSize sizeHint() const { QSize s = QHeaderView::sizeHint(); s.rheight() *= 2; return s; }
};

VisualFreeBusyWidget::VisualFreeBusyWidget( ConflictResolver* resolver, int spacing, QWidget* parent )
        : QWidget( parent ),
        mGanttGrid( 0 ),
        mScaleCombo( 0 ),
        mResolver( resolver )
{
    QVBoxLayout *topLayout = new QVBoxLayout( this );
    topLayout->setSpacing( spacing );

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
    connect( button, SIGNAL( clicked() ), SLOT( slotCenterOnStart() ) );
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
    connect( button, SIGNAL( clicked() ), SLOT( slotPickDate() ) );
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
    connect( button, SIGNAL( clicked() ), SLOT( manualReload() ) );

    QSplitter *splitter = new QSplitter( Qt::Horizontal, this );
    connect( splitter, SIGNAL( splitterMoved( int, int ) ), SLOT( splitterMoved() ) );
    mLeftView = new QTreeWidget;
    mLeftView->setHeader( new GanttHeaderView );
    mLeftView->setHeaderLabel( i18n( "Attendee" ) );
    mLeftView->setRootIsDecorated( false );
    mLeftView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    mLeftView->setContextMenuPolicy( Qt::CustomContextMenu );

    mGanttView = new KDGantt::GraphicsView( this );
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
    mModel = new QStandardItemModel( this );

    mRowController = new RowController;
    mRowController->setRowHeight( fontMetrics().height() ); //TODO: detect

    mRowController->setModel( mModel );
    mGanttView->setRowController( mRowController );

    mGanttGrid = new KDGantt::DateTimeGrid;
    mGanttGrid->setScale( KDGantt::DateTimeGrid::ScaleHour );
    mGanttGrid->setDayWidth( 800 );
    mGanttGrid->setRowSeparators( true );
    mGanttView->setGrid( mGanttGrid );
    mGanttView->setModel( mModel );
    mGanttView->viewport()->setFixedWidth( 800 * 30 );

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

    connect( mGanttView, SIGNAL( timeIntervalSelected( const KDateTime &, const KDateTime & ) ),
             mGanttView, SLOT( zoomToSelection( const KDateTime &, const  KDateTime & ) ) );
    connect( mGanttView, SIGNAL( doubleClicked( QModelIndex ) ),
             SLOT( editFreeBusyUrl( QModelIndex ) ) );
    connect( mGanttView, SIGNAL( intervalColorRectangleMoved( const KDateTime &, const KDateTime & ) ),
             this, SLOT( slotIntervalColorRectangleMoved( const KDateTime &, const KDateTime & ) ) );

    connect( mLeftView, SIGNAL( customContextMenuRequested( QPoint ) ),
             this, SLOT( showAttendeeStatusMenu() ) );

    foreach( FreeBusyItem::Ptr item, mResolver->freeBusyItems() ) {
        newFreeBusy( item );
    }

}

VisualFreeBusyWidget::~VisualFreeBusyWidget()
{

}

void VisualFreeBusyWidget::showAttendeeStatusMenu()
{
    KMenu *menu = new KMenu( 0 );

    QAction *needsaction =
        menu->addAction( SmallIcon( "help-about" ),
                         KCal::Attendee::statusName( KCal::Attendee::NeedsAction ) );
    QAction *accepted =
        menu->addAction( SmallIcon( "dialog-ok-apply" ),
                         KCal::Attendee::statusName( KCal::Attendee::Accepted ) );
    QAction *declined =
        menu->addAction( SmallIcon( "dialog-cancel" ),
                         KCal::Attendee::statusName( KCal::Attendee::Declined ) );
    QAction *tentative =
        menu->addAction( SmallIcon( "dialog-ok" ),
                         KCal::Attendee::statusName( KCal::Attendee::Tentative ) );
    QAction *delegated =
        menu->addAction( SmallIcon( "mail-forward" ),
                         KCal::Attendee::statusName( KCal::Attendee::Delegated ) );
    QAction *completed =
        menu->addAction( SmallIcon( "mail-mark-read" ),
                         KCal::Attendee::statusName( KCal::Attendee::Completed ) );
    QAction *inprocess =
        menu->addAction( SmallIcon( "help-about" ),
                         KCal::Attendee::statusName( KCal::Attendee::InProcess ) );
//   QAction *ret = menu->exec( QCursor::pos() );
//   delete menu;
//   if ( ret == needsaction ) {
//     currentAttendee()->setStatus( KCal::Attendee::NeedsAction );
//   } else if ( ret == accepted ) {
//     currentAttendee()->setStatus( KCal::Attendee::Accepted );
//   } else if ( ret == declined ) {
//     currentAttendee()->setStatus( KCal::Attendee::Declined );
//   } else if ( ret == tentative ) {
//     currentAttendee()->setStatus( KCal::Attendee::Tentative );
//   } else if ( ret == delegated ) {
//     currentAttendee()->setStatus( KCal::Attendee::Delegated );
//   } else if ( ret == completed ) {
//     currentAttendee()->setStatus( KCal::Attendee::Completed );
//   } else if ( ret == inprocess ) {
//     currentAttendee()->setStatus( KCal::Attendee::InProcess );
//   } else {
//     return;
//   }

//   updateCurrentItem();
}

void VisualFreeBusyWidget::slotCenterOnStart()
{
    KDGantt::DateTimeGrid *grid = static_cast<KDGantt::DateTimeGrid*>( mGanttView->grid() );
    int daysTo = grid->startDateTime().daysTo( mDtStart.dateTime() );
    mGanttView->horizontalScrollBar()->setValue( daysTo * 800 );
}

void VisualFreeBusyWidget::slotIntervalColorRectangleMoved( const KDateTime& start, const KDateTime& end )
{
    mDtStart = start;
    mDtEnd = end;
    emit dateTimesChanged( start, end );
}

/*!
  This slot is called when the user clicks the "Pick a date" button.
*/
void VisualFreeBusyWidget::slotPickDate()
{
    KDateTime::Spec timeSpec = KSystemTimeZones::local();
    KDateTime start = mDtStart;
    KDateTime end = mDtEnd;
    bool success = mResolver->findFreeSlot( KCal::Period( start, end ) );

    if ( success ) {
        if ( start == mDtStart && end == mDtEnd ) {
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
                emit dateTimesChanged( start, end );
                slotUpdateGanttView( start, end );
            }
        }
    } else {
        KMessageBox::sorry( this, i18nc( "@info", "No suitable date found." ) );
    }
}

void VisualFreeBusyWidget::slotScaleChanged( int newScale )
{
    const QVariant var = mScaleCombo->itemData( newScale );
    Q_ASSERT( var.isValid() );

    int value = var.value<int>();

    mGanttGrid->setScale(( KDGantt::DateTimeGrid::Scale ) value );
}

void VisualFreeBusyWidget::slotUpdateGanttView( const KDateTime& dtFrom, const KDateTime& dtTo )
{
    mDtStart = dtFrom;
    mDtEnd = dtTo;
    QDateTime horizonStart = QDateTime( dtFrom.addDays( -15 ).date() );
    KDGantt::DateTimeGrid *grid = static_cast<KDGantt::DateTimeGrid*>( mGanttView->grid() );
    grid->setStartDateTime( horizonStart );
    slotCenterOnStart();
    mGanttGrid->setStartDateTime( horizonStart );
}

void VisualFreeBusyWidget::slotZoomToTime()
{
#if 0
    mGanttView->zoomToFit();
#else
    kDebug() << "Disabled code, port to KDGantt2";
#endif
}

void VisualFreeBusyWidget::splitterMoved()
{
    mLeftView->setColumnWidth( 0, mLeftView->width() );
}

void VisualFreeBusyWidget::newFreeBusy( const FreeBusyItem::Ptr & item )
{
  KCal::FreeBusy* fb = item->freeBusy();
  if ( fb ) {
    kDebug() << "fb " << fb;
    KDateTime::Spec timeSpec = KSystemTimeZones::local();

      QList<QStandardItem *> newItems;
      // Evaluate free/busy information
      QList<KCal::FreeBusyPeriod> busyPeriods = fb->fullBusyPeriods();
      for ( QList<KCal::FreeBusyPeriod>::Iterator it = busyPeriods.begin();
            it != busyPeriods.end(); ++it ) {
        KCal::FreeBusyPeriod per = *it;

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
      mModel->appendRow( newItems );
  }
}


#include "visualfreebusywidget.moc"
