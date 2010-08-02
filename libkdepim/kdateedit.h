/*
    This file is part of libkdepim.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 David Jarvie <software@astrojar.org.uk>
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

#ifndef KDATEEDIT_H
#define KDATEEDIT_H

#include <tqcombobox.h>
#include <tqdatetime.h>
#include <tqmap.h>

#include <kdepimmacros.h>

#include "kdatepickerpopup.h"

class TQEvent;

/**
  A date editing widget that consists of an editable combo box.
  The combo box contains the date in text form, and clicking the combo
  box arrow will display a 'popup' style date picker.

  This widget also supports advanced features like allowing the user
  to type in the day name to get the date. The following keywords
  are supported (in the native language): tomorrow, yesturday, today,
  monday, tuesday, wednesday, thursday, friday, saturday, sunday.

  @image html kdateedit.png "This is how it looks"

  @author Cornelius Schumacher <schumacher@kde.org>
  @author Mike Pilone <mpilone@slac.com>
  @author David Jarvie <software@astrojar.org.uk>
  @author Tobias Koenig <tokoe@kde.org>
*/
class KDE_EXPORT KDateEdit : public QComboBox
{
  Q_OBJECT

  public:
    KDateEdit( TQWidget *parent = 0, const char *name = 0 );
    virtual ~KDateEdit();

    /**
      @return The date entered. This date could be invalid,
              you have to check validity yourself.
     */
    TQDate date() const;

    /**
      Sets whether the widget is read-only for the user. If read-only,
      the date picker pop-up is inactive, and the displayed date cannot be edited.

      @param readOnly True to set the widget read-only, false to set it read-write.
     */
    void setReadOnly( bool readOnly );

    /**
      @return True if the widget is read-only, false if read-write.
     */
    bool isReadOnly() const;

    virtual void popup();

  signals:
    /**
      This signal is emitted whenever the user has entered a new date.
      When the user changes the date by editing the line edit field,
      the signal is not emitted until focus leaves the line edit field.
      The passed date can be invalid.
     */
    void dateEntered( const TQDate &date );

    /**
      This signal is emitted whenever the user modifies the date.
      The passed date can be invalid.
     */
    void dateChanged( const TQDate &date );

  public slots:
    /**
      Sets the date.

      @param date The new date to display. This date must be valid or
                  it will not be set
     */
    void setDate( const TQDate &date );

  protected slots:
    void lineEnterPressed();
    void slotTextChanged( const TQString& );
    void dateSelected( TQDate );

  protected:
    virtual bool eventFilter( TQObject*, TQEvent* );
    virtual void mousePressEvent( TQMouseEvent* );

    /**
      Sets the date, without altering the display.
      This method is used internally to set the widget's date value.
      As a virtual method, it allows derived classes to perform additional validation
      on the date value before it is set. Derived classes should return true if
      TQDate::isValid(@p date) returns false.

      @param date The new date to set.
      @return True if the date was set, false if it was considered invalid and
              remains unchanged.
     */
    virtual bool assignDate( const TQDate &date );

    /**
      Fills the keyword map. Reimplement it if you want additional
      keywords.
     */
    void setupKeywords();

  private:
    TQDate parseDate( bool* = 0 ) const;
    void updateView();

    KDatePickerPopup *mPopup;

    TQDate mDate;
    bool mReadOnly;
    bool mTextChanged;
    bool mDiscardNextMousePress;

    TQMap<TQString, int> mKeywordMap;
};

#endif
