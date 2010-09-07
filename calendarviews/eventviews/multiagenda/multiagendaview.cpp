/*
  Copyright (c) 2007 Volker Krause <vkrause@kde.org>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Sergio Martins <sergio.martins@kdab.com>

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
*/
#include "multiagendaview.h"
#include "configdialoginterface.h"
#include "prefs.h"

#include "agenda/agenda.h"
#include "agenda/agendaview.h"
#include "agenda/timelabelszone.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/collectionselectionproxymodel.h>
#include <calendarsupport/entitymodelstatesaver.h>
#include <calendarsupport/utils.h>

#include <KCalCore/Event>

#include <KGlobalSettings>
#include <KLocale>
#include <KVBox>

#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplitter>
#include <QTimer>

using namespace EventViews;

/**
   Function for debugging purposes:
   prints an object's sizeHint()/minimumSizeHint()/policy
   and it's children's too, recursively
*/
/*
static void printObject( QObject *o, int level = 0 )
{
  QMap<int,QString> map;
  map.insert( 0, "Fixed" );
  map.insert( 1, "Minimum" );
  map.insert( 4, "Maximum" );
  map.insert( 5, "Preferred" );
  map.insert( 7, "Expanding" );
  map.insert( 3, "MinimumExpaning" );
  map.insert( 13, "Ignored" );

  QWidget *w = qobject_cast<QWidget*>( o );

  if ( w ) {
    qDebug() << QString( level*2, '-' ) << o
             << w->sizeHint() << "/" << map[w->sizePolicy().verticalPolicy()]
             << "; minimumSize = " << w->minimumSize()
             << "; minimumSizeHint = " << w->minimumSizeHint();
  } else {
    qDebug() << QString( level*2, '-' ) << o ;
  }

  foreach( QObject *child, o->children() ) {
    printObject( child, level + 1 );
  }
}
*/

static QString generateColumnLabel( int c )
{
  return i18n( "Agenda %1", c + 1 );
}

class MultiAgendaView::Private
{
  public:
    Private( MultiAgendaView *qq ) :
      q( qq ),
      mUpdateOnShow( true ),
      mPendingChanges( true ),
      mCustomColumnSetupUsed( false ),
      mCustomNumberOfColumns( 2 )
    {
    }

    ~Private()
    {
    }

    void addView( const Akonadi::Collection &collection );
    void addView( CalendarSupport::CollectionSelectionProxyModel *selectionProxy,
                  const QString &title );
    AgendaView *createView( const QString &title );
    void deleteViews();
    void setupViews();
    void resizeScrollView( const QSize &size );

    MultiAgendaView *q;
    QList<AgendaView*> mAgendaViews;
    QList<QWidget*> mAgendaWidgets;
    KHBox *mTopBox;
    QScrollArea *mScrollArea;
    TimeLabelsZone *mTimeLabelsZone;
    QSplitter *mLeftSplitter, *mRightSplitter;
    QScrollBar *mScrollBar;
    QWidget *mLeftBottomSpacer, *mRightBottomSpacer;
    QDate mStartDate, mEndDate;
    bool mUpdateOnShow;
    bool mPendingChanges;
    bool mCustomColumnSetupUsed;
    QVector<CalendarSupport::CollectionSelectionProxyModel*> mCollectionSelectionModels;
    QVector<QString> mCustomColumnTitles;
    int mCustomNumberOfColumns;
    QLabel *mLabel;
    QWidget *mRightDummyWidget;
};

MultiAgendaView::MultiAgendaView( QWidget *parent )
  : EventView( parent ), d( new Private( this ) )

{
  QHBoxLayout *topLevelLayout = new QHBoxLayout( this );
  topLevelLayout->setSpacing( 0 );
  topLevelLayout->setMargin( 0 );

  QFontMetrics fm( font() );
  int topLabelHeight = 2 * fm.height() + fm.lineSpacing();

  KVBox *topSideBox = new KVBox( this );

  QWidget *topSideSpacer = new QWidget( topSideBox );
  topSideSpacer->setFixedHeight( topLabelHeight );

  d->mLeftSplitter = new QSplitter( Qt::Vertical, topSideBox );
  d->mLeftSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );

  d->mLabel = new QLabel( i18n( "All Day" ), d->mLeftSplitter );
  d->mLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  d->mLabel->setWordWrap( true );

  KVBox *sideBox = new KVBox( d->mLeftSplitter );

  EventIndicator *eiSpacer = new EventIndicator( EventIndicator::Top, sideBox );
  eiSpacer->changeColumns( 0 );

  // compensate for the frame the agenda views but not the timelabels have
  QWidget *timeLabelTopAlignmentSpacer = new QWidget( sideBox );

  d->mTimeLabelsZone = new TimeLabelsZone( sideBox, PrefsPtr( new Prefs() ) );

  QWidget *timeLabelBotAlignmentSpacer = new QWidget( sideBox );

  eiSpacer = new EventIndicator( EventIndicator::Bottom, sideBox );
  eiSpacer->changeColumns( 0 );

  d->mLeftBottomSpacer = new QWidget( topSideBox );

  topLevelLayout->addWidget( topSideBox );

  d->mScrollArea = new QScrollArea( this );
  d->mScrollArea->setWidgetResizable( true );

  d->mScrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  // BUG: timelabels aren't aligned with the agenda's grid, 2 or 3 pixels of offset.
  // asymetric since the timelabels
  timeLabelTopAlignmentSpacer->setFixedHeight( d->mScrollArea->frameWidth() - 1 );
  // have 25 horizontal lines
  timeLabelBotAlignmentSpacer->setFixedHeight( d->mScrollArea->frameWidth() - 2 );

  d->mScrollArea->setFrameShape( QFrame::NoFrame );
  topLevelLayout->addWidget( d->mScrollArea, 100 );
  d->mTopBox = new KHBox( d->mScrollArea->viewport() );
  d->mScrollArea->setWidget( d->mTopBox );

  topSideBox = new KVBox( this );

  topSideSpacer = new QWidget( topSideBox );
  topSideSpacer->setFixedHeight( topLabelHeight );

  d->mRightSplitter = new QSplitter( Qt::Vertical, topSideBox );
  d->mRightSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );

  d->mRightDummyWidget = new QWidget( d->mRightSplitter );
  sideBox = new KVBox( d->mRightSplitter );

  eiSpacer = new EventIndicator( EventIndicator::Top, sideBox );
  eiSpacer->setFixedHeight( eiSpacer->minimumHeight() );
  eiSpacer->changeColumns( 0 );
  d->mScrollBar = new QScrollBar( Qt::Vertical, sideBox );
  eiSpacer = new EventIndicator( EventIndicator::Bottom, sideBox );
  eiSpacer->setFixedHeight( eiSpacer->minimumHeight() );
  eiSpacer->changeColumns( 0 );

  d->mRightBottomSpacer = new QWidget( topSideBox );
  topLevelLayout->addWidget( topSideBox );
}

void MultiAgendaView::setCalendar( CalendarSupport::Calendar *cal )
{
  EventView::setCalendar( cal );
  Q_FOREACH ( CalendarSupport::CollectionSelectionProxyModel *const i,
              d->mCollectionSelectionModels ) {
    i->setSourceModel( cal->treeModel() );
  }
  recreateViews();
}

void MultiAgendaView::recreateViews()
{
  if ( !d->mPendingChanges ) {
    return;
  }
  d->mPendingChanges = false;

  d->deleteViews();

  if ( d->mCustomColumnSetupUsed ) {
    Q_ASSERT( d->mCollectionSelectionModels.size() == d->mCustomNumberOfColumns );
    for ( int i = 0; i < d->mCustomNumberOfColumns; ++i ) {
      d->addView( d->mCollectionSelectionModels[i], d->mCustomColumnTitles[i] );
    }
  } else {
    Q_FOREACH( const Akonadi::Collection &i, collectionSelection()->selectedCollections() ) {
      if ( i.contentMimeTypes().contains( KCalCore::Event::eventMimeType() ) ) {
        d->addView( i );
      }
    }
  }
  // no resources activated, so stop here to avoid crashing somewhere down the line
  // TODO: show a nice message instead
  if ( d->mAgendaViews.isEmpty() ) {
    return;
  }

  d->setupViews();
  QTimer::singleShot( 0, this, SLOT(slotResizeScrollView()) );
  d->mTimeLabelsZone->updateAll();

  QScrollArea *timeLabel = d->mTimeLabelsZone->timeLabels().first();
  connect( timeLabel->verticalScrollBar(), SIGNAL(valueChanged(int)),
           d->mScrollBar, SLOT(setValue(int)) );
  connect( d->mScrollBar, SIGNAL(valueChanged(int)),
           timeLabel->verticalScrollBar(), SLOT(setValue(int)) );

  connect( d->mLeftSplitter, SIGNAL(splitterMoved(int,int)), SLOT(resizeSplitters()) );
  connect( d->mRightSplitter, SIGNAL(splitterMoved(int,int)), SLOT(resizeSplitters()) );
  QTimer::singleShot( 0, this, SLOT(resizeSplitters()) );
  QTimer::singleShot( 0, this, SLOT(setupScrollBar()) );

  d->mTimeLabelsZone->updateTimeLabelsPosition();

}

void MultiAgendaView::Private::deleteViews()
{
  Q_FOREACH ( AgendaView *const i, mAgendaViews ) {
    CalendarSupport::CollectionSelectionProxyModel *proxy =
      i->takeCustomCollectionSelectionProxyModel();
    if ( proxy && !mCollectionSelectionModels.contains( proxy ) ) {
      delete proxy;
    }
    delete i;
  }

  mAgendaViews.clear();
  mTimeLabelsZone->setAgendaView( 0 );
  qDeleteAll( mAgendaWidgets );
  mAgendaWidgets.clear();
}

void MultiAgendaView::Private::setupViews()
{
  foreach ( AgendaView *agenda, mAgendaViews ) {
    q->connect( agenda, SIGNAL(newEventSignal(Akonadi::Collection::List)),
                q, SIGNAL(newEventSignal(Akonadi::Collection::List)) );
    q->connect( agenda, SIGNAL(editIncidenceSignal(Akonadi::Item)),
                q, SIGNAL(editIncidenceSignal(Akonadi::Item)) );
    q->connect( agenda, SIGNAL(showIncidenceSignal(Akonadi::Item)),
                q, SIGNAL(showIncidenceSignal(Akonadi::Item)) );
    q->connect( agenda, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
                q, SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );
    q->connect( agenda, SIGNAL(startMultiModify(QString)),
                q, SIGNAL(startMultiModify(QString)) );
    q->connect( agenda, SIGNAL(endMultiModify()),
                q, SIGNAL(endMultiModify()) );

    q->connect( agenda, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
                q, SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

    q->connect( agenda, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
                q, SIGNAL(cutIncidenceSignal(Akonadi::Item)) );
    q->connect( agenda, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
                q, SIGNAL(copyIncidenceSignal(Akonadi::Item)) );
    q->connect( agenda, SIGNAL(pasteIncidenceSignal()),
                q, SIGNAL(pasteIncidenceSignal()) );
    q->connect( agenda, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
                q, SIGNAL(toggleAlarmSignal(Akonadi::Item)) );
    q->connect( agenda, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate) ),
                q, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)) );

    q->connect( agenda, SIGNAL(newEventSignal(Akonadi::Collection::List,QDate)),
                q, SIGNAL(newEventSignal(Akonadi::Collection::List,QDate)) );
    q->connect( agenda, SIGNAL(newEventSignal(Akonadi::Collection::List,QDateTime)),
                q, SIGNAL(newEventSignal(Akonadi::Collection::List,QDateTime)) );
    q->connect( agenda,
                SIGNAL(newEventSignal(Akonadi::Collection::List,QDateTime,QDateTime)),
                q, SIGNAL(newEventSignal(Akonadi::Collection::List,QDateTime,QDateTime)) );

    q->connect( agenda, SIGNAL(newTodoSignal(QDate)),
                q, SIGNAL(newTodoSignal(QDate)) );

    q->connect( agenda, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
                q, SLOT(slotSelectionChanged()) );

    q->connect( agenda, SIGNAL(timeSpanSelectionChanged()),
                q, SLOT(slotClearTimeSpanSelection()) );

    q->disconnect( agenda->agenda(),
                   SIGNAL(zoomView(int,QPoint,Qt::Orientation)),
                   agenda, 0 );
    q->connect( agenda->agenda(),
                SIGNAL(zoomView(int,QPoint,Qt::Orientation)),
                q, SLOT(zoomView(int,QPoint,Qt::Orientation)) );
  }

  AgendaView *lastView = mAgendaViews.last();
  foreach ( AgendaView *agenda, mAgendaViews ) {
    if ( agenda != lastView ) {
      connect( agenda->agenda()->verticalScrollBar(), SIGNAL(valueChanged(int)),
               lastView->agenda()->verticalScrollBar(), SLOT(setValue(int)) );
    }
  }

  foreach ( AgendaView *agenda, mAgendaViews ) {
    agenda->readSettings();
  }

  int minWidth = 0;
  foreach ( QWidget *widget, mAgendaWidgets ) {
    minWidth = qMax( minWidth, widget->minimumSizeHint().width() );
  }
  foreach ( QWidget *widget, mAgendaWidgets ) {
    widget->setMinimumWidth( minWidth );
  }
}

MultiAgendaView::~MultiAgendaView()
{
  delete d;
}

Akonadi::Item::List MultiAgendaView::selectedIncidences() const
{
  Akonadi::Item::List list;
  foreach ( AgendaView *agendaView, d->mAgendaViews ) {
    list += agendaView->selectedIncidences();
  }
  return list;
}

KCalCore::DateList MultiAgendaView::selectedIncidenceDates() const
{
  KCalCore::DateList list;
  foreach ( AgendaView *agendaView, d->mAgendaViews ) {
    list += agendaView->selectedIncidenceDates();
  }
  return list;
}

int MultiAgendaView::currentDateCount() const
{
  foreach ( AgendaView *agendaView, d->mAgendaViews ) {
    return agendaView->currentDateCount();
  }
  return 0;
}

void MultiAgendaView::showDates( const QDate &start, const QDate &end )
{
  d->mStartDate = start;
  d->mEndDate = end;
  slotResizeScrollView();
  d->mTimeLabelsZone->updateAll();
  foreach ( AgendaView *agendaView, d->mAgendaViews ) {
    agendaView->showDates( start, end );
  }
}

void MultiAgendaView::showIncidences( const Akonadi::Item::List &incidenceList,
                                      const QDate &date )
{
  foreach ( AgendaView *agendaView, d->mAgendaViews ) {
    agendaView->showIncidences( incidenceList, date );
  }
}

void MultiAgendaView::updateView()
{
  recreateViews();
  foreach ( AgendaView *agendaView, d->mAgendaViews ) {
    agendaView->updateView();
  }
}

int MultiAgendaView::maxDatesHint() const
{
  // these maxDatesHint functions aren't used
  return AgendaView::MAX_DAY_COUNT;
}

void MultiAgendaView::slotSelectionChanged()
{
  foreach ( AgendaView *agenda, d->mAgendaViews ) {
    if ( agenda != sender() ) {
      agenda->clearSelection();
    }
  }
}

bool MultiAgendaView::eventDurationHint( QDateTime &startDt, QDateTime &endDt,
                                         bool &allDay ) const
{
  foreach ( AgendaView *agenda, d->mAgendaViews ) {
    bool valid = agenda->eventDurationHint( startDt, endDt, allDay );
    if ( valid ) {
      return true;
    }
  }
  return false;
}

void MultiAgendaView::slotClearTimeSpanSelection()
{
  foreach ( AgendaView *agenda, d->mAgendaViews ) {
    if ( agenda != sender() ) {
      agenda->clearTimeSpanSelection();
    }
  }
}

AgendaView *MultiAgendaView::Private::createView( const QString &title )
{
  QWidget *box = new QWidget( mTopBox );
  QVBoxLayout *layout = new QVBoxLayout( box );
  layout->setMargin( 0 );
  QLabel *l = new QLabel( title );
  layout->addWidget( l );
  l->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
  AgendaView *av = new AgendaView( q->startDateTime().date(),
                                   q->endDateTime().date(),
                                   true, q );
  layout->addWidget( av );
  av->setCalendar( q->calendar() );
  av->setIncidenceChanger( q->changer() );
  av->agenda()->scrollArea()->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mAgendaViews.append( av );
  mAgendaWidgets.append( box );
  box->show();
  mTimeLabelsZone->setAgendaView( av );

  q->connect( mScrollBar, SIGNAL(valueChanged(int)),
              av->agenda()->verticalScrollBar(), SLOT(setValue(int)) );

  q->connect( av->splitter(), SIGNAL(splitterMoved(int,int)),
              q, SLOT(resizeSplitters()) );
  q->connect( av, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)),
              q, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)) );

  q->connect( av, SIGNAL(showNewEventPopupSignal()),
              q, SIGNAL(showNewEventPopupSignal()) );

  const QSize minHint = av->allDayAgenda()->scrollArea()->minimumSizeHint();

  if ( minHint.isValid() ) {
    mLabel->setMinimumHeight( minHint.height() );
    mRightDummyWidget->setMinimumHeight( minHint.height() );
  }

  return av;
}

void MultiAgendaView::Private::addView( const Akonadi::Collection &collection )
{
  AgendaView *av = createView( CalendarSupport::displayName( collection ) );
  av->setCollectionId( collection.id() );
}

void MultiAgendaView::Private::addView( CalendarSupport::CollectionSelectionProxyModel *sm,
                                        const QString &title )
{
  AgendaView *av = createView( title );
  av->setCustomCollectionSelectionProxyModel( sm );
}

void MultiAgendaView::resizeEvent( QResizeEvent *ev )
{
  d->resizeScrollView( ev->size() );
  EventView::resizeEvent( ev );
}

void MultiAgendaView::Private::resizeScrollView( const QSize &size )
{
  const int widgetWidth = size.width() - mTimeLabelsZone->width() -
                          mScrollBar->width();

  int height = size.height();
  if ( mScrollArea->horizontalScrollBar()->isVisible() ) {
    // this should never happen, you can't get horizontalScrollBars
    const int sbHeight = mScrollArea->horizontalScrollBar()->height();
    height -= sbHeight;
    mLeftBottomSpacer->setFixedHeight( sbHeight );
    mRightBottomSpacer->setFixedHeight( sbHeight );
  } else {
    mLeftBottomSpacer->setFixedHeight( 0 );
    mRightBottomSpacer->setFixedHeight( 0 );
  }

  mScrollArea->widget()->setFixedSize( widgetWidth, height );

  mTopBox->resize( widgetWidth, height );
}

void MultiAgendaView::setIncidenceChanger( CalendarSupport::IncidenceChanger *changer )
{
  EventView::setIncidenceChanger( changer );
  foreach ( AgendaView *agenda, d->mAgendaViews ) {
    agenda->setIncidenceChanger( changer );
  }
}

void MultiAgendaView::updateConfig()
{
  EventView::updateConfig();
  d->mTimeLabelsZone->updateAll();
  foreach ( AgendaView *agenda, d->mAgendaViews ) {
    agenda->updateConfig();
  }
}

void MultiAgendaView::resizeSplitters()
{
  if ( d->mAgendaViews.isEmpty() ) {
    return;
  }

  QSplitter *lastMovedSplitter = qobject_cast<QSplitter*>( sender() );
  if ( !lastMovedSplitter ) {
    lastMovedSplitter = d->mAgendaViews.first()->splitter();
  }
  foreach ( AgendaView *agenda, d->mAgendaViews ) {
    if ( agenda->splitter() == lastMovedSplitter ) {
      continue;
    }
    agenda->splitter()->setSizes( lastMovedSplitter->sizes() );
  }
  if ( lastMovedSplitter != d->mLeftSplitter ) {
    d->mLeftSplitter->setSizes( lastMovedSplitter->sizes() );
  }
  if ( lastMovedSplitter != d->mRightSplitter ) {
    d->mRightSplitter->setSizes( lastMovedSplitter->sizes() );
  }
}

void MultiAgendaView::zoomView( const int delta, const QPoint &pos, const Qt::Orientation ori )
{
  const int hourSz = preferences()->hourSize();
  if ( ori == Qt::Vertical ) {
    if ( delta > 0 ) {
      if ( hourSz > 4 ) {
        preferences()->setHourSize( hourSz - 1 );
      }
    } else {
      preferences()->setHourSize( hourSz + 1 );
    }
  }

  foreach ( AgendaView *agenda, d->mAgendaViews ) {
    agenda->zoomView( delta, pos, ori );
  }

  d->mTimeLabelsZone->updateAll();
}

void MultiAgendaView::slotResizeScrollView()
{
  d->resizeScrollView( size() );
}

void MultiAgendaView::showEvent( QShowEvent *event )
{
  EventView::showEvent( event );
  if ( d->mUpdateOnShow ) {
    d->mUpdateOnShow = false;
    d->mPendingChanges = true; // force a full view recreation
    showDates( d->mStartDate, d->mEndDate );
  }
}

void MultiAgendaView::setChanges( Changes changes )
{
  EventView::setChanges( changes );
  foreach ( AgendaView *agenda, d->mAgendaViews ) {
    agenda->setChanges( changes );
  }
}

void MultiAgendaView::setupScrollBar()
{
  if ( !d->mAgendaViews.isEmpty() && d->mAgendaViews.first()->agenda() ) {
    QScrollBar *scrollBar = d->mAgendaViews.first()->agenda()->verticalScrollBar();
    d->mScrollBar->setMinimum( scrollBar->minimum() );
    d->mScrollBar->setMaximum( scrollBar->maximum() );
    d->mScrollBar->setSingleStep( scrollBar->singleStep() );
    d->mScrollBar->setPageStep( scrollBar->pageStep() );
    d->mScrollBar->setValue( scrollBar->value() );
  }
}

void MultiAgendaView::collectionSelectionChanged()
{
  kDebug();
  d->mPendingChanges = true;
  recreateViews();
}

bool MultiAgendaView::hasConfigurationDialog() const
{

  /** The wrapper in korg has the dialog. Too complicated to move to CalendarViews.
      Depends on korg/AkonadiCollectionView, and will be refactored some day
      to get rid of CollectionSelectionProxyModel/EntityStateSaver */
  return false;
}

void MultiAgendaView::doRestoreConfig( const KConfigGroup &configGroup )
{
  d->mCustomColumnSetupUsed = configGroup.readEntry( "UseCustomColumnSetup", false );
  d->mCustomNumberOfColumns = configGroup.readEntry( "CustomNumberOfColumns", 2 );
  d->mCustomColumnTitles =  configGroup.readEntry( "ColumnTitles", QStringList() ).toVector();
  if ( d->mCustomColumnTitles.size() != d->mCustomNumberOfColumns ) {
    const int orig = d->mCustomColumnTitles.size();
    d->mCustomColumnTitles.resize( d->mCustomNumberOfColumns );
    for ( int i = orig; i < d->mCustomNumberOfColumns; ++i ) {
      d->mCustomColumnTitles[i] = generateColumnLabel( i );
    }
  }
  QVector<CalendarSupport::CollectionSelectionProxyModel*>oldModels = d->mCollectionSelectionModels;
  d->mCollectionSelectionModels.clear();
  d->mCollectionSelectionModels.resize( d->mCustomNumberOfColumns );
  for ( int i = 0; i < d->mCustomNumberOfColumns; ++i ) {
    const KConfigGroup g = configGroup.config()->group( configGroup.name() +
                                                        "_subView_" +
                                                        QByteArray::number( i ) );
    CalendarSupport::CollectionSelectionProxyModel *selection =
      new CalendarSupport::CollectionSelectionProxyModel;
    selection->setCheckableColumn( CalendarSupport::CalendarModel::CollectionTitle );
    selection->setDynamicSortFilter( true );
    if ( calendar() ) {
      selection->setSourceModel( calendar()->treeModel() );
    }

    QItemSelectionModel *qsm = new QItemSelectionModel( selection, selection );
    selection->setSelectionModel( qsm );
    CalendarSupport::EntityModelStateSaver *saver =
      new CalendarSupport::EntityModelStateSaver( selection, selection );
    saver->addRole( Qt::CheckStateRole, "CheckState" );
    saver->restoreConfig( g );
    d->mCollectionSelectionModels[i] = selection;
  }
  d->mPendingChanges = true;
  recreateViews();
  qDeleteAll( oldModels );
}

void MultiAgendaView::doSaveConfig( KConfigGroup &configGroup )
{
  configGroup.writeEntry( "UseCustomColumnSetup", d->mCustomColumnSetupUsed );
  configGroup.writeEntry( "CustomNumberOfColumns", d->mCustomNumberOfColumns );
  const QStringList titleList = d->mCustomColumnTitles.toList();
  configGroup.writeEntry( "ColumnTitles", titleList );
  int idx = 0;
  Q_FOREACH ( CalendarSupport::CollectionSelectionProxyModel *i, d->mCollectionSelectionModels ) {
    KConfigGroup g = configGroup.config()->group( configGroup.name() +
                                                  "_subView_" +
                                                  QByteArray::number( idx ) );
    ++idx;
    CalendarSupport::EntityModelStateSaver saver( i );
    saver.addRole( Qt::CheckStateRole, "CheckState" );
    saver.saveConfig( g );
  }
}

void MultiAgendaView::customCollectionsChanged( ConfigDialogInterface *dlg )
{
  if ( !d->mCustomColumnSetupUsed && !dlg->useCustomColumns() ) {
    // Config didn't change, no need to recreate views
    return;
  }

  d->mCustomColumnSetupUsed = dlg->useCustomColumns();
  d->mCustomNumberOfColumns = dlg->numberOfColumns();
  QVector<CalendarSupport::CollectionSelectionProxyModel*> newModels;
  newModels.resize( d->mCustomNumberOfColumns );
  d->mCustomColumnTitles.resize( d->mCustomNumberOfColumns );
  for ( int i = 0; i < d->mCustomNumberOfColumns; ++i ) {
    newModels[i] = dlg->takeSelectionModel( i );
    d->mCustomColumnTitles[i] = dlg->columnTitle( i );
  }
  d->mCollectionSelectionModels = newModels;
  d->mPendingChanges = true;
  recreateViews();
}

bool MultiAgendaView::customColumnSetupUsed() const
{
  return d->mCustomColumnSetupUsed;
}

int MultiAgendaView::customNumberOfColumns() const
{
  return d->mCustomNumberOfColumns;
}

QVector<CalendarSupport::CollectionSelectionProxyModel*>
MultiAgendaView::collectionSelectionModels() const
{
  return d->mCollectionSelectionModels;
}

QVector<QString> MultiAgendaView::customColumnTitles() const
{
  return d->mCustomColumnTitles;
}

#include "multiagendaview.moc"
