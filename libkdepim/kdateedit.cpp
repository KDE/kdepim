/*
    This file is part of libkdepim.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 David Jarvie <software@astrojar.org.uk>
    Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqapplication.h>
#include <tqlineedit.h>
#include <tqlistbox.h>
#include <tqvalidator.h>

#include <kcalendarsystem.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include "kdateedit.h"

class DateValidator : public QValidator
{
  public:
    DateValidator( const TQStringList &keywords, TQWidget* parent, const char* name = 0 )
      : TQValidator( parent, name ), mKeywords( keywords )
    {}

    virtual State validate( TQString &str, int& ) const
    {
      int length = str.length();

      // empty string is intermediate so one can clear the edit line and start from scratch
      if ( length <= 0 )
        return Intermediate;

      if ( mKeywords.contains( str.lower() ) )
        return Acceptable;

      bool ok = false;
      KGlobal::locale()->readDate( str, &ok );
      if ( ok )
        return Acceptable;
      else
        return Intermediate;
    }

  private:
    TQStringList mKeywords;
};

KDateEdit::KDateEdit( TQWidget *parent, const char *name )
  : TQComboBox( true, parent, name ),
    mReadOnly( false ),
    mDiscardNextMousePress( false )
{
  // need at least one entry for popup to work
  setMaxCount( 1 );

  mDate = TQDate::currentDate();
  TQString today = KGlobal::locale()->formatDate( mDate, true );

  insertItem( today );
  setCurrentItem( 0 );
  changeItem( today, 0 );
  setMinimumSize( sizeHint() );

  connect( lineEdit(), TQT_SIGNAL( returnPressed() ),
           this, TQT_SLOT( lineEnterPressed() ) );
  connect( this, TQT_SIGNAL( textChanged( const TQString& ) ),
           TQT_SLOT( slotTextChanged( const TQString& ) ) );

  mPopup = new KDatePickerPopup( KDatePickerPopup::DatePicker | KDatePickerPopup::Words );
  mPopup->hide();
  mPopup->installEventFilter( this );

  connect( mPopup, TQT_SIGNAL( dateChanged( TQDate ) ),
           TQT_SLOT( dateSelected( TQDate ) ) );

  // handle keyword entry
  setupKeywords();
  lineEdit()->installEventFilter( this );

  setValidator( new DateValidator( mKeywordMap.keys(), this ) );

  mTextChanged = false;
}

KDateEdit::~KDateEdit()
{
  delete mPopup;
  mPopup = 0;
}

void KDateEdit::setDate( const TQDate& date )
{
  assignDate( date );
  updateView();
}

TQDate KDateEdit::date() const
{
  return mDate;
}

void KDateEdit::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
  lineEdit()->setReadOnly( readOnly );
}

bool KDateEdit::isReadOnly() const
{
  return mReadOnly;
}

void KDateEdit::popup()
{
  if ( mReadOnly )
    return;

  TQRect desk = KGlobalSettings::desktopGeometry( this );

  TQPoint popupPoint = mapToGlobal( TQPoint( 0,0 ) );

  int dateFrameHeight = mPopup->sizeHint().height();
  if ( popupPoint.y() + height() + dateFrameHeight > desk.bottom() )
    popupPoint.setY( popupPoint.y() - dateFrameHeight );
  else
    popupPoint.setY( popupPoint.y() + height() );

  int dateFrameWidth = mPopup->sizeHint().width();
  if ( popupPoint.x() + dateFrameWidth > desk.right() )
    popupPoint.setX( desk.right() - dateFrameWidth );

  if ( popupPoint.x() < desk.left() )
    popupPoint.setX( desk.left() );

  if ( popupPoint.y() < desk.top() )
    popupPoint.setY( desk.top() );

  if ( mDate.isValid() )
    mPopup->setDate( mDate );
  else
    mPopup->setDate( TQDate::currentDate() );

  mPopup->popup( popupPoint );

  // The combo box is now shown pressed. Make it show not pressed again
  // by causing its (invisible) list box to emit a 'selected' signal.
  // First, ensure that the list box contains the date currently displayed.
  TQDate date = parseDate();
  assignDate( date );
  updateView();
  // Now, simulate an Enter to unpress it
  TQListBox *lb = listBox();
  if (lb) {
    lb->setCurrentItem(0);
    TQKeyEvent* keyEvent = new TQKeyEvent(TQEvent::KeyPress, Qt::Key_Enter, 0, 0);
    TQApplication::postEvent(lb, keyEvent);
  }
}

void KDateEdit::dateSelected( TQDate date )
{
  if (assignDate( date ) ) {
    updateView();
    emit dateChanged( date );
    emit dateEntered( date );

    if ( date.isValid() ) {
      mPopup->hide();
    }
  }
}

void KDateEdit::lineEnterPressed()
{
  bool replaced = false;

  TQDate date = parseDate( &replaced );

  if (assignDate( date ) ) {
    if ( replaced )
      updateView();

    emit dateChanged( date );
    emit dateEntered( date );
  }
}

TQDate KDateEdit::parseDate( bool *replaced ) const
{
  TQString text = currentText();
  TQDate result;

  if ( replaced )
    (*replaced) = false;

  if ( text.isEmpty() )
    result = TQDate();
  else if ( mKeywordMap.contains( text.lower() ) ) {
    TQDate today = TQDate::currentDate();
    int i = mKeywordMap[ text.lower() ];
    if ( i >= 100 ) {
      /* A day name has been entered. Convert to offset from today.
       * This uses some math tricks to figure out the offset in days
       * to the next date the given day of the week occurs. There
       * are two cases, that the new day is >= the current day, which means
       * the new day has not occurred yet or that the new day < the current day,
       * which means the new day is already passed (so we need to find the
       * day in the next week).
       */
      i -= 100;
      int currentDay = today.dayOfWeek();
      if ( i >= currentDay )
        i -= currentDay;
      else
        i += 7 - currentDay;
    }

    result = today.addDays( i );
    if ( replaced )
      (*replaced) = true;
  } else {
    result = KGlobal::locale()->readDate( text );
  }

  return result;
}

bool KDateEdit::eventFilter( TQObject *object, TQEvent *event )
{
  if ( object == lineEdit() ) {
    // We only process the focus out event if the text has changed
    // since we got focus
    if ( (event->type() == TQEvent::FocusOut) && mTextChanged ) {
      lineEnterPressed();
      mTextChanged = false;
    } else if ( event->type() == TQEvent::KeyPress ) {
      // Up and down arrow keys step the date
      TQKeyEvent* keyEvent = (TQKeyEvent*)event;

      if ( keyEvent->key() == Qt::Key_Return ) {
        lineEnterPressed();
        return true;
      }

      int step = 0;
      if ( keyEvent->key() == Qt::Key_Up )
        step = 1;
      else if ( keyEvent->key() == Qt::Key_Down )
        step = -1;
      // TODO: If it's not an input key, but something like Return, Enter, Tab, etc..., don't eat the keypress, but handle it through to the default eventfilter!
      if ( step && !mReadOnly ) {
        TQDate date = parseDate();
        if ( date.isValid() ) {
          date = date.addDays( step );
          if ( assignDate( date ) ) {
            updateView();
            emit dateChanged( date );
            emit dateEntered( date );
            return true;
          }
        }
      }
    }
  } else {
    // It's a date picker event
    switch ( event->type() ) {
      case TQEvent::MouseButtonDblClick:
      case TQEvent::MouseButtonPress: {
        TQMouseEvent *mouseEvent = (TQMouseEvent*)event;
        if ( !mPopup->rect().contains( mouseEvent->pos() ) ) {
          TQPoint globalPos = mPopup->mapToGlobal( mouseEvent->pos() );
          if ( TQApplication::widgetAt( globalPos, true ) == this ) {
            // The date picker is being closed by a click on the
            // KDateEdit widget. Avoid popping it up again immediately.
            mDiscardNextMousePress = true;
          }
        }

        break;
      }
      default:
        break;
    }
  }

  return false;
}

void KDateEdit::mousePressEvent( TQMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton && mDiscardNextMousePress ) {
    mDiscardNextMousePress = false;
    return;
  }

  TQComboBox::mousePressEvent( event );
}

void KDateEdit::slotTextChanged( const TQString& )
{
  TQDate date = parseDate();

  if ( assignDate( date ) )
    emit dateChanged( date );

  mTextChanged = true;
}

void KDateEdit::setupKeywords()
{
  // Create the keyword list. This will be used to match against when the user
  // enters information.
  mKeywordMap.insert( i18n( "tomorrow" ), 1 );
  mKeywordMap.insert( i18n( "today" ), 0 );
  mKeywordMap.insert( i18n( "yesterday" ), -1 );

  TQString dayName;
  for ( int i = 1; i <= 7; ++i ) {
    dayName = KGlobal::locale()->calendar()->weekDayName( i ).lower();
    mKeywordMap.insert( dayName, i + 100 );
  }
}

bool KDateEdit::assignDate( const TQDate& date )
{
  mDate = date;
  mTextChanged = false;
  return true;
}

void KDateEdit::updateView()
{
  TQString dateString;
  if ( mDate.isValid() )
    dateString = KGlobal::locale()->formatDate( mDate, true );

  // We do not want to generate a signal here,
  // since we explicitly setting the date
  bool blocked = signalsBlocked();
  blockSignals( true );
  changeItem( dateString, 0 );
  blockSignals( blocked );
}

#include "kdateedit.moc"
