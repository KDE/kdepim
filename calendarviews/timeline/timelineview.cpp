/*
  Copyright (c) 2007 Till Adam <adam@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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

#include "timelineview.h"
#include "timelineview_p.h"
#include "timelineitem.h"
#include "helper.h"

#include <kdgantt2/kdganttgraphicsitem.h>
#include <kdgantt2/kdganttgraphicsview.h>
#include <kdgantt2/kdganttabstractrowcontroller.h>
#include <kdgantt2/kdganttdatetimegrid.h>
#include <kdgantt2/kdganttitemdelegate.h>
#include <kdgantt2/kdganttstyleoptionganttitem.h>

#include <Akonadi/Calendar/ETMCalendar>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/kcalprefs.h>
#include <Akonadi/Calendar/IncidenceChanger>

#include <QDebug>

#include <QApplication>
#include <QPainter>
#include <QStandardItemModel>
#include <QSplitter>
#include <QTreeWidget>
#include <QHeaderView>
#include <QPointer>
#include <QVBoxLayout>
#include <QHelpEvent>

using namespace KCalCore;
using namespace EventViews;

namespace EventViews {

class RowController : public KDGantt::AbstractRowController
{
  private:
    static const int ROW_HEIGHT ;
    QPointer<QAbstractItemModel> m_model;

  public:
    RowController()
    {
      mRowHeight = 20;
    }

    void setModel( QAbstractItemModel *model )
    {
      m_model = model;
    }

    /*reimp*/int headerHeight() const
    {
      return 2 * mRowHeight + 10;
    }

    /*reimp*/bool isRowVisible( const QModelIndex & ) const
    {
      return true;
    }

    /*reimp*/bool isRowExpanded( const QModelIndex & ) const
    {
      return false;
    }

    /*reimp*/KDGantt::Span rowGeometry( const QModelIndex &idx ) const
    {
        return KDGantt::Span( idx.row() * mRowHeight, mRowHeight );
    }

    /*reimp*/int maximumItemHeight() const
    {
      return mRowHeight / 2;
    }

    /*reimp*/int totalHeight() const
    {
      return m_model->rowCount() * mRowHeight;
    }

    /*reimp*/QModelIndex indexAt( int height ) const
    {
      return m_model->index( height / mRowHeight, 0 );
    }

    /*reimp*/QModelIndex indexBelow( const QModelIndex &idx ) const
    {
      if ( !idx.isValid() ) {
        return QModelIndex();
      }
      return idx.model()->index( idx.row() + 1, idx.column(), idx.parent() );
    }

    /*reimp*/QModelIndex indexAbove( const QModelIndex &idx ) const
    {
      if ( !idx.isValid() ) {
        return QModelIndex();
      }
      return idx.model()->index( idx.row() - 1, idx.column(), idx.parent() );
    }

    void setRowHeight( int height )
    {
      mRowHeight = height;
    }

  private:
    int mRowHeight;
};

class GanttHeaderView : public QHeaderView
{
  public:
    explicit GanttHeaderView( QWidget *parent=0 ) : QHeaderView( Qt::Horizontal, parent )
    {
    }

    QSize sizeHint() const
    {
      QSize s = QHeaderView::sizeHint();
      s.rheight() *= 2;
      return s;
    }
};

class GanttItemDelegate : public KDGantt::ItemDelegate
{
  void paintGanttItem( QPainter *painter,
                       const KDGantt::StyleOptionGanttItem &opt,
                       const QModelIndex &idx )
  {
    painter->setRenderHints( QPainter::Antialiasing );
    if ( !idx.isValid() ) {
      return;
    }

    KDGantt::ItemType type = static_cast<KDGantt::ItemType>(
      idx.model()->data( idx, KDGantt::ItemTypeRole ).toInt() );

    QString txt = idx.model()->data( idx, Qt::DisplayRole ).toString();
    QRectF itemRect = opt.itemRect;
    QRectF boundingRect = opt.boundingRect;
    boundingRect.setY( itemRect.y() );
    boundingRect.setHeight( itemRect.height() );

    QBrush brush = defaultBrush( type );
    if ( opt.state & QStyle::State_Selected ) {
      QLinearGradient selectedGrad( 0., 0., 0.,
                                    QApplication::fontMetrics().height() );
      selectedGrad.setColorAt( 0., Qt::red );
      selectedGrad.setColorAt( 1., Qt::darkRed );

      brush = QBrush( selectedGrad );
      painter->setBrush( brush );
    } else {
      painter->setBrush( idx.model()->data( idx, Qt::DecorationRole ).value<QColor>() );
    }

    painter->setPen( defaultPen( type ) );
    painter->setBrushOrigin( itemRect.topLeft() );

    switch( type ) {
    case KDGantt::TypeTask:
      if ( itemRect.isValid() ) {
        QRectF r = itemRect;
        painter->drawRect( r );

        Qt::Alignment ta;
        switch( opt.displayPosition ) {
        case KDGantt::StyleOptionGanttItem::Left:
          ta = Qt::AlignLeft;
          break;
        case KDGantt::StyleOptionGanttItem::Right:
          ta = Qt::AlignRight;
          break;
        case KDGantt::StyleOptionGanttItem::Center:
          ta = Qt::AlignCenter;
          break;
        }
        painter->drawText( boundingRect, ta, txt );
      }
      break;
    default:
      KDGantt::ItemDelegate::paintGanttItem( painter, opt, idx );
      break;
    }
  }
};

}

TimelineView::TimelineView( QWidget *parent )
  : EventView( parent ), d( new Private( this ) )
{
  QVBoxLayout *vbox = new QVBoxLayout( this );
  vbox->setMargin( 0 );
  QSplitter *splitter = new QSplitter( Qt::Horizontal, this );
  d->mLeftView = new QTreeWidget;
  d->mLeftView->setHeader( new GanttHeaderView );
  d->mLeftView->setHeaderLabel( i18n( "Calendar" ) );
  d->mLeftView->setRootIsDecorated( false );
  d->mLeftView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  d->mGantt = new KDGantt::GraphicsView();
  splitter->addWidget( d->mLeftView );
  splitter->addWidget( d->mGantt );
  connect( splitter, SIGNAL(splitterMoved(int,int)),
           d, SLOT(splitterMoved()) );
  QStandardItemModel *model = new QStandardItemModel( this );

  d->mRowController = new RowController;
  d->mRowController->setRowHeight( fontMetrics().height() ); //TODO: detect

  d->mRowController->setModel( model );
  d->mGantt->setRowController( d->mRowController );

  KDGantt::DateTimeGrid *grid = new KDGantt::DateTimeGrid;
  grid->setScale( KDGantt::DateTimeGrid::ScaleHour );
  grid->setDayWidth( 800 );
  grid->setRowSeparators( true );
  d->mGantt->setGrid( grid );
  d->mGantt->setModel( model );
  d->mGantt->viewport()->setFixedWidth( 8000 );

  d->mGantt->viewport()->installEventFilter( this );

#if 0
  d->mGantt->setCalendarMode( true );
  d->mGantt->setShowLegendButton( false );
  d->mGantt->setFixedHorizon( true );
  d->mGantt->removeColumn( 0 );
  d->mGantt->addColumn( i18n( "Calendar" ) );
  d->mGantt->setHeaderVisible( true );
  if ( KGlobal::locale()->use12Clock() ) {
    d->mGantt->setHourFormat( KDGanttView::Hour_12 );
  } else {
    d->mGantt->setHourFormat( KDGanttView::Hour_24_FourDigit );
  }
#else
  qDebug() << "Disabled code, port to KDGantt2";
#endif
  d->mGantt->setItemDelegate( new GanttItemDelegate );

  vbox->addWidget( splitter );

#if 0
  connect( d->mGantt, SIGNAL(rescaling(KDGanttView::Scale)),
           SLOT(overscale(KDGanttView::Scale)) );
#else
  qDebug() << "Disabled code, port to KDGantt2";
#endif
  connect( model, SIGNAL(itemChanged(QStandardItem*)),
           d, SLOT(itemChanged(QStandardItem*)) );
  connect( d->mGantt, SIGNAL(doubleClicked(QModelIndex)),
           d, SLOT(itemDoubleClicked(QModelIndex)) );
  connect( d->mGantt, SIGNAL(activated(QModelIndex)),
           d, SLOT(itemSelected(QModelIndex)) );
  d->mGantt->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( d->mGantt, SIGNAL(customContextMenuRequested(QPoint)),
           d, SLOT(contextMenuRequested(QPoint)) );

#if 0
  connect( d->mGantt, SIGNAL(dateTimeDoubleClicked(QDateTime)),
           d, SLOT(newEventWithHint(QDateTime)) );
#else
  qDebug() << "Disabled code, port to KDGantt2";
#endif
}

TimelineView::~TimelineView()
{
  delete d->mRowController;
  delete d;
}

/*virtual*/
Akonadi::Item::List TimelineView::selectedIncidences() const
{
  return d->mSelectedItemList;
}

/*virtual*/
KCalCore::DateList TimelineView::selectedIncidenceDates() const
{
  return KCalCore::DateList();
}

/*virtual*/
int TimelineView::currentDateCount() const
{
  return 0;
}

/*virtual*/
void TimelineView::showDates( const QDate &start, const QDate &end, const QDate &preferredMonth )
{
  Q_UNUSED( preferredMonth );
  Q_ASSERT_X( calendar(), "showDates()", "set a Akonadi::ETMCalendar" );
  Q_ASSERT_X( start.isValid(), "showDates()", "start date must be valid" );
  Q_ASSERT_X( end.isValid(), "showDates()", "end date must be valid" );

  qDebug() << "start=" << start << "end=" << end;

  d->mStartDate = start;
  d->mEndDate = end;
  d->mHintDate = QDateTime();
  KDGantt::DateTimeGrid *grid = static_cast<KDGantt::DateTimeGrid*>( d->mGantt->grid() );
  grid->setStartDateTime( QDateTime( start ) );
#if 0
  d->mGantt->setHorizonStart( QDateTime( start ) );
  d->mGantt->setHorizonEnd( QDateTime( end.addDays( 1 ) ) );
  d->mGantt->setMinorScaleCount( 1 );
  d->mGantt->setScale( KDGanttView::Hour );
  d->mGantt->setMinimumScale( KDGanttView::Hour );
  d->mGantt->setMaximumScale( KDGanttView::Hour );
  d->mGantt->zoomToFit();

  d->mGantt->setUpdateEnabled( false );
  d->mGantt->clear();
#else
  qDebug() << "Disabled code, port to KDGantt2";
#endif

  d->mLeftView->clear();
  uint index = 0;
  // item for every calendar
  TimelineItem *item = 0;
  Akonadi::ETMCalendar::Ptr calres = calendar();
  if ( !calres ) {
    item = new TimelineItem( calendar(),
                             index++,
                             static_cast<QStandardItemModel*>( d->mGantt->model() ),
                             d->mGantt );
    d->mLeftView->addTopLevelItem( new QTreeWidgetItem( QStringList() << i18n( "Calendar" ) ) );
    d->mCalendarItemMap.insert( -1, item );

  } else {
    const CalendarSupport::CollectionSelection *colSel = collectionSelection();
    const Akonadi::Collection::List collections = colSel->selectedCollections();

    Q_FOREACH ( const Akonadi::Collection &collection, collections ) {
      if ( collection.contentMimeTypes().contains( Event::eventMimeType() ) ) {
        item = new TimelineItem( calendar(),
                                 index++,
                                 static_cast<QStandardItemModel*>( d->mGantt->model() ),
                                 d->mGantt );
        d->mLeftView->addTopLevelItem(
          new QTreeWidgetItem(
            QStringList() << CalendarSupport::displayName( calendar().data(), collection ) ) );
        const QColor resourceColor = EventViews::resourceColor( collection, preferences() );
        if ( resourceColor.isValid() ) {
          item->setColor( resourceColor );
        }
        qDebug() << "Created item " << item
                 << " (" <<  CalendarSupport::displayName( calendar().data(), collection ) << ") "
                 << "with index " <<  index - 1 << " from collection " << collection.id();
        d->mCalendarItemMap.insert( collection.id(), item );
      }
    }
  }

  // add incidences

  /**
   * We remove the model from the view here while we fill it with items,
   * because every call to insertIncidence will cause the view to do an expensive
   * updateScene() call otherwise.
   */
  QAbstractItemModel *ganttModel = d->mGantt->model();
  d->mGantt->setModel( 0 );

  KCalCore::Event::List events;
  KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
  for ( QDate day = start; day <= end; day = day.addDays( 1 ) ) {
    events = calendar()->events( day, timeSpec,
                                 KCalCore::EventSortStartDate,
                                 KCalCore::SortDirectionAscending );
    Q_FOREACH ( const KCalCore::Event::Ptr &event, events ) {
      if ( event->hasRecurrenceId() ) {
        continue;
      }
      Akonadi::Item item = calendar()->item( event );
      d->insertIncidence( item, day );
    }
  }
  d->mGantt->setModel( ganttModel );
  d->splitterMoved();
}

/*virtual*/
void TimelineView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
}

/*virtual*/
void TimelineView::updateView()
{
  if ( d->mStartDate.isValid() && d->mEndDate.isValid() ) {
    showDates( d->mStartDate, d->mEndDate );
  }
}

/*virtual*/
void TimelineView::changeIncidenceDisplay( const Akonadi::Item &incidence, int mode )
{
  switch ( mode ) {
  case Akonadi::IncidenceChanger::ChangeTypeCreate:
    d->insertIncidence( incidence );
    break;
  case Akonadi::IncidenceChanger::ChangeTypeModify:
    d->removeIncidence( incidence );
    d->insertIncidence( incidence );
    break;
  case Akonadi::IncidenceChanger::ChangeTypeDelete:
    d->removeIncidence( incidence );
    break;
  default:
    updateView();
  }
}

bool TimelineView::eventDurationHint( QDateTime &startDt, QDateTime &endDt,
                                      bool &allDay ) const
{
  startDt = QDateTime( d->mHintDate );
  endDt = QDateTime( d->mHintDate.addSecs( 2 * 60 * 60 ) );
  allDay = false;
  return d->mHintDate.isValid();
}

// void TimelineView::overscale( KDGanttView::Scale scale )
// {
//   Q_UNUSED( scale );
//   /* Disabled, looks *really* bogus:
//      this triggers and endless rescaling loop; we want to set
//      a fixed scale, the Gantt view doesn't like it and rescales
//      (emitting a rescaling signal that leads here) and so on...
//   //set a relative zoom factor of 1 (?!)
//   d->mGantt->setZoomFactor( 1, false );
//   d->mGantt->setScale( KDGanttView::Hour );
//   d->mGantt->setMinorScaleCount( 12 );
//   */
// }

QDate TimelineView::startDate() const
{
  return d->mStartDate;
}

QDate TimelineView::endDate() const
{
  return d->mEndDate;
}

bool TimelineView::eventFilter( QObject *object, QEvent *event )
{
  if ( event->type() == QEvent::ToolTip ) {
    QHelpEvent *helpEvent = static_cast<QHelpEvent*>( event );
    QGraphicsItem *item = d->mGantt->itemAt( helpEvent->pos() );
    if ( item ) {
      if ( item->type() == KDGantt::GraphicsItem::Type ) {
        KDGantt::GraphicsItem *graphicsItem = static_cast<KDGantt::GraphicsItem*>( item );
        const QModelIndex itemIndex = graphicsItem->index();

        QStandardItemModel *itemModel =
          qobject_cast<QStandardItemModel*>( d->mGantt->model() );

        TimelineSubItem *timelineItem =
          dynamic_cast<TimelineSubItem*>( itemModel->item( itemIndex.row(), itemIndex.column() ) );

        if ( timelineItem ) {
          timelineItem->updateToolTip();
        }
      }
    }
  }

  return EventView::eventFilter( object, event );
}

