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

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the time has been changed.
     *
     * @note The time can be invalid.
     */
    void timeChanged( const QTime &time );

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
