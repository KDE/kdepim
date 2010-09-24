/*
  This file is part of libkdepim.

  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 1999 Ian Dawes <iadawes@globalserve.net>

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
#ifndef KDEPIM_KTIMEEDIT_H
#define KDEPIM_KTIMEEDIT_H

#include "kdepim_export.h"

#include <QtCore/QTime>
#include <QtGui/QComboBox>

class QKeyEvent;

namespace KPIM {

/**
 * @short Provides a way to edit times in a user-friendly manner.
 * This is a class that provides an easy, user friendly way to edit times.
 * up/down/ increase or decrease time, respectively.
 *
 * @author Preston Brown, Ian Dawes
 */
class KDEPIM_EXPORT KTimeEdit : public QComboBox
{
  Q_OBJECT

  public:
    /**
     * Creates a new time edit.
     *
     * @param parent The parent widget.
     * @param time The initial time to show.
     */
    explicit KTimeEdit( QWidget *parent = 0, const QTime &time = QTime( 12, 0 ) );

    /**
     * Destroys the time edit.
     */
    virtual ~KTimeEdit();

    /**
     * Returns the currently selected time.
     */
    QTime time() const;

    /**
     * Returns whether the current input is a valid time.
     */
    bool inputIsValid() const;

    /**
     * Returns the preferred size policy of the KTimeEdit
     */
    QSizePolicy sizePolicy() const;

    /**
     * Sets the maximum time of the time edit.
     * When setting this property, the minimumTime is adjusted if necessary to
     * ensure that the range remains valid.
     * If the time is not a valid QTime object, this function does nothing.
     * By default, this property contains a time of 23:59:59 and 999 milliseconds.
     */
    void setMaximumTime( const QTime &max );
    /**
     * Retrieves the maximum time of the time edit
     * By default, this property contains a time of 23:59:59 and 999 milliseconds.
     */
    QTime maximumTime () const;
    /**
     * Sets the minimum time of the time edit.
     * When setting this property the maximumTime is adjusted if necessary,
     * to ensure that the range remains valid.
     * If the time is not a valid QTime object, this function does nothing.
     * By default, the min time is set to a value of 00:00:00 and 0 milliseconds.
     */
    void setMinimumTime( const QTime &min );
    /**
     * Retrieves the minimum time of the time edit
     * By default, the min time is set to a value of 00:00:00 and 0 milliseconds.
     */
    QTime minimumTime() const;

    /**
     * Convenience function to set minimum and maximum time with one function call.
     * is analogous to:
     * setMinimumTime(min);
     * setMaximumTime(max);
     * If either min or max are not valid, this function does nothing.
     */
    void setTimeRange( const QTime &min, const QTime &max );

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the time has been changed.
     * Unlike timeEdited(), this signal is also emitted when the time is changed
     * programmatically.
     *
     * @note The time can be invalid.
     */
    void timeChanged( const QTime &time );

    /**
     * This signal is emitted whenever the time has been edited by the user.
     * Unlike timeChanged(), this signal is not emitted when the time is changed
     * programmatically.
     *
     * @note The time can be invalid.
     */
    void timeEdited( const QTime &time );

  public Q_SLOTS:
    /**
     * Sets the current time.
     */
    void setTime( const QTime &time );

  protected:
    virtual void keyPressEvent( QKeyEvent *qke );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotActivated( int ) )
    Q_PRIVATE_SLOT( d, void slotTextChanged() )
    //@endcond
};

}

#endif
