/*
  This file is part of libkdepim.

  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 1999 Ian Dawes <iadawes@globalserve.net>
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

//krazy:excludeall=qclasses as we want to subclass from QComboBox, not KComboBox

#include "ktimeedit.h"

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>

#include <QtCore/QDateTime>
#include <QtGui/QLineEdit>
#include <QtGui/QKeyEvent>
#include <QtGui/QValidator>

using namespace KPIM;

// Validator for a time value with only hours and minutes (no seconds)
// Mostly locale aware. Author: David Faure <faure@kde.org>
class KOTimeValidator : public QValidator
{
  public:
    KOTimeValidator( QWidget *parent ) : QValidator( parent )
    {}

    virtual State validate( QString &str, int &/*cursorPos*/ ) const
    {
      int length = str.length();
      // empty string is intermediate so one can clear the edit line and start from scratch
      if ( length <= 0 ) {
        return Intermediate;
      }

      bool ok = false;
      KGlobal::locale()->readTime( str, KLocale::WithoutSeconds, &ok );
      if ( ok ) {
        return Acceptable;
      }
      // kDebug(5300)<<"Time"<<str<<" not directly acceptable, trying military format";
      // Also try to accept times in "military format", i.e. no delimiter, like 1200
      int tm = str.toInt( &ok );
      if ( ok && ( 0 <= tm ) ) {
        if ( ( tm < 2400 ) && ( tm % 100 < 60 ) ) {
          return Acceptable;
        } else {
          return Intermediate;
        }
      }
      // kDebug(5300)<<str<<" not acceptable or intermediate for military format, either"<<str;

      // readTime doesn't help knowing when the string is "Intermediate".
      // HACK. Not fully locale aware etc. (esp. the separator is '.' in sv_SE...)
      QChar sep = ':';
      // I want to allow "HH:", ":MM" and ":" to make editing easier
      if ( str[0] == sep ) {
        if ( length == 1 ) { // just ":"
          return Intermediate;
        }
        QString minutes = str.mid(1);
        int m = minutes.toInt( &ok );
        if ( ok && m >= 0 && m < 60 ) {
          return Intermediate;
        }
      } else if ( str[str.length()-1] == sep ) {
        QString hours = str.left( length - 1 );
        int h = hours.toInt( &ok );
        if ( ok && h >= 0 && h < 24 ) {
          return Intermediate;
        }
      }
//    return Invalid;
      return Intermediate;
    }
    virtual void fixup ( QString & input ) const {
      bool ok = false;
      KGlobal::locale()->readTime( input, KLocale::WithoutSeconds, &ok );
      if ( !ok ) {
        // Also try to accept times in "military format", i.e. no delimiter, like 1200
        int tm = input.toInt( &ok );
        if ( ( 0 <= tm ) && ( tm < 2400 ) && ( tm%100 < 60 ) && ok ) {
          input = KGlobal::locale()->formatTime( QTime( tm / 100, tm % 100, 0 ) );
        }
      }
    }
};

class KTimeEdit::Private
{
  public:
    Private( KTimeEdit *qq )
      : q( qq )
    {
    }

    void addTime( const QTime& );
    void subTime( const QTime& );
    void updateText();

    void slotActivated( int );
    void slotTextChanged();

    KTimeEdit *q;
    QTime mTime;
};

void KTimeEdit::Private::addTime( const QTime &time )
{
  // Calculate the new time.
  mTime = time.addSecs( mTime.minute() * 60 + mTime.hour() * 3600 );
  updateText();
  emit q->timeChanged( mTime );
}

void KTimeEdit::Private::subTime( const QTime &time )
{
  int h, m;

  // Note that we cannot use the same method for determining the new
  // time as we did in addTime, because QTime does not handle adding
  // negative seconds well at all.
  h = mTime.hour() - time.hour();
  m = mTime.minute() - time.minute();

  if ( m < 0 ) {
    m += 60;
    h -= 1;
  }

  if ( h < 0 )
    h += 24;

  // store the newly calculated time.
  mTime.setHMS( h, m, 0 );
  updateText();
  emit q->timeChanged( mTime );
}

void KTimeEdit::Private::updateText()
{
  const QString text = KGlobal::locale()->formatTime( mTime );

  // Set the text but without emitting signals, nor losing the cursor position
  QLineEdit *line = q->lineEdit();
  line->blockSignals( true );
  const int pos = line->cursorPosition();

  // select item with nearest time, must be done while line edit is blocked
  // as setCurrentItem() calls setText() with triggers KTimeEdit::changedText()
  q->setCurrentIndex( ( mTime.hour() * 4 ) + ( ( mTime.minute() + 7 ) / 15 ) );

  line->setText( text );
  line->setCursorPosition( pos );
  line->blockSignals( false );
}

void KTimeEdit::Private::slotActivated( int index )
{
  // The last entry, 23:59, is a special case
  if ( index == q->count() - 1 )
    mTime = QTime( 23, 59, 0 );
  else
    mTime = QTime( 0, 0, 0 ).addSecs( index * 15 * 60 );

  emit q->timeChanged( mTime );
}

void KTimeEdit::Private::slotTextChanged()
{
  if ( q->inputIsValid() ) {
    mTime = q->time();
    emit q->timeChanged( mTime );
  }
}

// KTimeWidget/QTimeEdit provide nicer editing, but don't provide a combobox.
// Difficult to get all in one...
// But Qt-3.2 will offer QLineEdit::setMask, so a "99:99" mask would help.
KTimeEdit::KTimeEdit( QWidget *parent, const QTime &time )
  : QComboBox( parent ), d( new Private( this ) )
{
  setEditable( true );
  setInsertPolicy( NoInsert );
  setValidator( new KOTimeValidator( this ) );

  d->mTime = time;

  // Fill combo box with selection of times in localized format.
  QTime timeEntry( 0, 0, 0 );
  const QTime endEntry = timeEntry;
  do {
    addItem( KGlobal::locale()->formatTime( timeEntry ) );
    timeEntry = timeEntry.addSecs( 60 * 15 );
  } while ( timeEntry != endEntry );

  // Add end of day.
  addItem( KGlobal::locale()->formatTime( QTime( 23, 59, 59 ) ) );

  d->updateText();
  setFocusPolicy( Qt::StrongFocus );

  connect( this, SIGNAL( activated( int ) ), this, SLOT( slotActivated( int ) ) );
  connect( this, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotTextChanged() ) );
}

KTimeEdit::~KTimeEdit()
{
  delete d;
}

QTime KTimeEdit::time() const
{
  bool ok = false;

  QTime result = KGlobal::locale()->readTime( currentText(), KLocale::WithoutSeconds, &ok );
  if ( !ok ) {
    // Also try to accept times in "military format", i.e. no delimiter, like 1200
    const int tm = currentText().toInt( &ok );
    if ( (0 <= tm) && (tm < 2400) && (tm%100 < 60) && ok )
      result.setHMS( tm / 100, tm % 100, 0 );
    else
      ok = false;
  }

  return result;
}

QSizePolicy KTimeEdit::sizePolicy() const
{
  // Set size policy to Fixed, because edit cannot contain more text than the
  // string representing the time. It doesn't make sense to provide more space.
  return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

void KTimeEdit::setTime( const QTime &newTime )
{
  if ( d->mTime != newTime ) {
    d->mTime = newTime;
    d->updateText();
  }
}

void KTimeEdit::keyPressEvent( QKeyEvent *event )
{
  switch ( event->key() ) {
    case Qt::Key_Down:
      d->addTime( QTime( 0, 1, 0 ) );
      break;
    case Qt::Key_Up:
      d->subTime( QTime( 0, 1, 0 ) );
      break;
    case Qt::Key_PageUp:
      d->subTime( QTime( 1, 0, 0 ) );
      break;
    case Qt::Key_PageDown:
      d->addTime( QTime( 1, 0, 0 ) );
      break;
    default:
      QComboBox::keyPressEvent( event );
      break;
  }
}

bool KTimeEdit::inputIsValid() const
{
  int cursorPos = lineEdit()->cursorPosition();
  QString str = currentText();

  return validator()->validate( str, cursorPos ) == QValidator::Acceptable;
}

#include "ktimeedit.moc"
