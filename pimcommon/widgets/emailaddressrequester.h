/*
  Copyright (c) 2001 Marc Mutz <mutz@kde.org>

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
*/

#ifndef MESSAGECORE_EMAILADDRESSREQUESTER_H
#define MESSAGECORE_EMAILADDRESSREQUESTER_H

#include "pimcommon_export.h"

#include <QWidget>

class KLineEdit;

namespace PimCommon {

/**
 * @short A widget to input one or more email addresses.
 *
 * @author Marc Mutz <mutz@kde.org>
 */
class PIMCOMMON_EXPORT EmailAddressRequester : public QWidget
{
    Q_OBJECT

    Q_PROPERTY( QString text READ text WRITE setText NOTIFY textChanged USER true )

public:
    /**
     * Creates a new email address requester.
     *
     * @param parent The parent widget.
     */
    explicit EmailAddressRequester( QWidget* parent = 0 );

    /**
     * Destroys the email address requester.
     */
    ~EmailAddressRequester();

    /**
     * Clears the text of the email address requester.
     */
    void clear();

    /**
     * Sets the @p text of the email address requester.
     */
    void setText( const QString &text );

    /**
     * Returns the text of the email address requester.
     */
    QString text() const;

    /**
     * Returns the line edit that is used by the email address requester.
     */
    KLineEdit* lineEdit() const;

Q_SIGNALS:
    /**
     * This signal is emitted whenever the text of the email address requester
     * has been changed.
     */
    void textChanged();

private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotAddressBook() )
    //@endcond
};

}

#endif
