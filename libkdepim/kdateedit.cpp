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

//krazy:excludeall=qclasses as we want to subclass from QComboBox, not KComboBox

#include "kdateedit.h"

#include "kdatepickerpopup.h"

#include <KCalendarSystem>
#include <KGlobal>
#include <KGlobalSettings>
#include <KLocale>

#include <QtCore/QEvent>
#include <QtCore/QDate>
#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QLineEdit>
#include <QtGui/QMouseEvent>
#include <QtGui/QValidator>

using namespace KPIM;

class DateValidator : public QValidator
{
  public:
    DateValidator( const QStringList &keywords, QWidget *parent )
      : QValidator( parent ), mKeywords( keywords )
    {}

    virtual State validate( QString &str, int & ) const
    {
      int length = str.length();

      // empty string is intermediate so one can clear the edit line and start from scratch
      if ( length <= 0 ) {
        return Intermediate;
      }

      if ( mKeywords.contains( str.toLower() ) ) {
        return Acceptable;
      }

      bool ok = false;
      KGlobal::locale()->readDate( str, &ok );
      if ( ok ) {
        return Acceptable;
      } else {
        return Intermediate;
      }
    }

  private:
    QStringList mKeywords;
};

class KDateEdit::Private
{
  public:
    Private( KDateEdit *qq )
      : q( qq ),
        mDate( QDate::currentDate() ),
        mReadOnly( false ),
        mDiscardNextMousePress( false )
    {
    }

    QDate parseDate( bool *replaced = 0 ) const;
    void updateView();
    void setupKeywords();

    // slots
    void lineEnterPressed();
    void slotTextChanged( const QString& );
    void dateSelected( const QDate& );

    KDateEdit *q;

    KDatePickerPopup *mPopup;

    QDate mDate;
    bool mReadOnly;
    bool mTextChanged;
    bool mDiscardNextMousePress;

    QMap<QString, int> mKeywordMap;
};

QDate KDateEdit::Private::parseDate( bool *replaced ) const
{
  const QString text = q->currentText();

  if ( replaced )
    (*replaced) = false;

  QDate result;
  if ( text.isEmpty() ) {
    result = QDate();
  } else if ( mKeywordMap.contains( text.toLower() ) ) {
    const QDate today = QDate::currentDate();
    int i = mKeywordMap[ text.toLower() ];
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

      const int currentDay = today.dayOfWeek();
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

void KDateEdit::Private::updateView()
{
  QString dateString;
  if ( mDate.isValid() )
    dateString = KGlobal::locale()->formatDate( mDate, KLocale::ShortDate );

  // We do not want to generate a signal here,
  // since we explicitly setting the date
  const bool blocked = q->signalsBlocked();
  q->blockSignals( true );
  q->removeItem( 0 );
  q->insertItem( 0, dateString );
  q->blockSignals( blocked );
}

void KDateEdit::Private::setupKeywords()
{
  // Create the keyword list. This will be used to match against when the user
  // enters information.
  mKeywordMap.insert( i18nc( "the day after today", "tomorrow" ), 1 );
  mKeywordMap.insert( i18nc( "this day", "today" ), 0 );
  mKeywordMap.insert( i18nc( "the day before today", "yesterday" ), -1 );

  QString dayName;
  for ( int i = 1; i <= 7; ++i ) {
    dayName = KGlobal::locale()->calendar()->weekDayName( i ).toLower();
    mKeywordMap.insert( dayName, i + 100 );
  }
}

void KDateEdit::Private::lineEnterPressed()
{
  bool replaced = false;

  const QDate date = parseDate( &replaced );

  if ( q->assignDate( date ) ) {
    if ( replaced ) {
      updateView();
    }

    emit q->dateChanged( date );
    emit q->dateEntered( date );
  }
}

void KDateEdit::Private::slotTextChanged( const QString& )
{
  const QDate date = parseDate();

  if ( q->assignDate( date ) )
    emit q->dateChanged( date );

  mTextChanged = true;
}

void KDateEdit::Private::dateSelected( const QDate &date )
{
  if ( q->assignDate( date ) ) {
    updateView();
    emit q->dateChanged( date );
    emit q->dateEntered( date );

    if ( date.isValid() )
      mPopup->hide();
  }
}


KDateEdit::KDateEdit( QWidget *parent )
  : QComboBox( parent ), d( new Private( this ) )
{
  // need at least one entry for popup to work
  setMaxCount( 1 );
  setEditable( true );

  const QString today = KGlobal::locale()->formatDate( d->mDate, KLocale::ShortDate );

  addItem( today );
  setCurrentIndex( 0 );
  setSizeAdjustPolicy( AdjustToContents );

  connect( lineEdit(), SIGNAL( returnPressed() ),
           this, SLOT( lineEnterPressed() ) );
  connect( this, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotTextChanged( const QString& ) ) );

#ifndef KDEPIM_MOBILE_UI
  d->mPopup = new KDatePickerPopup( KDatePickerPopup::DatePicker | KDatePickerPopup::Words,
                                    QDate::currentDate(), this );
#else
  d->mPopup = new KDatePickerPopup( KDatePickerPopup::DatePicker | KDatePickerPopup::Words,
                                    QDate::currentDate(), 0 );
#endif

  d->mPopup->hide();
  d->mPopup->installEventFilter( this );

  connect( d->mPopup, SIGNAL( dateChanged( const QDate& ) ),
           SLOT( dateSelected( const QDate& ) ) );

  // handle keyword entry
  d->setupKeywords();
  lineEdit()->installEventFilter( this );

  setValidator( new DateValidator( d->mKeywordMap.keys(), this ) );

  d->mTextChanged = false;
}

KDateEdit::~KDateEdit()
{
  delete d;
}

void KDateEdit::setDate( const QDate &date )
{
  assignDate( date );
  d->updateView();
}

QDate KDateEdit::date() const
{
  return d->mDate;
}

void KDateEdit::setReadOnly( bool readOnly )
{
  d->mReadOnly = readOnly;
  lineEdit()->setReadOnly( readOnly );
}

bool KDateEdit::isReadOnly() const
{
  return d->mReadOnly;
}

void KDateEdit::showPopup()
{
  if ( d->mReadOnly )
    return;

  const QRect desk = KGlobalSettings::desktopGeometry( this );

  QPoint popupPoint = mapToGlobal( QPoint( 0, 0 ) );

  const int dateFrameHeight = d->mPopup->sizeHint().height();
  if ( popupPoint.y() + height() + dateFrameHeight > desk.bottom() )
    popupPoint.setY( popupPoint.y() - dateFrameHeight );
  else
    popupPoint.setY( popupPoint.y() + height() );

  const int dateFrameWidth = d->mPopup->sizeHint().width();
  if ( popupPoint.x() + dateFrameWidth > desk.right() )
    popupPoint.setX( desk.right() - dateFrameWidth );

  if ( popupPoint.x() < desk.left() )
    popupPoint.setX( desk.left() );

  if ( popupPoint.y() < desk.top() )
    popupPoint.setY( desk.top() );

  if ( d->mDate.isValid() )
    d->mPopup->setDate( d->mDate );
  else
    d->mPopup->setDate( QDate::currentDate() );

  d->mPopup->popup( popupPoint );

  // The combo box is now shown pressed. Make it show not pressed again
  // by causing its (invisible) list box to emit a 'selected' signal.
  // First, ensure that the list box contains the date currently displayed.
  const QDate date = d->parseDate();
  assignDate( date );
  d->updateView();

  // Now, simulate an Enter to unpress it
  QAbstractItemView *lb = view();
  if ( lb ) {
    lb->setCurrentIndex( lb->model()->index( 0, 0 ) );
    QKeyEvent *keyEvent = new QKeyEvent( QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier );
    QApplication::postEvent( lb, keyEvent );
  }
}

void KDateEdit::focusOutEvent( QFocusEvent *event )
{
  if ( d->mTextChanged ) {
    d->lineEnterPressed();
    d->mTextChanged = false;
  }

  QComboBox::focusOutEvent( event );
}

void KDateEdit::keyPressEvent( QKeyEvent *event )
{
  int step = 0;

  if ( event->key() == Qt::Key_Up )
    step = 1;
  else if ( event->key() == Qt::Key_Down )
    step = -1;

  if ( step && !d->mReadOnly ) {
    QDate date = d->parseDate();
    if ( date.isValid() ) {
      date = date.addDays( step );
      if ( assignDate( date ) ) {
        d->updateView();
        emit dateChanged( date );
        emit dateEntered( date );
      }
    }
  }

  QComboBox::keyPressEvent( event );
}

bool KDateEdit::eventFilter( QObject *object, QEvent *event )
{
  if ( object == lineEdit() ) {
    // We only process the focus out event if the text has changed
    // since we got focus
    if ( (event->type() == QEvent::FocusOut) && d->mTextChanged ) {
      d->lineEnterPressed();
      d->mTextChanged = false;
    } else if ( event->type() == QEvent::KeyPress ) {
      // Up and down arrow keys step the date
      QKeyEvent *keyEvent = (QKeyEvent *)event;

      if ( keyEvent->key() == Qt::Key_Return ) {
        d->lineEnterPressed();
        return true;
      }
    }
  } else {
    // It's a date picker event
    switch ( event->type() ) {
      case QEvent::MouseButtonDblClick:
      case QEvent::MouseButtonPress:
        {
          QMouseEvent *mouseEvent = (QMouseEvent*)event;
          if ( !d->mPopup->rect().contains( mouseEvent->pos() ) ) {
            const QPoint globalPos = d->mPopup->mapToGlobal( mouseEvent->pos() );
            if ( QApplication::widgetAt( globalPos ) == this ) {
              // The date picker is being closed by a click on the
              // KDateEdit widget. Avoid popping it up again immediately.
              d->mDiscardNextMousePress = true;
            }
          }

        }
        break;
      default:
        break;
    }
  }

  return false;
}

void KDateEdit::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton && d->mDiscardNextMousePress ) {
    d->mDiscardNextMousePress = false;
    return;
  }

  QComboBox::mousePressEvent( event );
}

bool KDateEdit::assignDate( const QDate &date )
{
  d->mDate = date;
  d->mTextChanged = false;

  return true;
}

#include "kdateedit.moc"
