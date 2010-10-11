/*
  This file is part of KOrganizer.

  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2010 Sérgio Martins <iamsergio@gmail.com>

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

#include "listview.h"
#include "helper.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/incidencechanger.h>

#include <KCalUtils/IncidenceFormatter>
#include <KCalCore/Todo>
#include <KCalCore/Visitor>
#include <KCalCore/Journal>

#include <KIconLoader>

#include <QTreeWidget>
#include <QBoxLayout>
#include <QStyle>
#include <QHeaderView>

using namespace EventViews;
using namespace KCalUtils;
using namespace KCalCore;

class ListView::Private
{
  public:
  Private( ListView *mListView ): q( mListView )
  {
  }

  ~Private()
  {
  }
  void addIncidences( const Akonadi::Item::List &incidenceList, const QDate & );
  void addIncidence( const Akonadi::Item &, const QDate &date );
  ListViewItem *getItemForIncidence( const Akonadi::Item & );

  QTreeWidget *mTreeWidget;
  ListViewItem *mActiveItem;
  QHash<Akonadi::Item::Id,Akonadi::Item> mItems;
  QHash<Akonadi::Item::Id, QDate> mDateList;
  QDate mStartDate;
  QDate mEndDate;
  DateList mSelectedDates;

  // if it's non interactive we disable context menu, and incidence editing
  bool mIsNonInteractive;
  class ListItemVisitor;

  private:
    ListView *const q;
};


enum {
  Summary_Column = 0,
  Reminder_Column,
  Recurs_Column,
  StartDateTime_Column,
  EndDateTime_Column,
  Categories_Column,
  Dummy_EOF_Column // Dummy enum value for iteration purposes only. Always keep at the end.
};

/**
  This class provides the initialization of a KOListViewItem for calendar
  components using the Incidence::Visitor.
*/
class ListView::Private::ListItemVisitor : public KCalCore::Visitor
{
  public:
    ListItemVisitor( ListViewItem *item ) : mItem( item ) {}
    ~ListItemVisitor() {}

    bool visit( Event::Ptr  );
    bool visit( Todo::Ptr  );
    bool visit( Journal::Ptr  );
    bool visit( FreeBusy::Ptr  ) { // to inhibit hidden virtual compile warning
      return true;
    };
  private:
    ListViewItem *mItem;
};

bool ListView::Private::ListItemVisitor::visit( Event::Ptr e )
{
  mItem->setText( Summary_Column, e->summary() );
  if ( e->hasEnabledAlarms() ) {
    static const QPixmap alarmPxmp = SmallIcon( "appointment-reminder" );
    mItem->setIcon( Reminder_Column, alarmPxmp );
    mItem->setSortKey( Reminder_Column, "1" );
  } else {
    mItem->setSortKey( Reminder_Column, "0" );
  }

  if ( e->recurs() ) {
    static const QPixmap recurPxmp = SmallIcon( "appointment-recurring" );
    mItem->setIcon( Recurs_Column, recurPxmp );
    mItem->setSortKey( Recurs_Column, "1" );
  } else {
    mItem->setSortKey( Recurs_Column, "0" );
  }

  QPixmap eventPxmp;

  if ( e->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
    eventPxmp = SmallIcon( "view-calendar-wedding-anniversary" );
  } else if ( e->customProperty( "KABC", "BIRTHDAY" ) == "YES" ) {
    eventPxmp = SmallIcon( "view-calendar-birthday" );
  } else {
    eventPxmp = SmallIcon( "view-calendar-day" );
  }

  mItem->setIcon( Summary_Column, eventPxmp );

  mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString(
                    e->dtStart(), e->allDay(), true, CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

  mItem->setSortKey( StartDateTime_Column, e->dtStart().toTimeSpec(
                       CalendarSupport::KCalPrefs::instance()->timeSpec() ).toString( KDateTime::ISODate ) );

  mItem->setText( EndDateTime_Column, IncidenceFormatter::dateTimeToString(
                    e->dtEnd(), e->allDay(), true, CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

  mItem->setSortKey( EndDateTime_Column, e->dtEnd().toTimeSpec(
                       CalendarSupport::KCalPrefs::instance()->timeSpec() ).toString( KDateTime::ISODate ) );

  mItem->setText( Categories_Column, e->categoriesStr() );

  return true;
}

bool ListView::Private::ListItemVisitor::visit( Todo::Ptr t )
{
  static const QPixmap todoPxmp = SmallIcon( "view-calendar-tasks" );
  static const QPixmap todoDonePxmp = SmallIcon( "task-complete" );

  mItem->setIcon( Summary_Column, t->isCompleted() ? todoDonePxmp : todoPxmp );
  mItem->setText( Summary_Column, t->summary() );
  if ( t->hasEnabledAlarms() ) {
    static const QPixmap alarmPxmp = SmallIcon( "appointment-reminder" );
    mItem->setIcon( Reminder_Column, alarmPxmp );
    mItem->setSortKey( Reminder_Column, "1" );
  } else {
    mItem->setSortKey( Reminder_Column, "0" );
  }

  if ( t->recurs() ) {
    static const QPixmap recurPxmp = SmallIcon( "appointment-recurring" );
    mItem->setIcon( Recurs_Column, recurPxmp );
    mItem->setSortKey( Recurs_Column, "1" );
  } else {
    mItem->setSortKey( Recurs_Column, "0" );
  }

  if ( t->hasStartDate() ) {
    mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString(
                      t->dtStart(), t->allDay(), true, CalendarSupport::KCalPrefs::instance()->timeSpec() ) );
    mItem->setSortKey( StartDateTime_Column, t->dtStart().toTimeSpec(
                       CalendarSupport::KCalPrefs::instance()->timeSpec() ).toString( KDateTime::ISODate ) );
  } else {
    mItem->setText( StartDateTime_Column, "---" );
  }

  if ( t->hasDueDate() ) {
    mItem->setText( EndDateTime_Column, IncidenceFormatter::dateTimeToString(
                      t->dtDue(), t->allDay(), true, CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

    mItem->setSortKey( EndDateTime_Column, t->dtDue().toTimeSpec(
                         CalendarSupport::KCalPrefs::instance()->timeSpec() ).toString( KDateTime::ISODate ) );
  } else {
    mItem->setText( EndDateTime_Column, "---" );
  }
  mItem->setText( Categories_Column, t->categoriesStr() );

  return true;
}

bool ListView::Private::ListItemVisitor::visit( Journal::Ptr j )
{
  static const QPixmap jrnalPxmp = SmallIcon( "view-pim-journal" );
  mItem->setIcon( Summary_Column, jrnalPxmp );
  if ( j->summary().isEmpty() ) {
    mItem->setText( Summary_Column, j->description().section( '\n', 0, 0 ) );
  } else {
    mItem->setText( Summary_Column, j->summary() );
  }
  mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString(
                  j->dtStart(), j->allDay(), true, CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

  mItem->setSortKey( StartDateTime_Column, j->dtStart().toString( KDateTime::ISODate ) );

  return true;
}

ListView::ListView( CalendarSupport::Calendar *calendar,
                    QWidget *parent, bool nonInteractive )
  : EventView( parent ), d( new Private( this ) )
{
  setCalendar( calendar );
  d->mActiveItem = 0;
  d->mIsNonInteractive = nonInteractive;

  d->mTreeWidget = new QTreeWidget( this );
  d->mTreeWidget->setColumnCount( 6 );
  d->mTreeWidget->headerItem()->setText( 0, i18n( "Summary" ) );
  d->mTreeWidget->headerItem()->setText( 1, i18n( "Reminder" ) );
  d->mTreeWidget->headerItem()->setText( 2, i18n( "Recurs" ) );
  d->mTreeWidget->headerItem()->setText( 3, i18n( "Start Date/Time" ) );
  d->mTreeWidget->headerItem()->setText( 4, i18n( "End Date/Time" ) );
  d->mTreeWidget->headerItem()->setText( 5, i18n( "Categories" ) );

  d->mTreeWidget->setContextMenuPolicy( Qt::CustomContextMenu );

  // d->mTreeWidget->setColumnAlignment( StartDateTime_Column, Qt::AlignHCenter );

  //d->mTreeWidget->setColumnAlignment( EndDateTime_Column, Qt::AlignHCenter );

  QBoxLayout *layoutTop = new QVBoxLayout( this );
  layoutTop->setMargin( 0 );
  layoutTop->addWidget( d->mTreeWidget );

  QObject::connect( d->mTreeWidget, SIGNAL(doubleClicked(QModelIndex)),
                    SLOT(defaultItemAction(QModelIndex)) );
  QObject::connect( d->mTreeWidget,
                    SIGNAL(customContextMenuRequested(QPoint)),
                    SLOT(popupMenu(QPoint)) );
  QObject::connect( d->mTreeWidget, SIGNAL(itemSelectionChanged()),
                    SLOT(processSelectionChange()) );
  // TODO
  //d->mTreeWidget->restoreLayout( KOGlobals::self()->config(), "ListView Layout" );

  d->mSelectedDates.append( QDate::currentDate() );
}

ListView::~ListView()
{
  delete d;
}

int ListView::currentDateCount() const
{
  return d->mSelectedDates.count();
}

Akonadi::Item::List ListView::selectedIncidences() const
{
  Akonadi::Item::List eventList;
  QTreeWidgetItem *item = d->mTreeWidget->selectedItems().isEmpty() ? 0 :
                          d->mTreeWidget->selectedItems().first() ;
  if ( item ) {
    ListViewItem *i = static_cast<ListViewItem *>( item );
    eventList.append( d->mItems.value( i->data() ) );
  }
  return eventList;
}

DateList ListView::selectedIncidenceDates() const
{
  return d->mSelectedDates;
}

void ListView::showDates( bool show )
{
  // Shouldn't we set it to a value greater 0? When showDates is called with
  // show == true at first, then the columnwidths are set to zero.
  static int oldColWidth1 = 0;
  static int oldColWidth3 = 0;

  if ( !show ) {
    oldColWidth1 = d->mTreeWidget->columnWidth( 1 );
    oldColWidth3 = d->mTreeWidget->columnWidth( 3 );
    d->mTreeWidget->setColumnWidth( 1, 0 );
    d->mTreeWidget->setColumnWidth( 3, 0 );
  } else {
    d->mTreeWidget->setColumnWidth( 1, oldColWidth1 );
    d->mTreeWidget->setColumnWidth( 3, oldColWidth3 );
  }
  d->mTreeWidget->repaint();
}

void ListView::showDates()
{
  showDates( true );
}

void ListView::hideDates()
{
  showDates( false );
}

void ListView::updateView()
{
  kDebug() << "not implemented yet";
}

void ListView::showDates( const QDate &start, const QDate &end )
{
  clear();

  d->mStartDate = start;
  d->mEndDate = end;

  QDate date = start;
  while ( date <= end ) {
    d->addIncidences( calendar()->incidences( date ), date );
    d->mSelectedDates.append( date );
    date = date.addDays( 1 );
  }

  emit incidenceSelected( Akonadi::Item(), QDate() );
}

void ListView::showAll()
{
  const Akonadi::Item::List incidenceList = calendar()->incidences();
  d->addIncidences( incidenceList, QDate() );
}

void ListView::Private::addIncidences( const Akonadi::Item::List &incidenceList,
                                       const QDate &date )
{
  Q_FOREACH ( const Akonadi::Item & i, incidenceList ) {
    addIncidence( i, date );
  }
}

void ListView::Private::addIncidence( const Akonadi::Item &aitem, const QDate &date )
{
  if ( !CalendarSupport::hasIncidence( aitem ) || mItems.contains( aitem.id() ) ) {
    return;
  }

  mDateList.insert( aitem.id(), date );
  mItems.insert( aitem.id(), aitem );

  Incidence::Ptr tinc = CalendarSupport::incidence( aitem );

  if ( tinc->customProperty( "KABC", "BIRTHDAY" ) == "YES" ||
       tinc->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
    const int years = EventViews::yearDiff( tinc->dtStart().date(), mEndDate );
    if ( years > 0 ) {
      tinc = Incidence::Ptr( tinc->clone() );
      tinc->setReadOnly( false );
      tinc->setSummary( i18np( "%2 (1 year)", "%2 (%1 years)", years, tinc->summary() ) );
      tinc->setReadOnly( true );
    }
  }
  ListViewItem *item = new ListViewItem( aitem.id(), mTreeWidget, q );

  // set tooltips
  for ( int col = 0; col < Dummy_EOF_Column; ++col ) {
    item->setToolTip( col, IncidenceFormatter::toolTipStr( CalendarSupport::displayName( aitem.parentCollection() ),
                                                           CalendarSupport::incidence( aitem ) ) );
  }

  ListItemVisitor v( item );
  if ( !tinc->accept( v, tinc ) ) {
    delete item;
  }

  item->setData( 0, Qt::UserRole, QVariant( aitem.id() ) );
}

void ListView::showIncidences( const Akonadi::Item::List &incidenceList,
                               const QDate &date )
{
  clear();

 d->addIncidences( incidenceList, date );

  // After new creation of list view no events are selected.
  emit incidenceSelected( Akonadi::Item(), date );
}

void ListView::changeIncidenceDisplay( const Akonadi::Item & aitem, int action )
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  ListViewItem *item;
  QDate f = d->mSelectedDates.first();
  QDate l = d->mSelectedDates.last();

  QDate date;
  if ( CalendarSupport::hasTodo( aitem ) ) {
    date = CalendarSupport::todo( aitem )->dtDue().
           toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date();
  } else {
    date = incidence->dtStart().
           toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date();
  }

  switch( action ) {
  case CalendarSupport::IncidenceChanger::INCIDENCEADDED:
  {
    if ( date >= f && date <= l ) {
      d->addIncidence( aitem, date );
    }
    break;
  }
  case CalendarSupport::IncidenceChanger::INCIDENCEEDITED:
  {
    item = d->getItemForIncidence( aitem );
    if ( item ) {
      delete item;
      d->mItems.remove( aitem.id() );
      d->mDateList.remove( aitem.id() );
    }
    if ( date >= f && date <= l ) {
      d->addIncidence( aitem, date );
    }
    break;
  }
  case CalendarSupport::IncidenceChanger::INCIDENCEDELETED:
  {
    item = d->getItemForIncidence( aitem );
    if ( item ) {
      delete item;
    }
    break;
  }
  default:
    kDebug() << "Illegal action" << action;
  }
}

ListViewItem *ListView::Private::getItemForIncidence( const Akonadi::Item &aitem )
{
  int index = 0;
  while ( QTreeWidgetItem *it = mTreeWidget->topLevelItem( index ) ) {
    ListViewItem *item = static_cast<ListViewItem *>( it );
    if ( item->data() == aitem.id() ) {
      return item;
    }
    ++index;
  }

  return 0;
}

void ListView::defaultItemAction( const QModelIndex &index )
{
  if ( !d->mIsNonInteractive ) {
    // Get the first column, it has our Akonadi::Id
    const QModelIndex col0Idx = d->mTreeWidget->model()->index( index.row(), 0 );
    Akonadi::Item::Id id = d->mTreeWidget->model()->data( col0Idx, Qt::UserRole ).toLongLong();
    defaultAction( d->mItems.value( id ) );
  }
}

void ListView::defaultItemAction( const Akonadi::Item::Id id )
{
  if ( !d->mIsNonInteractive ) {
    defaultAction( d->mItems.value( id ) );
  }
}

void ListView::popupMenu( const QPoint &point )
{
  d->mActiveItem = static_cast<ListViewItem *>( d->mTreeWidget->itemAt( point ) );

  if ( d->mActiveItem && !d->mIsNonInteractive ) {
    const Akonadi::Item aitem = d->mItems.value( d->mActiveItem->data() );
    // FIXME: For recurring incidences we don't know the date of this
    // occurrence, there's no reference to it at all!

    emit showIncidencePopupSignal( aitem,
                                   CalendarSupport::incidence( aitem )->dtStart().date() );
  } else {
    emit showNewEventPopupSignal();
  }
}

void ListView::readSettings( KConfig *config )
{
  KConfigGroup cfgGroup = config->group( "ListView Layout" );
  const QByteArray state = cfgGroup.readEntry( "ViewState", QByteArray() );
  d->mTreeWidget->header()->restoreState( state );
}

void ListView::writeSettings( KConfig *config )
{
  const QByteArray state = d->mTreeWidget->header()->saveState();
  KConfigGroup cfgGroup = config->group( "ListView Layout" );

  cfgGroup.writeEntry( "ViewState", state );
}

void ListView::processSelectionChange()
{
  if ( !d->mIsNonInteractive ) {
    ListViewItem *item;
    if ( d->mTreeWidget->selectedItems().isEmpty() ) {
      item = 0;
    } else {
      item = static_cast<ListViewItem *>( d->mTreeWidget->selectedItems().first() );
    }

    if ( !item ) {
      emit incidenceSelected( Akonadi::Item(), QDate() );
    } else {
      emit incidenceSelected( d->mItems.value( item->data() ), d->mDateList.value( item->data() ) );
    }
  }
}

void ListView::clearSelection()
{
  d->mTreeWidget->clearSelection();
}

void ListView::clear()
{
  d->mSelectedDates.clear();
  d->mTreeWidget->clear();
  d->mDateList.clear();
  d->mItems.clear();
}

QSize ListView::sizeHint() const
{
  const QSize s = EventView::sizeHint();
  return QSize( s.width() + style()->pixelMetric( QStyle::PM_ScrollBarExtent ) + 1,
                s.height() );
}

#include "listview.moc"
