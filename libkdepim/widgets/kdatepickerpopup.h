/*
  This file is part of libkdepim.

  Copyright (c) 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>

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
#ifndef KDEPIM_KDATEPICKERPOPUP_H
#define KDEPIM_KDATEPICKERPOPUP_H

#include "kdepim_export.h"

#include <QtCore/QDate>
#include <QMenu>

class KDatePicker;

namespace KPIM {

/**
 * @short This menu helps the user to select a date quickly.
 *
 * This menu helps the user to select a date quickly. It offers various
 * modes of selecting, e.g. with a KDatePicker or with words like "Tomorrow".
 *
 * The available modes are:
 *
 * @li NoDate: A menu-item with "No Date". If chosen, the datepicker will emit
 *     a null QDate.
 * @li DatePicker: Shows a KDatePicker-widget.
 * @li Words: Shows items like "Today", "Tomorrow" or "Next Week".
 *
 * @author Bram Schoenmakers <bram_s@softhome.net>
 */
class KDEPIM_EXPORT KDatePickerPopup: public QMenu
{
  Q_OBJECT

  public:
    /**
     * Describes the available selection modes.
     */
    enum Mode
    {
      NoDate = 1,     ///< A menu-item with "No Date". Will always return an invalid date.
      DatePicker = 2, ///< A menu-item with a KDatePicker.
      Words = 4       ///< A menu-item with list of words that describe a date.
    };

    /**
     * Describes the a set of combined modes.
     */
    Q_DECLARE_FLAGS( Modes, Mode )

    /**
     * Creates a new date picker popup.
     *
     * @param modes The selection modes that shall be offered
     * @param date The initial date of date picker widget.
     * @param parent The parent object.
     */
    explicit KDatePickerPopup( Modes modes = DatePicker,
                               const QDate &date = QDate::currentDate(),
                               QWidget *parent = 0 );

    /**
     * Destroys the date picker popup.
     */
    ~KDatePickerPopup();

    /**
     * Returns the used KDatePicker object.
     */
    KDatePicker *datePicker() const;

  public Q_SLOTS:
    /**
     * Sets the current @p date.
     */
    void setDate( const QDate &date );

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the user has selected a new date.
     *
     * @param date The new date.
     */
    void dateChanged( const QDate &date );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotDateChanged( const QDate& ) )
    Q_PRIVATE_SLOT( d, void slotToday() )
    Q_PRIVATE_SLOT( d, void slotTomorrow() )
    Q_PRIVATE_SLOT( d, void slotNextWeek() )
    Q_PRIVATE_SLOT( d, void slotNextMonth() )
    Q_PRIVATE_SLOT( d, void slotNoDate() )
    //@endcond
};

Q_DECLARE_OPERATORS_FOR_FLAGS( KDatePickerPopup::Modes )

}

#endif
