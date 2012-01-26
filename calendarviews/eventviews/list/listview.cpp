/*
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2010 Sérgio Martins <iamsergio@gmail.com>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

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

//TODO: put a reminder and/or recurs icon on the item?
// remove customlistitem.h

#include "listview.h"
#include "helper.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/incidencechanger.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalUtils/IncidenceFormatter>
using namespace KCalUtils;

#include <KCalCore/Visitor>
using namespace KCalCore;

#include <KIconLoader>

#include <QBoxLayout>
#include <QHeaderView>
#include <QTreeWidget>

using namespace EventViews;

enum {
  Summary_Column = 0,
  StartDateTime_Column,
  EndDateTime_Column,
  Categories_Column,
  Dummy_EOF_Column // Dummy enum value for iteration purposes only. Always keep at the end.
};

static QString cleanSummary( const QString &summary, const KDateTime &next )
{
  static QString etc = i18nc( "@label an elipsis", "..." );
  int maxLen = 40;

  QString retStr = summary;
  retStr.replace( '\n', ' ' );
  if ( retStr.length() > maxLen ) {
    maxLen -= etc.length();
    retStr = retStr.left( maxLen );
    retStr += etc;
  }
  if ( next.isValid() ) {
    const QString dateStr =
      KGlobal::locale()->formatDate(
        next.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date(),
        KLocale::ShortDate );
    retStr = i18nc( "%1 is an item summary. %2 is the date when this item reoccurs",
                    "%1 (next: %2)", retStr, dateStr );
  }
  return retStr;
}

class ListViewItem : public QTreeWidgetItem
{
  public:
    ListViewItem( const Akonadi::Item &incidence, QTreeWidget *parent )
      : QTreeWidgetItem( parent ), mTreeWidget( parent ), mIncidence( incidence )
    {
    }

    bool operator<( const QTreeWidgetItem & other ) const;

    const QTreeWidget *mTreeWidget;
    const Akonadi::Item mIncidence;
    KDateTime start;
    KDateTime end;
};

bool ListViewItem::operator<( const QTreeWidgetItem &other ) const
{
  const ListViewItem *otheritem = static_cast<const ListViewItem *>( &other );

  switch( treeWidget()->sortColumn() ) {

  case StartDateTime_Column:
  {
    return otheritem->start < start;
  }
  case EndDateTime_Column:
  {
    KDateTime thisEnd;
    Incidence::Ptr thisInc = CalendarSupport::incidence( mIncidence );
    thisEnd = thisInc->dateTime( Incidence::RoleDisplayEnd );

    KDateTime otherEnd;
    Incidence::Ptr otherInc = CalendarSupport::incidence( otheritem->mIncidence );
    otherEnd = otherInc->dateTime( Incidence::RoleDisplayEnd );

    return otherEnd < thisEnd;
  }
  default:
    return QTreeWidgetItem::operator < ( other );
  }
}

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

/**
  This class provides the initialization of a ListViewItem for calendar
  components using the Incidence::Visitor.
*/
class ListView::Private::ListItemVisitor : public KCalCore::Visitor
{
  public:
    ListItemVisitor( ListViewItem *item, QDate dt ) : mItem( item ), mStartDate( dt )
    {
    }
    ~ListItemVisitor()
    {
    }

    bool visit( Event::Ptr );
    bool visit( Todo::Ptr );
    bool visit( Journal::Ptr );
    bool visit( FreeBusy::Ptr )
    { // to inhibit hidden virtual compile warning
      return true;
    };

  private:
    ListViewItem *mItem;
    QDate mStartDate;
};

bool ListView::Private::ListItemVisitor::visit( Event::Ptr e )
{
  QPixmap eventPxmp;
  if ( e->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
    eventPxmp = cachedSmallIcon( "view-calendar-wedding-anniversary" );
  } else if ( e->customProperty( "KABC", "BIRTHDAY" ) == "YES" ) {
    eventPxmp = cachedSmallIcon( "view-calendar-birthday" );
  } else {
    eventPxmp = cachedSmallIcon( e->iconName() );
  }
  mItem->setIcon( Summary_Column, eventPxmp );

  KDateTime next;
  mItem->start = e->dateTime( Incidence::RoleDisplayStart );
  mItem->end = e->dateTime( Incidence::RoleDisplayEnd );
  if ( e->recurs() ) {
    const int duration = e->dtStart().secsTo( e->dtEnd() );
    KDateTime kdt = KDateTime::currentDateTime(
      CalendarSupport::KCalPrefs::instance()->timeSpec() );
    kdt = kdt.addSecs( -1 );
    mItem->start.setDate( e->recurrence()->getNextDateTime( kdt ).date() );
    mItem->end = mItem->start.addSecs( duration );
    next = e->recurrence()->getNextDateTime( mItem->start );
  }

  mItem->setText( Summary_Column, cleanSummary( e->summary(), next ) );

  mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString(
                    mItem->start, e->allDay(), true,
                    CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

  mItem->setText( EndDateTime_Column, IncidenceFormatter::dateTimeToString(
                    mItem->end, e->allDay(), true,
                    CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

  mItem->setText( Categories_Column, e->categoriesStr() );

  return true;
}

bool ListView::Private::ListItemVisitor::visit( Todo::Ptr t )
{
  mItem->setIcon( Summary_Column, cachedSmallIcon( t->iconName() ) );

  mItem->setText( Summary_Column, cleanSummary( t->summary(), KDateTime() ) );

  if ( t->hasStartDate() ) {
    mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString(
                      t->dateTime( Incidence::RoleDisplayStart ), t->allDay(), true,
                      CalendarSupport::KCalPrefs::instance()->timeSpec() ) );
  } else {
    mItem->setText( StartDateTime_Column, "---" );
  }

  if ( t->hasDueDate() ) {
    mItem->setText( EndDateTime_Column, IncidenceFormatter::dateTimeToString(
                      t->dateTime( Incidence::RoleDisplayEnd ), t->allDay(), true,
                      CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

  } else {
    mItem->setText( EndDateTime_Column, "---" );
  }
  mItem->setText( Categories_Column, t->categoriesStr() );

  return true;
}

bool ListView::Private::ListItemVisitor::visit( Journal::Ptr j )
{
  static const QPixmap jrnalPxmp = SmallIcon( j->iconName() );
  mItem->setIcon( Summary_Column, jrnalPxmp );
  if ( j->summary().isEmpty() ) {
    mItem->setText( Summary_Column,
                    cleanSummary( j->description().section( '\n', 0, 0 ),
                                  KDateTime() ) );
  } else {
    mItem->setText( Summary_Column, cleanSummary( j->summary(), KDateTime() ) );
  }
  mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString(
                    j->dateTime( Incidence::RoleDisplayStart ), j->allDay(), true,
                    CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

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
  d->mTreeWidget->setColumnCount( 4 );
  d->mTreeWidget->setSortingEnabled( true );
  d->mTreeWidget->headerItem()->setText( Summary_Column, i18n( "Summary" ) );
  d->mTreeWidget->headerItem()->setText( StartDateTime_Column, i18n( "Start Date/Time" ) );
  d->mTreeWidget->headerItem()->setText( EndDateTime_Column, i18n( "End Date/Time" ) );
  d->mTreeWidget->headerItem()->setText( Categories_Column, i18n( "Categories" ) );

  d->mTreeWidget->setWordWrap( true );
  d->mTreeWidget->setAllColumnsShowFocus( true );
  d->mTreeWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  d->mTreeWidget->setRootIsDecorated( false );

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

  updateView();
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
    eventList.append( i->mIncidence );
  }
  return eventList;
}

DateList ListView::selectedIncidenceDates() const
{
  return d->mSelectedDates;
}

void ListView::updateView()
{
  for ( int col = 0; col < Dummy_EOF_Column; ++col ) {
    d->mTreeWidget->resizeColumnToContents( col );
  }
  d->mTreeWidget->sortItems( StartDateTime_Column, Qt::DescendingOrder );
}

void ListView::showDates( const QDate &start, const QDate &end, const QDate &preferredMonth )
{
  Q_UNUSED( preferredMonth );
  clear();

  d->mStartDate = start;
  d->mEndDate = end;

  KDateTime kStart( start );
  const QString startStr =
    KGlobal::locale()->formatDate(
      kStart.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date(),
      KLocale::ShortDate );

  KDateTime kEnd( end );
  const QString endStr =
    KGlobal::locale()->formatDate(
      kEnd.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date(),
      KLocale::ShortDate );

  d->mTreeWidget->headerItem()->setText( Summary_Column,
                                         i18n( "Summary [%1 - %2]", startStr, endStr ) );

  QDate date = start;
  while ( date <= end ) {
    d->addIncidences( calendar()->incidences( date ), date );
    d->mSelectedDates.append( date );
    date = date.addDays( 1 );
  }

  updateView();

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
      tinc->setSummary( i18np( "%2 (1 year)", "%2 (%1 years)", years,
                               cleanSummary( tinc->summary(), KDateTime() ) ) );
      tinc->setReadOnly( true );
    }
  }
  ListViewItem *item = new ListViewItem( aitem, mTreeWidget );

  // set tooltips
  for ( int col = 0; col < Dummy_EOF_Column; ++col ) {
    item->setToolTip( col,
                      IncidenceFormatter::toolTipStr(
                        CalendarSupport::displayName( aitem.parentCollection() ),
                        CalendarSupport::incidence( aitem ) ) );
  }

  ListItemVisitor v( item, mStartDate );
  if ( !tinc->accept( v, tinc ) ) {
    delete item;
  }

  item->setData( 0, Qt::UserRole, QVariant( aitem.id() ) );
}

void ListView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  clear();

  d->addIncidences( incidenceList, date );

  updateView();

  // After new creation of list view no events are selected.
  emit incidenceSelected( Akonadi::Item(), date );
}

void ListView::changeIncidenceDisplay( const Akonadi::Item &aitem, int action )
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
    delete item;
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
    if ( item->mIncidence.id() == aitem.id() ) {
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
    const Akonadi::Item aitem = d->mActiveItem->mIncidence;
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
      emit incidenceSelected( item->mIncidence, d->mDateList.value( item->mIncidence.id() ) );
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
