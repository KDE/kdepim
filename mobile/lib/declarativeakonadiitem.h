#ifndef DECLARATIVEAKONADIITEM_H
#define DECLARATIVEAKONADIITEM_H

#include <QDeclarativeItem>

#include <AkonadiCore/Item>

#include "mobileui_export.h"

class DeclarativeAkonadiItemPrivate;

class MOBILEUI_EXPORT DeclarativeAkonadiItem : public QDeclarativeItem
{
  Q_OBJECT
  Q_PROPERTY( Akonadi::Item item READ item )
  Q_PROPERTY( int itemId READ itemId WRITE setItemId )
  Q_PROPERTY( double swipeLength READ swipeLength WRITE setSwipeLength )

public:
  /**
   * Returns the current Akonadi::Item. The default implementation returns an
   * item without payload.
   */
  virtual Akonadi::Item item() const;

  /**
   * Set/get the Akonadi::Item::Id. We use quint64 here so we can deal with
   * the id in QML files too.
   */
  virtual qint64 itemId() const = 0;
  virtual void setItemId( qint64 id ) = 0;

  /**
   * The length, expressed as percentage of the width, which trigers the next
   * or previous requests.
   *
   * Value must be between 0 and 1.
   */
  double swipeLength() const;
  void setSwipeLength( double length );

signals:
  void nextItemRequest();
  void previousItemRequest();

protected:
  explicit DeclarativeAkonadiItem( QDeclarativeItem *parent = 0 );
  ~DeclarativeAkonadiItem();

  /**
   * Sets the widget that show the current item. If a widget currently is set,
   * it will be deleted.
   */
  void setWidget( QWidget *widget );

  /** Reimplement QObject::eventFilter() for swiping */
  virtual bool eventFilter( QObject *obj, QEvent *event );

  /** Reimplement QObject::geometryChanged() to update the size of the internal widget */
  virtual void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry);

  /**
   * Are called by the event filter when when a vertical drag is detected. By
   * default these functions do nothing. Subclasses should reimplement them when
   * needed.
   *
   * @param dist the distance in pixels to scroll.
   */
  virtual void scrollDown( int dist );
  virtual void scrollUp( int dist );

  /**
   * A mouse click was detected by the custom event handling. If you need to
   * pass through mouse clicks to an internal widget, reimplement this method.
   *
   * Note: We assume usage on a device, which has not the concept of left or right
   *       mouse buttons as it is operated with a finger. Therefore, on normal
   *       systems this method is only triggered for left mouse button clicks.
   */
  virtual void simulateMouseClick( const QPoint &pos );

private:
  DeclarativeAkonadiItemPrivate * const d_ptr;
  Q_DECLARE_PRIVATE( DeclarativeAkonadiItem )
  Q_DISABLE_COPY( DeclarativeAkonadiItem )
};

#endif // DECLARATIVEAKONADIITEM_H
