/*
  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#include "agendaitem.h"
#include "eventview.h"
#include "helper.h"
#include "prefs.h"
#include "prefs_base.h" // for enums

#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KContacts/VCardDrag>

#include <KCalUtils/ICalDrag>
#include <KCalUtils/VCalDrag>
#include <KCalUtils/IncidenceFormatter>

#include <KPIMUtils/Email>

#include <KLocalizedString>
#include <KMessageBox>
#include <KWordWrap>
#include <KIconLoader>

#include <QDragEnterEvent>
#include <QPainter>
#include <QPixmapCache>
#include <QToolTip>
#include <QMimeData>
#include <KLocale>

using namespace KCalCore;
using namespace EventViews;

//-----------------------------------------------------------------------------

QPixmap *AgendaItem::alarmPxmp = 0;
QPixmap *AgendaItem::recurPxmp = 0;
QPixmap *AgendaItem::readonlyPxmp = 0;
QPixmap *AgendaItem::replyPxmp = 0;
QPixmap *AgendaItem::groupPxmp = 0;
QPixmap *AgendaItem::groupPxmpTent = 0;
QPixmap *AgendaItem::organizerPxmp = 0;
QPixmap *AgendaItem::eventPxmp = 0;

//-----------------------------------------------------------------------------

AgendaItem::AgendaItem( EventView *eventView, const Akonadi::ETMCalendar::Ptr &calendar,
                        const Akonadi::Item &item,
                        int itemPos, int itemCount,
                        const KDateTime &qd, bool isSelected, QWidget *parent )
  : QWidget( parent ), mEventView( eventView ), mCalendar( calendar ), mIncidence( item ),
    mOccurrenceDateTime( qd ), mValid( true ), mCloned( false ), mSelected( isSelected ), mSpecialEvent( false )
{
  if ( !CalendarSupport::hasIncidence( mIncidence ) ) {
    mValid = false;
    return;
  }

  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  mIncidence.setPayload( KCalCore::Incidence::Ptr( incidence->clone() ) );
  Q_ASSERT( incidence );
  if ( incidence->customProperty( "KABC", "BIRTHDAY" ) == QLatin1String("YES") ||
       incidence->customProperty( "KABC", "ANNIVERSARY" ) == QLatin1String("YES") ) {
    const int years = EventViews::yearDiff( incidence->dtStart().date(), qd.toTimeSpec( mEventView->preferences()->timeSpec() ).date() );
    if ( years > 0 ) {
      incidence = KCalCore::Incidence::Ptr( incidence->clone() );
      incidence->setReadOnly( false );
      incidence->setSummary( i18np( "%2 (1 year)", "%2 (%1 years)", years, incidence->summary() ) );
      incidence->setReadOnly( true );
      mCloned = true;
      mIncidence.setPayload<KCalCore::Incidence::Ptr>( incidence );
    }
  }

  mLabelText = incidence->summary();
  mIconAlarm = false;
  mIconRecur = false;
  mIconReadonly = false;
  mIconReply = false;
  mIconGroup = false;
  mIconGroupTent = false;
  mIconOrganizer = false;
  mMultiItemInfo = 0;
  mStartMoveInfo = 0;

  mItemPos = itemPos;
  mItemCount = itemCount;

  QPalette pal = palette();
  pal.setColor( QPalette::Window, Qt::transparent );
  setPalette( pal );

  setCellXY( 0, 0, 1 );
  setCellXRight( 0 );
  setMouseTracking( true );
  mResourceColor = QColor();
  updateIcons();

  setAcceptDrops( true );
}

AgendaItem::~AgendaItem()
{
}

void AgendaItem::updateIcons()
{
  if ( !mValid ) {
    return;
  }
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
  Q_ASSERT( incidence );
  mIconReadonly = incidence->isReadOnly();
  mIconRecur = incidence->recurs() || incidence->hasRecurrenceId();
  mIconAlarm = incidence->hasEnabledAlarms();
  if ( incidence->attendeeCount() > 1 ) {
    if ( mEventView->kcalPreferences()->thatIsMe( incidence->organizer()->email() ) ) {
      mIconReply = false;
      mIconGroup = false;
      mIconGroupTent = false;
      mIconOrganizer = true;
    } else {
      KCalCore::Attendee::Ptr me =
        incidence->attendeeByMails( mEventView->kcalPreferences()->allEmails() );

      if ( me ) {
        if ( me->status() == KCalCore::Attendee::NeedsAction && me->RSVP() ) {
          mIconReply = true;
          mIconGroup = false;
          mIconGroupTent = false;
          mIconOrganizer = false;
        } else if ( me->status() == KCalCore::Attendee::Tentative ) {
          mIconReply = false;
          mIconGroup = false;
          mIconGroupTent = true;
          mIconOrganizer = false;
        } else {
          mIconReply = false;
          mIconGroup = true;
          mIconGroupTent = false;
          mIconOrganizer = false;
        }
      } else {
        mIconReply = false;
        mIconGroup = true;
        mIconGroupTent = false;
        mIconOrganizer = false;
      }
    }
  }
  update();
}

void AgendaItem::select( bool selected )
{
  if ( mSelected != selected ) {
    mSelected = selected;
    update();
  }
}

bool AgendaItem::dissociateFromMultiItem()
{
  if ( !isMultiItem() ) {
    return false;
  }

  AgendaItem::QPtr firstItem = firstMultiItem();
  if ( firstItem == this ) {
    firstItem = nextMultiItem();
  }

  AgendaItem::QPtr lastItem = lastMultiItem();
  if ( lastItem == this ) {
    lastItem = prevMultiItem();
  }

  AgendaItem::QPtr prevItem = prevMultiItem();
  AgendaItem::QPtr nextItem = nextMultiItem();

  if ( prevItem ) {
    prevItem->setMultiItem( firstItem, prevItem->prevMultiItem(), nextItem, lastItem );
  }
  if ( nextItem ) {
    nextItem->setMultiItem( firstItem, prevItem, nextItem->prevMultiItem(), lastItem );
  }
  delete mMultiItemInfo;
  mMultiItemInfo = 0;
  return true;
}

void AgendaItem::setIncidence( const Akonadi::Item &incidence )
{
  mValid = false;
  if ( CalendarSupport::hasIncidence( incidence ) ) {
    mValid = true;
    mIncidence = incidence;
    mLabelText = CalendarSupport::incidence( incidence )->summary();
    updateIcons();
  }
}

/*
  Return height of item in units of agenda cells
*/
int AgendaItem::cellHeight() const
{
  return mCellYBottom - mCellYTop + 1;
}

/*
  Return height of item in units of agenda cells
*/
int AgendaItem::cellWidth() const
{
  return mCellXRight - mCellXLeft + 1;
}

void AgendaItem::setOccurrenceDateTime(const KDateTime& qd)
{
  mOccurrenceDateTime = qd;
}

QDate AgendaItem::occurrenceDate() const
{
  return mOccurrenceDateTime.toTimeSpec( mEventView->preferences()->timeSpec() ).date();
}

void AgendaItem::setCellXY( int X, int YTop, int YBottom )
{
  mCellXLeft = X;
  mCellYTop = YTop;
  mCellYBottom = YBottom;
}

void AgendaItem::setCellXRight( int XRight )
{
  mCellXRight = XRight;
}

void AgendaItem::setCellX( int XLeft, int XRight )
{
  mCellXLeft = XLeft;
  mCellXRight = XRight;
}

void AgendaItem::setCellY( int YTop, int YBottom )
{
  mCellYTop = YTop;
  mCellYBottom = YBottom;
}

void AgendaItem::setMultiItem( AgendaItem::QPtr first, AgendaItem::QPtr prev,
                               AgendaItem::QPtr next, AgendaItem::QPtr last )
{
  if ( !mMultiItemInfo ) {
    mMultiItemInfo = new MultiItemInfo;
  }
  mMultiItemInfo->mFirstMultiItem = first;
  mMultiItemInfo->mPrevMultiItem = prev;
  mMultiItemInfo->mNextMultiItem = next;
  mMultiItemInfo->mLastMultiItem = last;
}

bool AgendaItem::isMultiItem() const
{
  return mMultiItemInfo;
}

AgendaItem::QPtr AgendaItem::prependMoveItem( AgendaItem::QPtr e )
{
  if ( !e ) {
    return 0;
  }

  AgendaItem::QPtr first = 0, last = 0;
  if ( isMultiItem() ) {
    first = mMultiItemInfo->mFirstMultiItem;
    last = mMultiItemInfo->mLastMultiItem;
  }
  if ( !first ) {
    first = this;
  }
  if ( !last ) {
    last = this;
  }

  e->setMultiItem( 0, 0, first, last );
  first->setMultiItem( e, e, first->nextMultiItem(), first->lastMultiItem() );

  AgendaItem::QPtr tmp = first->nextMultiItem();
  while ( tmp ) {
    tmp->setMultiItem( e, tmp->prevMultiItem(), tmp->nextMultiItem(), tmp->lastMultiItem() );
    tmp = tmp->nextMultiItem();
  }

  if ( mStartMoveInfo && !e->moveInfo() ) {
    e->mStartMoveInfo=new MultiItemInfo( *mStartMoveInfo );
//    e->moveInfo()->mFirstMultiItem = moveInfo()->mFirstMultiItem;
//    e->moveInfo()->mLastMultiItem = moveInfo()->mLastMultiItem;
    e->moveInfo()->mPrevMultiItem = 0;
    e->moveInfo()->mNextMultiItem = first;
  }

  if ( first && first->moveInfo() ) {
    first->moveInfo()->mPrevMultiItem = e;
  }
  return e;
}

AgendaItem::QPtr AgendaItem::appendMoveItem( AgendaItem::QPtr e )
{
  if ( !e ) {
    return 0;
  }

  AgendaItem::QPtr first = 0, last = 0;
  if ( isMultiItem() ) {
    first = mMultiItemInfo->mFirstMultiItem;
    last = mMultiItemInfo->mLastMultiItem;
  }
  if ( !first ) {
    first = this;
  }
  if ( !last ) {
    last = this;
  }

  e->setMultiItem( first, last, 0, 0 );
  AgendaItem::QPtr tmp = first;

  while ( tmp ) {
    tmp->setMultiItem( tmp->firstMultiItem(), tmp->prevMultiItem(), tmp->nextMultiItem(), e );
    tmp = tmp->nextMultiItem();
  }
  last->setMultiItem( last->firstMultiItem(), last->prevMultiItem(), e, e );

  if ( mStartMoveInfo && !e->moveInfo() ) {
    e->mStartMoveInfo=new MultiItemInfo( *mStartMoveInfo );
//    e->moveInfo()->mFirstMultiItem = moveInfo()->mFirstMultiItem;
//    e->moveInfo()->mLastMultiItem = moveInfo()->mLastMultiItem;
    e->moveInfo()->mPrevMultiItem = last;
    e->moveInfo()->mNextMultiItem = 0;
  }
  if ( last && last->moveInfo() ) {
    last->moveInfo()->mNextMultiItem = e;
  }
  return e;
}

AgendaItem::QPtr AgendaItem::removeMoveItem( AgendaItem::QPtr e )
{
  if ( isMultiItem() ) {
    AgendaItem::QPtr first = mMultiItemInfo->mFirstMultiItem;
    AgendaItem::QPtr next, prev;
    AgendaItem::QPtr last = mMultiItemInfo->mLastMultiItem;
    if ( !first ) {
      first = this;
    }
    if ( !last ) {
      last = this;
    }
    if ( first == e ) {
      first = first->nextMultiItem();
      first->setMultiItem( 0, 0, first->nextMultiItem(), first->lastMultiItem() );
    }
    if ( last == e ) {
      last = last->prevMultiItem();
      last->setMultiItem( last->firstMultiItem(), last->prevMultiItem(), 0, 0 );
    }

    AgendaItem::QPtr tmp =  first;
    if ( first == last ) {
      delete mMultiItemInfo;
      tmp = 0;
      mMultiItemInfo = 0;
    }
    while ( tmp ) {
      next = tmp->nextMultiItem();
      prev = tmp->prevMultiItem();
      if ( e == next ) {
        next = next->nextMultiItem();
      }
      if ( e == prev ) {
        prev = prev->prevMultiItem();
      }
      tmp->setMultiItem( ( tmp == first ) ? 0 : first,
                         ( tmp == prev ) ? 0 : prev,
                         ( tmp == next ) ? 0 : next,
                         ( tmp == last ) ? 0 : last );
      tmp = tmp->nextMultiItem();
    }
  }

  return e;
}

void AgendaItem::startMove()
{
  AgendaItem::QPtr first = this;
  if ( isMultiItem() && mMultiItemInfo->mFirstMultiItem ) {
    first=mMultiItemInfo->mFirstMultiItem;
  }
  first->startMovePrivate();
}

void AgendaItem::startMovePrivate()
{
  mStartMoveInfo = new MultiItemInfo;
  mStartMoveInfo->mStartCellXLeft = mCellXLeft;
  mStartMoveInfo->mStartCellXRight = mCellXRight;
  mStartMoveInfo->mStartCellYTop = mCellYTop;
  mStartMoveInfo->mStartCellYBottom = mCellYBottom;
  if ( mMultiItemInfo ) {
    mStartMoveInfo->mFirstMultiItem = mMultiItemInfo->mFirstMultiItem;
    mStartMoveInfo->mLastMultiItem = mMultiItemInfo->mLastMultiItem;
    mStartMoveInfo->mPrevMultiItem = mMultiItemInfo->mPrevMultiItem;
    mStartMoveInfo->mNextMultiItem = mMultiItemInfo->mNextMultiItem;
  } else {
    mStartMoveInfo->mFirstMultiItem = 0;
    mStartMoveInfo->mLastMultiItem = 0;
    mStartMoveInfo->mPrevMultiItem = 0;
    mStartMoveInfo->mNextMultiItem = 0;
  }
  if ( isMultiItem() && mMultiItemInfo->mNextMultiItem ) {
    mMultiItemInfo->mNextMultiItem->startMovePrivate();
  }
}

void AgendaItem::resetMove()
{
  if ( mStartMoveInfo ) {
    if ( mStartMoveInfo->mFirstMultiItem ) {
      mStartMoveInfo->mFirstMultiItem->resetMovePrivate();
    } else {
      resetMovePrivate();
    }
  }
}

void AgendaItem::resetMovePrivate()
{
  if ( mStartMoveInfo ) {
    mCellXLeft = mStartMoveInfo->mStartCellXLeft;
    mCellXRight = mStartMoveInfo->mStartCellXRight;
    mCellYTop = mStartMoveInfo->mStartCellYTop;
    mCellYBottom = mStartMoveInfo->mStartCellYBottom;

    // if we don't have mMultiItemInfo, the item didn't span two days before,
    // and wasn't moved over midnight, either, so we don't have to reset
    // anything. Otherwise, restore from mMoveItemInfo
    if ( mMultiItemInfo ) {
      // It was already a multi-day info
      mMultiItemInfo->mFirstMultiItem = mStartMoveInfo->mFirstMultiItem;
      mMultiItemInfo->mPrevMultiItem = mStartMoveInfo->mPrevMultiItem;
      mMultiItemInfo->mNextMultiItem = mStartMoveInfo->mNextMultiItem;
      mMultiItemInfo->mLastMultiItem = mStartMoveInfo->mLastMultiItem;

      if ( !mStartMoveInfo->mFirstMultiItem ) {
        // This was the first multi-item when the move started, delete all previous
        AgendaItem::QPtr toDel = mStartMoveInfo->mPrevMultiItem;
        AgendaItem::QPtr nowDel = 0;
        while ( toDel ) {
          nowDel = toDel;
          if ( nowDel->moveInfo() ) {
            toDel = nowDel->moveInfo()->mPrevMultiItem;
          }
          emit removeAgendaItem( nowDel );
        }
        mMultiItemInfo->mFirstMultiItem = 0;
        mMultiItemInfo->mPrevMultiItem = 0;
      }
      if ( !mStartMoveInfo->mLastMultiItem ) {
        // This was the last multi-item when the move started, delete all next
        AgendaItem::QPtr toDel = mStartMoveInfo->mNextMultiItem;
        AgendaItem::QPtr nowDel = 0;
        while ( toDel ) {
          nowDel = toDel;
          if ( nowDel->moveInfo() ) {
            toDel=nowDel->moveInfo()->mNextMultiItem;
          }
          emit removeAgendaItem( nowDel );
        }
        mMultiItemInfo->mLastMultiItem = 0;
        mMultiItemInfo->mNextMultiItem = 0;
      }

      if ( mStartMoveInfo->mFirstMultiItem == 0 && mStartMoveInfo->mLastMultiItem == 0 ) {
        // it was a single-day event before we started the move.
        delete mMultiItemInfo;
        mMultiItemInfo = 0;
      }
    }
    delete mStartMoveInfo;
    mStartMoveInfo = 0;
  }
  emit showAgendaItem( this );
  if ( nextMultiItem() ) {
    nextMultiItem()->resetMovePrivate();
  }
}

void AgendaItem::endMove()
{
  AgendaItem::QPtr first = firstMultiItem();
  if ( !first ) {
    first = this;
  }
  first->endMovePrivate();
}

void AgendaItem::endMovePrivate()
{
  if ( mStartMoveInfo ) {
    // if first, delete all previous
    if ( !firstMultiItem() || firstMultiItem() == this ) {
      AgendaItem::QPtr toDel = mStartMoveInfo->mPrevMultiItem;
      AgendaItem::QPtr nowDel = 0;
      while ( toDel ) {
        nowDel = toDel;
        if ( nowDel->moveInfo() ) {
          toDel=nowDel->moveInfo()->mPrevMultiItem;
        }
        emit removeAgendaItem( nowDel );
      }
    }
    // if last, delete all next
    if ( !lastMultiItem() || lastMultiItem() == this ) {
      AgendaItem::QPtr toDel=mStartMoveInfo->mNextMultiItem;
      AgendaItem::QPtr nowDel = 0;
      while ( toDel ) {
        nowDel = toDel;
        if ( nowDel->moveInfo() ) {
          toDel=nowDel->moveInfo()->mNextMultiItem;
        }
        emit removeAgendaItem( nowDel );
      }
    }
    // also delete the moving info
    delete mStartMoveInfo;
    mStartMoveInfo = 0;
    if ( nextMultiItem() ) {
      nextMultiItem()->endMovePrivate();
    }
  }
}

void AgendaItem::moveRelative( int dx, int dy )
{
  int newXLeft = cellXLeft() + dx;
  int newXRight = cellXRight() + dx;
  int newYTop = cellYTop() + dy;
  int newYBottom = cellYBottom() + dy;
  setCellXY( newXLeft, newYTop, newYBottom );
  setCellXRight( newXRight );
}

void AgendaItem::expandTop( int dy, const bool allowOverLimit )
{
  int newYTop = cellYTop() + dy;
  int newYBottom = cellYBottom();
  if ( newYTop > newYBottom && !allowOverLimit ) {
    newYTop = newYBottom;
  }
  setCellY( newYTop, newYBottom );
}

void AgendaItem::expandBottom( int dy )
{
  int newYTop = cellYTop();
  int newYBottom = cellYBottom() + dy;
  if ( newYBottom < newYTop ) {
    newYBottom = newYTop;
  }
  setCellY( newYTop, newYBottom );
}

void AgendaItem::expandLeft( int dx )
{
  int newXLeft = cellXLeft() + dx;
  int newXRight = cellXRight();
  if ( newXLeft > newXRight ) {
    newXLeft = newXRight;
  }
  setCellX( newXLeft, newXRight );
}

void AgendaItem::expandRight( int dx )
{
  int newXLeft = cellXLeft();
  int newXRight = cellXRight() + dx;
  if ( newXRight < newXLeft ) {
    newXRight = newXLeft;
  }
  setCellX( newXLeft, newXRight );
}

void AgendaItem::dragEnterEvent( QDragEnterEvent *e )
{
#ifndef KORG_NODND
  const QMimeData *md = e->mimeData();
  if ( KCalUtils::ICalDrag::canDecode( md ) || KCalUtils::VCalDrag::canDecode( md ) ) {
    // TODO: Allow dragging events/todos onto other events to create a relation
    e->ignore();
    return;
  }
  if ( KContacts::VCardDrag::canDecode( md ) || md->hasText() ) {
    e->accept();
  } else {
    e->ignore();
  }
#else
  Q_UNUSED( e );
#endif
}

void AgendaItem::addAttendee( const QString &newAttendee )
{
  if ( !mValid ) {
    return;
  }

  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
  QString name, email;
  KPIMUtils::extractEmailAddressAndName( newAttendee, email, name );
  if ( !( name.isEmpty() && email.isEmpty() ) ) {
    incidence->addAttendee( KCalCore::Attendee::Ptr( new KCalCore::Attendee( name, email ) ) );
    KMessageBox::information(
      this,
      i18n( "Attendee \"%1\" added to the calendar item \"%2\"",
            KPIMUtils::normalizedAddress( name, email, QString() ), text() ),
      i18n( "Attendee added" ), QLatin1String("AttendeeDroppedAdded") );
  }
}

void AgendaItem::dropEvent( QDropEvent *e )
{
  // TODO: Organize this better: First check for attachment
  // (not only file, also any other url!), then if it's a vcard,
  // otherwise check for attendees, then if the data is binary,
  // add a binary attachment.
#ifndef KORG_NODND
  if ( !mValid ) {
    return;
  }

  const QMimeData *md = e->mimeData();

  bool decoded = md->hasText();
  QString text = md->text();
  if ( decoded && text.startsWith( QLatin1String( "file:" ) ) ) {
    const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
    incidence->addAttachment( KCalCore::Attachment::Ptr( new KCalCore::Attachment( text ) ) );
    return;
  }

  KContacts::Addressee::List list;

  if ( KContacts::VCardDrag::fromMimeData( md, list ) ) {
    Q_FOREACH ( const KContacts::Addressee &addressee, list ) {
      QString em( addressee.fullEmail() );
      if ( em.isEmpty() ) {
        em = addressee.realName();
      }
      addAttendee( em );
    }
  }
#else
  Q_UNUSED( e );
#endif // KORG_NODND
}

QList<AgendaItem::QPtr> &AgendaItem::conflictItems()
{
  return mConflictItems;
}

void AgendaItem::setConflictItems( QList<AgendaItem::QPtr> ci )
{
  mConflictItems = ci;
  QList<AgendaItem::QPtr>::iterator it;
  for ( it = mConflictItems.begin(); it != mConflictItems.end(); ++it ) {
    (*it)->addConflictItem( this );
  }
}

void AgendaItem::addConflictItem( AgendaItem::QPtr ci )
{
  if ( !mConflictItems.contains( ci ) ) {
    mConflictItems.append( ci );
  }
}

QString AgendaItem::label() const
{
  return mLabelText;
}

bool AgendaItem::overlaps( CellItem *o ) const
{
  AgendaItem::QPtr other = static_cast<AgendaItem*>( o );

  if ( cellXLeft() <= other->cellXRight() && cellXRight() >= other->cellXLeft() ) {
    if ( ( cellYTop() <= other->cellYBottom() ) && ( cellYBottom() >= other->cellYTop() ) ) {
      return true;
    }
  }

  return false;
}

static void conditionalPaint( QPainter *p, bool condition, int &x, int y,
                              int ft, const QPixmap &pxmp )
{
  if ( condition ) {
    p->drawPixmap( x, y, pxmp );
    x += pxmp.width() + ft;
   }
}

void AgendaItem::paintIcon( QPainter *p, int &x, int y, int ft )
{
  QString iconName;
  Incidence::Ptr incidence = mIncidence.payload<KCalCore::Incidence::Ptr>();
  if ( incidence->customProperty( "KABC", "ANNIVERSARY" ) == QLatin1String("YES") ) {
    mSpecialEvent = true;
    iconName =  QLatin1String("view-calendar-wedding-anniversary");
  } else if ( incidence->customProperty( "KABC", "BIRTHDAY" ) == QLatin1String("YES") ) {
    mSpecialEvent = true;
    // We don't draw icon. The icon is drawn already, because it's the Akonadi::Collection's icon
  }

  conditionalPaint( p, !iconName.isEmpty(), x, y, ft, cachedSmallIcon( iconName ) );
}

void AgendaItem::paintIcons( QPainter *p, int &x, int y, int ft )
{
  if ( !mEventView->preferences()->enableAgendaItemIcons() ) {
    return;
  }

  paintIcon( p, x, y, ft );

  QSet<EventView::ItemIcon> icons = mEventView->preferences()->agendaViewIcons();

  if ( icons.contains( EventViews::EventView::CalendarCustomIcon ) ) {
    const QString iconName = mEventView->iconForItem( mIncidence );
    if ( !iconName.isEmpty() && iconName != QLatin1String("view-calendar") && iconName != QLatin1String("office-calendar") ) {
      conditionalPaint( p, true, x, y, ft, SmallIcon( iconName ) );
    }
  }

  Incidence::Ptr incidence = mIncidence.payload<KCalCore::Incidence::Ptr>();
  const bool isTodo = incidence && incidence->type() == Incidence::TypeTodo;

  if ( isTodo && icons.contains( EventViews::EventView::TaskIcon ) ) {
    const QString iconName = incidence->iconName( mOccurrenceDateTime.toTimeSpec( incidence->dtStart().timeSpec() ) );
    conditionalPaint( p, !mSpecialEvent, x, y, ft, SmallIcon( iconName ) );
  }

  if ( icons.contains( EventView::RecurringIcon ) ) {
    conditionalPaint( p, mIconRecur && !mSpecialEvent, x, y, ft, *recurPxmp );
  }

  if ( icons.contains( EventView::ReminderIcon ) ) {
    conditionalPaint( p, mIconAlarm && !mSpecialEvent, x, y, ft, *alarmPxmp );
  }

  if ( icons.contains( EventView::ReadOnlyIcon ) ) {
    conditionalPaint( p, mIconReadonly && !mSpecialEvent, x, y, ft, *readonlyPxmp );
  }

  if ( icons.contains( EventView::ReplyIcon ) ) {
    conditionalPaint( p, mIconReply, x, y, ft, *replyPxmp );
  }

  if ( icons.contains( EventView::AttendingIcon ) ) {
    conditionalPaint( p, mIconGroup, x, y, ft, *groupPxmp );
  }

  if ( icons.contains( EventView::TentativeIcon ) ) {
    conditionalPaint( p, mIconGroupTent, x, y, ft, *groupPxmpTent );
  }

  if ( icons.contains( EventView::OrganizerIcon ) ) {
    conditionalPaint( p, mIconOrganizer, x, y, ft, *organizerPxmp );
  }
}

void AgendaItem::paintEvent( QPaintEvent *ev )
{
  if ( !mValid ) {
    return;
  }

  QRect visRect = visibleRegion().boundingRect();
  // when scrolling horizontally in the side-by-side view, the repainted area is clipped
  // to the newly visible area, which is a problem since the content changes when visRect
  // changes, so repaint the full item in that case
  if ( ev->rect() != visRect && visRect.isValid() && ev->rect().isValid() ) {
    update( visRect );
    return;
  }

  QPainter p( this );
  p.setRenderHint( QPainter::Antialiasing );
  const int fmargin = 0; // frame margin
  const int ft = 1; // frame thickness for layout, see drawRoundedRect(),
                    // keep multiple of 2
  const int margin = 5 + ft + fmargin ; // frame + space between frame and content

  // General idea is to always show the icons (even in the all-day events).
  // This creates a consistent feeling for the user when the view mode
  // changes and therefore the available width changes.
  // Also look at #17984

  if ( !alarmPxmp ) {
    alarmPxmp     = new QPixmap( SmallIcon( QLatin1String("task-reminder") ) );
    recurPxmp     = new QPixmap( SmallIcon( QLatin1String("appointment-recurring") ) );
    readonlyPxmp  = new QPixmap( SmallIcon( QLatin1String("object-locked") ) );
    replyPxmp     = new QPixmap( SmallIcon( QLatin1String("mail-reply-sender") ) );
    groupPxmp     = new QPixmap( SmallIcon( QLatin1String("meeting-attending") ) );
    groupPxmpTent = new QPixmap( SmallIcon( QLatin1String("meeting-attending-tentative") ) );
    organizerPxmp = new QPixmap( SmallIcon( QLatin1String("meeting-organizer") ) );
  }

  QColor bgColor;

  if ( CalendarSupport::hasTodo( mIncidence ) &&
       !mEventView->preferences()->todosUseCategoryColors() ) {
    Todo::Ptr todo = CalendarSupport::todo( mIncidence );
    Q_ASSERT( todo );
    const QDate dueDate =
      todo->dtDue().toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date();
    const QDate today =
      KDateTime::currentDateTime( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date();
    const QDate occurrenceDate = mOccurrenceDateTime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date();
    if ( todo->isOverdue() && today >= occurrenceDate ) {
      bgColor = mEventView->preferences()->todoOverdueColor();
    } else if ( dueDate == today && dueDate == occurrenceDate ) {
      bgColor = mEventView->preferences()->todoDueTodayColor();
    }
  }

  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
  Q_ASSERT( incidence );
  QColor categoryColor;
  const QStringList categories = incidence->categories();
  QString cat;
  if ( !categories.isEmpty() ) {
    cat = categories.first();
  }

  categoryColor = cat.isEmpty() ? CalendarSupport::KCalPrefs::instance()->unsetCategoryColor() :
                                  CalendarSupport::KCalPrefs::instance()->categoryColor( cat );

  QColor resourceColor = mResourceColor;
  if ( !resourceColor.isValid() ) {
    resourceColor = categoryColor;
  }

  QColor frameColor;
  // TODO PrefsBase enums should probably be redefined in Prefs
  if ( mEventView->preferences()->agendaViewColors() == PrefsBase::ResourceOnly ||
       mEventView->preferences()->agendaViewColors() == PrefsBase::CategoryInsideResourceOutside ) {
    frameColor = bgColor.isValid() ? bgColor : resourceColor;
  } else {
    frameColor = bgColor.isValid() ? bgColor : categoryColor;
  }

  if ( !bgColor.isValid() ) {
    if ( mEventView->preferences()->agendaViewColors() == PrefsBase::ResourceOnly ||
         mEventView->preferences()->agendaViewColors() == PrefsBase::ResourceInsideCategoryOutside ) {
      bgColor = resourceColor;
    } else {
      bgColor = categoryColor;
    }
  }

  if ( cat.isEmpty() &&
       mEventView->preferences()->agendaViewColors() == PrefsBase::ResourceInsideCategoryOutside ) {
    frameColor = bgColor;
  }

  if ( cat.isEmpty() &&
       mEventView->preferences()->agendaViewColors() == PrefsBase::CategoryInsideResourceOutside ) {
    bgColor = frameColor;
  }

  frameColor = EventView::itemFrameColor( frameColor, mSelected );

  if ( !CalendarSupport::KCalPrefs::instance()->hasCategoryColor( cat ) ) {
    categoryColor = resourceColor;
  }

  if ( !bgColor.isValid() ) {
    bgColor = categoryColor;
  }

  if ( mSelected ) {
    bgColor = bgColor.light( EventView::BRIGHTNESS_FACTOR );
  }

  const QColor textColor = EventViews::getTextColor( bgColor );
  p.setPen( textColor );

  p.setFont( mEventView->preferences()->agendaViewFont() );
  QFontMetrics fm = p.fontMetrics();

  const int singleLineHeight = fm.boundingRect( mLabelText ).height();

  const bool roundTop = !prevMultiItem();
  const bool roundBottom = !nextMultiItem();

  drawRoundedRect( &p, QRect( fmargin, fmargin, width() - fmargin * 2, height() - fmargin * 2 ),
                   mSelected, bgColor, true, ft, roundTop, roundBottom );

  // calculate the height of the full version (case 4) to test whether it is
  // possible

  QString shortH;
  QString longH;
  if ( !isMultiItem() ) {
    shortH = KLocale::global()->formatTime(incidence->dateTime( KCalCore::Incidence::RoleDisplayStart ).
             toTimeSpec( mEventView->preferences()->timeSpec() ).time() );

    if ( CalendarSupport::hasEvent( mIncidence ) ) {
      longH = i18n( "%1 - %2",
                    shortH,
                    KLocale::global()->formatTime(
                      incidence->dateTime( KCalCore::Incidence::RoleEnd ).toTimeSpec(
                        mEventView->preferences()->timeSpec() ).time() ) );
    } else {
      longH = shortH;
    }
  } else if ( !mMultiItemInfo->mFirstMultiItem ) {
    shortH = KLocale::global()->formatTime(
      incidence->dtStart().toTimeSpec( mEventView->preferences()->timeSpec() ).time() );
    longH = shortH;
  } else {
    shortH = KLocale::global()->formatTime(
      incidence->dateTime( KCalCore::Incidence::RoleEnd ).toTimeSpec(
        mEventView->preferences()->timeSpec() ).time() );
    longH = i18n( "- %1", shortH );
  }

  KWordWrap ww = KWordWrap::formatText(
    fm, QRect( 0, 0, width() - ( 2 * margin ), -1 ), 0, mLabelText );
  int th = ww.boundingRect().height();

  int hlHeight = qMax( fm.boundingRect( longH ).height(),
                       qMax( alarmPxmp->height(),
                            qMax( recurPxmp->height(),
                                 qMax( readonlyPxmp->height(),
                                      qMax( replyPxmp->height(),
                                           qMax( groupPxmp->height(),
                                                 organizerPxmp->height() ) ) ) ) ) );

  const bool completelyRenderable = th < ( height() - 2 * ft - 2 - hlHeight );

  // case 1: do not draw text when not even a single line fits
  // Don't do this any more, always try to print out the text.
  // Even if it's just a few pixel, one can still guess the whole
  // text from just four pixels' height!
  if ( //( singleLineHeight > height() - 4 ) ||
       ( width() < 16 ) ) {
    int x = qRound( ( width() - 16 ) / 2.0 );
    paintIcon( &p, x/*by-ref*/, margin, ft );
    return;
  }

  // case 2: draw a single line when no more space
  if ( ( 2 * singleLineHeight ) > ( height() - 2 * margin ) ) {
    int x = margin, txtWidth;

    if ( incidence->allDay() ) {
      x += visRect.left();
      const int y =  qRound( ( height() - 16 ) / 2.0 );
      paintIcons( &p, x, y, ft );
      txtWidth = visRect.right() - margin - x;
    } else {
      const int y =  qRound( ( height() - 16 ) / 2.0 );
      paintIcons( &p, x, y, ft );
      txtWidth = width() - margin - x;
    }

    const int y = ( ( height() - singleLineHeight ) / 2 ) + fm.ascent();
    KWordWrap::drawFadeoutText( &p, x, y, txtWidth, mLabelText );
    return;
  }

  // case 3: enough for 2-5 lines, but not for the header.
  //         Also used for the middle days in multi-events
  if ( ( ( !completelyRenderable ) &&
         ( ( height() - ( 2 * margin ) ) <= ( 5 * singleLineHeight ) ) ) ||
       ( isMultiItem() && mMultiItemInfo->mNextMultiItem && mMultiItemInfo->mFirstMultiItem ) ) {
    int x = margin, txtWidth;

    if ( incidence->allDay() ) {
      x += visRect.left();
      paintIcons( &p, x, margin, ft );
      txtWidth = visRect.right() - margin - x;
    } else {
      paintIcons( &p, x, margin, ft );
      txtWidth = width() - margin - x;
    }

    ww = KWordWrap::formatText(
      fm, QRect( 0, 0, txtWidth, ( height() - ( 2 * margin ) ) ), 0, mLabelText );

    ww.drawText( &p, x, margin, Qt::AlignHCenter | KWordWrap::FadeOut );
    return;
  }

  // case 4: paint everything, with header:
  // consists of (vertically) ft + headline&icons + ft + text + margin
  int y = 2 * ft + hlHeight;
  if ( completelyRenderable ) {
    y += ( height() - ( 2 * ft ) - margin - hlHeight - th ) / 2;
  }

  int x = margin, txtWidth, hTxtWidth, eventX;

  if ( incidence->allDay() ) {
    shortH.clear();
    longH.clear();

    if ( const KCalCore::Event::Ptr event = CalendarSupport::event( mIncidence ) ) {
      if ( event->isMultiDay( mEventView->preferences()->timeSpec() ) ) {
        // multi-day, all-day event
        shortH =
          i18n( "%1 - %2",
                KLocale::global()->formatDate(
                  incidence->dtStart().toTimeSpec( mEventView->preferences()->timeSpec() ).date() ),
                KLocale::global()->formatDate(
                  incidence->dateTime( KCalCore::Incidence::RoleEnd ).toTimeSpec(
                    mEventView->preferences()->timeSpec() ).date() ) );
        longH = shortH;

        // paint headline
        drawRoundedRect(
          &p,
          QRect( fmargin, fmargin, width() - fmargin * 2, - fmargin * 2 + margin + hlHeight ),
          mSelected, frameColor, false, ft, roundTop, false );
      } else {
        // single-day, all-day event

        // paint headline
        drawRoundedRect(
          &p,
          QRect( fmargin, fmargin, width() - fmargin * 2, - fmargin * 2 + margin + hlHeight ),
          mSelected, frameColor, false, ft, roundTop, false );
      }
    } else {
      // to-do

      // paint headline
      drawRoundedRect(
        &p,
        QRect( fmargin, fmargin, width() - fmargin * 2, - fmargin * 2 + margin + hlHeight ),
        mSelected, frameColor, false, ft, roundTop, false );
    }

    x += visRect.left();
    eventX = x;
    txtWidth = visRect.right() - margin - x;
    paintIcons( &p, x, margin / 2, ft );
    hTxtWidth = visRect.right() - margin - x;
  } else {
    // paint headline
     drawRoundedRect(
       &p,
       QRect( fmargin, fmargin, width() - fmargin * 2, - fmargin * 2 + margin + hlHeight ),
       mSelected, frameColor, false, ft, roundTop, false );

    txtWidth = width() - margin - x;
    eventX = x;
    paintIcons( &p, x, margin / 2, ft );
    hTxtWidth = width() - margin - x;
  }

  QString headline;
  int hw = fm.boundingRect( longH ).width();
  if ( hw > hTxtWidth ) {
    headline = shortH;
    hw = fm.boundingRect( shortH ).width();
    if ( hw < txtWidth ) {
      x += ( hTxtWidth - hw ) / 2;
    }
  } else {
    headline = longH;
    x += ( hTxtWidth - hw ) / 2;
  }
  p.setBackground( QBrush( frameColor ) );
  p.setPen( EventViews::getTextColor( frameColor ) );
  KWordWrap::drawFadeoutText( &p, x, ( margin + hlHeight + fm.ascent() ) / 2 - 2,
                              hTxtWidth, headline );

  // draw event text
  ww = KWordWrap::formatText(
    fm, QRect( 0, 0, txtWidth, height() - margin - y ), 0, mLabelText );

  p.setBackground( QBrush( bgColor ) );
  p.setPen( textColor );
  QString ws = ww.wrappedString();
  if ( ws.left( ws.length()-1 ).indexOf( QLatin1Char('\n') ) >= 0 ) {
    ww.drawText( &p, eventX, y, Qt::AlignLeft | KWordWrap::FadeOut );
  } else {
    ww.drawText( &p, eventX + ( txtWidth - ww.boundingRect().width() - 2 * margin ) / 2, y,
                  Qt::AlignHCenter | KWordWrap::FadeOut );
  }

}

void AgendaItem::drawRoundedRect( QPainter *p, const QRect &rect,
                                    bool selected, const QColor &bgColor,
                                    bool frame, int ft, bool roundTop,
                                    bool roundBottom )
{
  Q_UNUSED( ft );
  if ( !mValid ) {
    return;
  }

  QRect r = rect;
  r.adjust( 0, 0, 1, 1 );

  p->save();

  QPainterPath path;

  bool shrinkWidth = r.width() < 16;
  bool shrinkHeight = r.height() < 16;

  qreal rnd = 2.1;
  int sw = shrinkWidth ? 10 : 11;
  int sh = shrinkHeight ? 10 : 11;
  QRectF tr( r.x() + r.width() - sw - rnd, r.y() + rnd, sw, sh );
  QRectF tl( r.x() + rnd, r.y() + rnd, sw, sh );
  QRectF bl( r.x() + rnd, r.y() + r.height() - sh - 1 - rnd, sw, sh );
  QRectF br( r.x() + r.width() - sw - rnd, r.y() + r.height() - sh - 1 - rnd, sw, sh );

  if( roundTop ) {
    path.moveTo( tr.topRight() );
    path.arcTo( tr, 0.0, 90.0 );
    path.lineTo( tl.topRight() );
    path.arcTo( tl, 90.0, 90.0 );
  } else {
    path.moveTo( tr.topRight() );
    path.lineTo( tl.topLeft() );
  }

  if( roundBottom ) {
    path.lineTo( bl.topLeft() );
    path.arcTo( bl, 180.0, 90.0 );
    path.lineTo( br.bottomLeft() );
    path.arcTo( br, 270.0, 90.0 );
  } else {
    path.lineTo( bl.bottomLeft() );
    path.lineTo( br.bottomRight() );
  }
  path.closeSubpath();

  // header
  if ( !frame ) {
    QLinearGradient gradient( QPointF( r.x(), r.y() ), QPointF( r.x(), r.height() ) );

    if ( selected ) {
      QColor top = bgColor.dark( 250 );
      top.setAlpha( 40 );
      gradient.setColorAt( 0, top );
      gradient.setColorAt( 1, QColor( 255, 255, 255, 30 ) );
    } else {
      gradient.setColorAt( 0, QColor( 255, 255, 255, 90 ) );
      gradient.setColorAt( 1, QColor( 0, 0, 0, 10 ) );
    }

    p->setBrush( bgColor );
    p->setPen( Qt::NoPen );
    p->drawPath( path );

    p->setBrush( gradient );
    p->setPen( Qt::NoPen );
    p->drawPath( path );

    QPixmap separator;
    QString key( QLatin1String("ko_hsep") );
    if ( !QPixmapCache::find( key, separator ) ) {
      separator = QPixmap( QLatin1String(":/headerSeparator.png") );
      QPixmapCache::insert( key, separator );
    }
    p->fillRect( QRect( r.x() + 3, r.y() + r.height() - 2, r.x() + r.width() - 4, 2 ),
                 QBrush( separator ) );

    p->restore();
    return;
  }

  QLinearGradient gradient( QPointF( r.x(), r.y() ), QPointF( r.x(), r.height() ) );

  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
  Q_ASSERT( incidence );

  if ( r.height() > 50 ) {
    if ( incidence->allDay() &&
         incidence->dtStart() == incidence->dateTime( KCalCore::Incidence::RoleEnd ) &&
         CalendarSupport::hasEvent( mIncidence ) ) {
      gradient.setColorAt( 0, bgColor.light( 130 ) );
      qreal t = 1.0 - ( r.height() - 18.0 ) / r.height();
      gradient.setColorAt( t, bgColor.light( 115 ) );
      qreal b = ( r.height() - 20.0 ) / r.height();
      gradient.setColorAt( b, bgColor );
    } else {
      gradient.setColorAt( 0, bgColor.light( 115 ) );
      qreal b = ( r.height() - 20.0 ) / r.height();
      gradient.setColorAt( b, bgColor );
    }
    gradient.setColorAt( 1, bgColor.dark( 110 ) );
  } else {
    if ( incidence->allDay() &&
         incidence->dtStart() == incidence->dateTime( KCalCore::Incidence::RoleEnd ) &&
         !CalendarSupport::hasTodo( mIncidence ) ) {
      gradient.setColorAt( 0, bgColor.light( 130 ) );
      gradient.setColorAt( 0.35, bgColor.light( 115 ) );
      gradient.setColorAt( 0.65, bgColor );
    } else {
      gradient.setColorAt( 0, bgColor.light( 115 ) );
      gradient.setColorAt( 0.65, bgColor );
    }
    gradient.setColorAt( 1, bgColor.dark( 110 ) );
  }

  p->setBrush( gradient );
  p->setPen( Qt::NoPen );
  p->drawPath( path );

  p->setRenderHint( QPainter::Antialiasing, false );

  if ( r.width() - 16 > 0 ) {
    QPixmap topLines;
    QString key( QLatin1String("ko_t") );
    if ( !QPixmapCache::find( key, topLines ) ) {
      topLines = QPixmap( QLatin1String(":/topLines.png") );
      QPixmapCache::insert( key, topLines );
    }
    p->setBrushOrigin( r.x() + 8, r.y() );
    p->fillRect( QRect( r.x() + 8, r.y(), r.width() - 16, 5 ),
                 QBrush( topLines ) );

    QPixmap bottomLines;
    key = QLatin1String("ko_b");
    if ( !QPixmapCache::find( key, bottomLines ) ) {
      bottomLines = QPixmap( QLatin1String(":/bottomLines.png") );
      QPixmapCache::insert( key, bottomLines );
    }
    p->setBrushOrigin( r.x() + 8, r.y() + r.height() - 6 );
    p->fillRect( QRect( r.x() + 8, r.y() + r.height() - 6, r.width() - 16, 6 ),
                 QBrush( bottomLines ) );

  }

  if ( r.height() - 16 > 0 ) {

    QPixmap leftLines;
    QString key( QLatin1String("ko_l") );
    if ( !QPixmapCache::find( key, leftLines ) ) {
      leftLines = QPixmap( QLatin1String(":/leftLines.png") );
      QPixmapCache::insert( key, leftLines );
    }
    p->setBrushOrigin( r.x(), r.y() + 8 );
    p->fillRect( QRect( r.x(), r.y() + 8, 5, r.height() - 16 ),
                 QBrush( leftLines ) );

    QPixmap rightLines;
    key = QLatin1String( "ko_r" );
    if ( !QPixmapCache::find( key, rightLines ) ) {
      rightLines = QPixmap( QLatin1String(":/rightLines.png") );
      QPixmapCache::insert( key, rightLines );
    }
    p->setBrushOrigin( r.x() + r.width() - 5, r.y() + 8 );
    p->fillRect( QRect( r.x() + r.width() - 5, r.y() + 8, 5, r.height() - 16 ),
                 QBrush( rightLines ) );
  }

  // don't overlap the edges
  int lw = shrinkWidth ? r.width() / 2 : 8;
  int rw = shrinkWidth ? r.width() - lw : 8;
  int th = shrinkHeight ? r.height() / 2 : 8;
  int bh = shrinkHeight ? r.height() - th : 8;

  // keep the bottom round for items which ending at 00:15
  if( shrinkHeight && !roundTop && roundBottom && r.height() > 3 ) {
    bh += th - 3;
    th = 3;
  }

  QPixmap topLeft;
  QString key = roundTop ? QLatin1String( "ko_tl" ) : QLatin1String( "ko_rtl" );
  if ( !QPixmapCache::find( key, topLeft ) ) {
    topLeft = roundTop ? QPixmap( QLatin1String(":/roundTopLeft.png") ) : QPixmap( QLatin1String(":/rectangularTopLeft.png") );
    QPixmapCache::insert( key, topLeft );
  }
  p->drawPixmap( r.x(), r.y(), topLeft, 0, 0, lw, th );

  QPixmap topRight;
  key = roundTop ? QLatin1String( "ko_tr" ) : QLatin1String( "ko_rtr" );
  if ( !QPixmapCache::find( key, topRight ) ) {
    topRight = roundTop ? QPixmap( QLatin1String(":/roundTopRight.png") ) : QPixmap( QLatin1String(":/rectangularTopRight.png") );
    QPixmapCache::insert( key, topRight );
  }
  p->drawPixmap( r.x() + r.width() - rw, r.y(), topRight, 8 - rw, 0, rw, th );

  QPixmap bottomLeft;
  key = roundBottom ? QLatin1String( "ko_bl" ) : QLatin1String( "ko_rbl" );
  if ( !QPixmapCache::find( key, bottomLeft ) ) {
    bottomLeft = roundBottom ? QPixmap( QLatin1String(":/roundBottomLeft.png") ) :
                 QPixmap( QLatin1String(":/rectangularBottomLeft.png") );
    QPixmapCache::insert( key, bottomLeft );
  }
  p->drawPixmap( r.x(), r.y() + r.height() - bh, bottomLeft, 0, 8 - bh, lw, bh );

  QPixmap bottomRight;
  key = roundBottom ? QLatin1String( "ko_br" ) : QLatin1String( "ko_rbr" );
  if ( !QPixmapCache::find( key, bottomRight ) ) {
    bottomRight = roundBottom ? QPixmap( QLatin1String(":/roundBottomRight.png") ) :
                  QPixmap( QLatin1String(":/rectangularBottomRight.png") );
    QPixmapCache::insert( key, bottomRight );
  }
  p->drawPixmap( r.x() + r.width() - rw, r.y() + r.height() - bh, bottomRight,
                 8 - rw, 8 - bh, rw, 8 );

  p->restore();
}

bool AgendaItem::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::Paint ) {
    return mValid;
  } else {
    // standard event processing
    return QObject::eventFilter( obj, event );
  }
}

bool AgendaItem::event( QEvent *event )
{
  if ( event->type() == QEvent::ToolTip ) {
    if( !mEventView->preferences()->enableToolTips() ) {
      return true;
    } else if ( mValid ) {
      QHelpEvent *helpEvent = static_cast<QHelpEvent*>( event );
      QToolTip::showText(
        helpEvent->globalPos(),
        KCalUtils::IncidenceFormatter::toolTipStr(
          CalendarSupport::displayName( mCalendar.data(), mIncidence.parentCollection() ),
          CalendarSupport::incidence( mIncidence ),
          mOccurrenceDateTime.toTimeSpec( mEventView->preferences()->timeSpec() ).date(), true, mEventView->preferences()->timeSpec() ),
        this );
    }
  }
  return QWidget::event( event );
}

