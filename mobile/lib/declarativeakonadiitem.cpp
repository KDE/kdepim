#include "declarativeakonadiitem.h"


#include <QtCore/QTimer>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <qabstractscrollarea.h>
#include <qscrollbar.h>
#include <QCoreApplication>

#include <qmath.h>

static const double sDirectionThreshHold = 8.5; /// Threshold in pixels

/// DeclarativeAkonadiItemPrivate

struct DeclarativeAkonadiItemPrivate
{
/// Enum
  enum Direction {
    Unknown, /// Need more events to determin the actual direction.
    Up,
    Down,
    Left,
    Right
  };

/// Members
  QGraphicsProxyWidget *mProxy;

  /// Handle mouse events for clicks and swipes
  QTimer mClickDetectionTimer;
  bool mMousePressed;
  int mDx;
  int mDy;
  double mSwipeLength;

/// Methods
  DeclarativeAkonadiItemPrivate();
  Direction direction() const;
};

DeclarativeAkonadiItemPrivate::DeclarativeAkonadiItemPrivate()
  : mProxy( 0 )
  , mMousePressed( false )
  , mDx( 0 )
  , mDy( 0 )
  , mSwipeLength( 0 )
{ }

DeclarativeAkonadiItemPrivate::Direction DeclarativeAkonadiItemPrivate::direction() const
{
  const double length = qSqrt( ( mDx ^ 2 )  + ( mDy ^ 2 ) );
  if (length < sDirectionThreshHold )
    return Unknown;

  bool horizontal = false;
  if ( mDx != 0 ) {
    // We Use an X shape to determine the direction of the move.
    // tan(45) == 1; && tan(-45) == -1; Same for 135 and 225 degrees
    double angle = mDy / mDx;
    horizontal = angle > -1 && angle <= 1;
  }

  Direction dir;
  if ( horizontal ) {
    dir = mDx > 0 ? Right : Left;
  } else {
    dir = mDy > 0 ? Up : Down;
  }

  return dir;
}

/// DeclarativeAkonadiItem

DeclarativeAkonadiItem::DeclarativeAkonadiItem( QDeclarativeItem *parent )
  : QDeclarativeItem( parent )
  , d_ptr( new DeclarativeAkonadiItemPrivate )
{
  Q_D( DeclarativeAkonadiItem );

  d->mClickDetectionTimer.setInterval( 150 );
  d->mClickDetectionTimer.setSingleShot( true );

  d->mProxy = new QGraphicsProxyWidget( this );
  d->mProxy->installEventFilter( this );
}

DeclarativeAkonadiItem::~DeclarativeAkonadiItem()
{
  // Weird, the proxy seems to be already deleted at this point. If we get crashes
  // related to events we need to do this different.
  //d_ptr->mProxy->removeEventFilter( this );
  delete d_ptr;
}

Akonadi::Item DeclarativeAkonadiItem::item() const
{
  return Akonadi::Item( itemId() );
}

void DeclarativeAkonadiItem::setWidget( QWidget *widget )
{
  Q_D( DeclarativeAkonadiItem );

  if ( QWidget *curWidget = d->mProxy->widget() )
    delete curWidget;

  d->mProxy->setWidget( widget );
}

double DeclarativeAkonadiItem::swipeLength() const
{
  Q_D( const DeclarativeAkonadiItem );
  return d->mSwipeLength;
}

void DeclarativeAkonadiItem::setSwipeLength( double length )
{
  Q_D( DeclarativeAkonadiItem );
  Q_ASSERT( length >= 0 && length <= 1 );
  d->mSwipeLength = length;
}

void DeclarativeAkonadiItem::geometryChanged( const QRectF &newGeometry,
                                              const QRectF&oldGeometry )
{
  Q_D( DeclarativeAkonadiItem );
  QDeclarativeItem::geometryChanged( newGeometry, oldGeometry );
  d->mProxy->resize( newGeometry.size() );
}

bool DeclarativeAkonadiItem::eventFilter( QObject *obj, QEvent *ev )
{
  Q_D( DeclarativeAkonadiItem );

  if ( ev->type() == QEvent::GraphicsSceneMousePress ) {
    QGraphicsSceneMouseEvent *mev = static_cast<QGraphicsSceneMouseEvent*>( ev );
    if ( mev->button() == Qt::LeftButton ) {
      d->mMousePressed = true;
      d->mClickDetectionTimer.stop(); // Make sure that it isn't running atm
      d->mClickDetectionTimer.start();
      return true;
    }
  } else if ( ev->type() == QEvent::GraphicsSceneMouseRelease ) {
    const bool wasActive = d->mClickDetectionTimer.isActive();
    QGraphicsSceneMouseEvent *mev = static_cast<QGraphicsSceneMouseEvent*>( ev );
    if ( mev->button() == Qt::LeftButton ) {
      if ( wasActive ) // Timer didn't time out, we're dealing with a click
        simulateMouseClick( mev->pos().toPoint() );
      else if ( qAbs( d->mDx ) >= ( d->mSwipeLength * width() ) ) {
        // We don't trigger a next or previous *always*. Only when the configured
        // swipelength is met.
        const DeclarativeAkonadiItemPrivate::Direction dir = d->direction();
        if ( dir == DeclarativeAkonadiItemPrivate::Left ) {
          emit nextItemRequest();
        } else if ( dir == DeclarativeAkonadiItemPrivate::Right ) {
          emit previousItemRequest();
        }
      }

      d->mMousePressed = false;
      d->mDx = 0;
      d->mDy = 0;
      return true;
    }
  } else if ( ev->type() == QEvent::GraphicsSceneMouseMove && d->mMousePressed ) {
    QGraphicsSceneMouseEvent *mev = static_cast<QGraphicsSceneMouseEvent*>( ev );
    d->mDx += mev->pos().x() - mev->lastPos().x(); // Moving to right gives positive values
    d->mDy += mev->pos().y() - mev->lastPos().y(); // Moving up gives positive values

    const DeclarativeAkonadiItemPrivate::Direction dir = d->direction();
    if ( dir == DeclarativeAkonadiItemPrivate::Unknown )
      return true;
    if ( dir == DeclarativeAkonadiItemPrivate::Up ) {
      scrollUp( d->mDy );
      d->mDx = 0;
      d->mDy = 0;
    } else if ( dir == DeclarativeAkonadiItemPrivate::Down ) {
      scrollDown( d->mDy );
      d->mDx = 0;
      d->mDy = 0;
    }

    return true;
  }

  return QObject::eventFilter( obj, ev );
}

void DeclarativeAkonadiItem::scrollDown( int dist )
{
  QAbstractScrollArea *sa = qFindChild<QAbstractScrollArea*>( d_ptr->mProxy->widget() );
  if ( sa ) {
    // TODO make this actually scroll by the given pixel value
    sa->verticalScrollBar()->setValue( sa->verticalScrollBar()->value() - dist );
  }
}

void DeclarativeAkonadiItem::scrollUp( int dist )
{
  scrollDown( dist );
}

void DeclarativeAkonadiItem::simulateMouseClick( const QPoint &pos )
{
  if ( !d_ptr->mProxy->widget() )
    return;
  QWidget *receiver = d_ptr->mProxy->widget()->childAt( pos );
  if ( receiver ) {
    QPoint mappedPos = pos;
    if ( receiver != d_ptr->mProxy->widget() )
      mappedPos = receiver->mapFrom( d_ptr->mProxy->widget(), pos );
    QMouseEvent *event = new QMouseEvent( QEvent::MouseButtonPress, mappedPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
    QCoreApplication::sendEvent( receiver, event );
    event = new QMouseEvent( QEvent::MouseButtonRelease, mappedPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
    QCoreApplication::sendEvent( receiver, event );
  }
}

