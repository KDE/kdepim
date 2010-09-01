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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqkeycode.h>
#include <tqcombobox.h>
#include <tqdatetime.h>
#include <tqlineedit.h>

#include <kmessagebox.h>
#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>

#include "ktimeedit.h"
#include <tqvalidator.h>
#include "ktimeedit.moc"

// Validator for a time value with only hours and minutes (no seconds)
// Mostly locale aware. Author: David Faure <faure@kde.org>
class KOTimeValidator : public QValidator
{
public:
    KOTimeValidator(TQWidget* parent, const char* name=0) : TQValidator(parent, name) {}

    virtual State validate(TQString& str, int& /*cursorPos*/) const
    {
        int length = str.length();
        // empty string is intermediate so one can clear the edit line and start from scratch
        if ( length <= 0 )
            return Intermediate;

        bool ok = false;
        /*TQTime time =*/ KGlobal::locale()->readTime(str, KLocale::WithoutSeconds, &ok);
        if ( ok )
            return Acceptable;
//         kdDebug(5300)<<"Time "<<str<<" not directly acceptable, trying military format "<<endl;
        // Also try to accept times in "military format", i.e. no delimiter, like 1200
        int tm = str.toInt( &ok );
        if ( ok && ( 0 <= tm ) ) {
          if ( ( tm < 2400 ) && ( tm%100 < 60 ) )
            return Acceptable;
          else
            return Intermediate;
        }
//         kdDebug(5300)<<str<<" not acceptable or intermediate for military format, either "<<str<<endl;

        // readTime doesn't help knowing when the string is "Intermediate".
        // HACK. Not fully locale aware etc. (esp. the separator is '.' in sv_SE...)
        TQChar sep = ':';
        // I want to allow "HH:", ":MM" and ":" to make editing easier
        if ( str[0] == sep )
        {
            if ( length == 1 ) // just ":"
                return Intermediate;
            TQString minutes = str.mid(1);
            int m = minutes.toInt(&ok);
            if ( ok && m >= 0 && m < 60 )
                return Intermediate;
        } else if ( str[str.length()-1] == sep )
        {
            TQString hours = str.left(length-1);
            int h = hours.toInt(&ok);
            if ( ok && h >= 0 && h < 24 )
                return Intermediate;
        }
//        return Invalid;
        return Intermediate;
    }
    virtual void fixup ( TQString & input ) const {
      bool ok = false;
      KGlobal::locale()->readTime( input, KLocale::WithoutSeconds, &ok );
      if ( !ok ) {
        // Also try to accept times in "military format", i.e. no delimiter, like 1200
        int tm = input.toInt( &ok );
        if ( ( 0 <= tm ) && ( tm < 2400 ) && ( tm%100 < 60 ) && ok ) {
          input = KGlobal::locale()->formatTime( TQTime( tm / 100, tm % 100, 0 ) );
        }
      }
    }
};

// KTimeWidget/QTimeEdit provide nicer editing, but don't provide a combobox.
// Difficult to get all in one...
// But Qt-3.2 will offer TQLineEdit::setMask, so a "99:99" mask would help.
KTimeEdit::KTimeEdit( TQWidget *parent, TQTime qt, const char *name )
  : TQComboBox( true, parent, name )
{
  setInsertionPolicy( NoInsertion );
  setValidator( new KOTimeValidator( this ) );

  mTime = qt;

//  mNoTimeString = i18n("No Time");
//  insertItem( mNoTimeString );

  // Fill combo box with selection of times in localized format.
  TQTime timeEntry(0,0,0);
  do {
    insertItem(KGlobal::locale()->formatTime(timeEntry));
    timeEntry = timeEntry.addSecs(60*15);
  } while (!timeEntry.isNull());
  // Add end of day.
  insertItem( KGlobal::locale()->formatTime( TQTime( 23, 59, 59 ) ) );

  updateText();
  setFocusPolicy(TQWidget::StrongFocus);

  connect(this, TQT_SIGNAL(activated(int)), this, TQT_SLOT(active(int)));
  connect(this, TQT_SIGNAL(highlighted(int)), this, TQT_SLOT(hilit(int)));
  connect(this, TQT_SIGNAL(textChanged(const TQString&)),this,TQT_SLOT(changedText()));
}

KTimeEdit::~KTimeEdit()
{
}

bool KTimeEdit::hasTime() const
{
  // Can't happen
  if ( currentText().isEmpty() ) return false;
  //if ( currentText() == mNoTimeString ) return false;

  return true; // always
}

TQTime KTimeEdit::getTime() const
{
  //kdDebug(5300) << "KTimeEdit::getTime(), currentText() = " << currentText() << endl;
  // TODO use KLocale::WithoutSeconds in HEAD
  bool ok = false;
  TQTime time = KGlobal::locale()->readTime( currentText(), KLocale::WithoutSeconds, &ok );
  if ( !ok ) {
    // Also try to accept times in "military format", i.e. no delimiter, like 1200
    int tm = currentText().toInt( &ok );
    if ( ( 0 <= tm ) && ( tm < 2400 ) && ( tm%100 < 60 ) && ok ) {
      time.setHMS( tm / 100, tm % 100, 0 );
    } else {
      ok = false;
    }
  }
  // kdDebug(5300) << "KTimeEdit::getTime(): " << time.toString() << endl;
  return time;
}

TQSizePolicy  KTimeEdit::sizePolicy() const
{
  // Set size policy to Fixed, because edit cannot contain more text than the
  // string representing the time. It doesn't make sense to provide more space.
  TQSizePolicy sizePolicy(TQSizePolicy::Fixed,TQSizePolicy::Fixed);

  return sizePolicy;
}

void KTimeEdit::setTime(TQTime newTime)
{
  if ( mTime != newTime )
  {
    kdDebug(5300) << "KTimeEdit::setTime(): " << newTime.toString() << endl;

    mTime = newTime;
    updateText();
  }
}

void KTimeEdit::active(int i)
{
    // The last entry, 23:59, is a special case
    if( i == count() - 1 )
        mTime = TQTime( 23, 59, 0 );
    else
        mTime = TQTime(0,0,0).addSecs(i*15*60);
    emit timeChanged(mTime);
}

void KTimeEdit::hilit(int )
{
  // we don't currently need to do anything here.
}

void KTimeEdit::addTime(TQTime qt)
{
  // Calculate the new time.
  mTime = qt.addSecs(mTime.minute()*60+mTime.hour()*3600);
  updateText();
  emit timeChanged(mTime);
}

void KTimeEdit::subTime(TQTime qt)
{
  int h, m;

  // Note that we cannot use the same method for determining the new
  // time as we did in addTime, because TQTime does not handle adding
  // negative seconds well at all.
  h = mTime.hour()-qt.hour();
  m = mTime.minute()-qt.minute();

  if(m < 0) {
    m += 60;
    h -= 1;
  }

  if(h < 0) {
    h += 24;
  }

  // store the newly calculated time.
  mTime.setHMS(h, m, 0);
  updateText();
  emit timeChanged(mTime);
}

void KTimeEdit::keyPressEvent(TQKeyEvent *qke)
{
  switch(qke->key()) {
  case Key_Down:
    addTime(TQTime(0,1,0));
    break;
  case Key_Up:
    subTime(TQTime(0,1,0));
    break;
  case Key_Prior:
    subTime(TQTime(1,0,0));
    break;
  case Key_Next:
    addTime(TQTime(1,0,0));
    break;
  default:
    TQComboBox::keyPressEvent(qke);
    break;
  } // switch
}

void KTimeEdit::updateText()
{
//  kdDebug(5300) << "KTimeEdit::updateText() " << endl;
  TQString s = KGlobal::locale()->formatTime(mTime);
  // Set the text but without emitting signals, nor losing the cursor position
  TQLineEdit *line = lineEdit();
  line->blockSignals(true);
  int pos = line->cursorPosition();

  // select item with nearest time, must be done while line edit is blocked
  // as setCurrentItem() calls setText() with triggers KTimeEdit::changedText()
  setCurrentItem((mTime.hour()*4)+((mTime.minute()+7)/15));

  line->setText(s);
  line->setCursorPosition(pos);
  line->blockSignals(false);

//  kdDebug(5300) << "KTimeEdit::updateText(): " << s << endl;
}

bool KTimeEdit::inputIsValid() const
{
  int cursorPos = lineEdit()->cursorPosition();
  TQString str = currentText();
  return validator()->validate( str, cursorPos ) == TQValidator::Acceptable;
}

void KTimeEdit::changedText()
{
  //kdDebug(5300) << "KTimeEdit::changedText()" << endl;
  if ( inputIsValid() )
  {
    mTime = getTime();
    emit timeChanged(mTime);
  }
}
