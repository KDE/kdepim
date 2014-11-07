/*
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef PHONETYPECOMBO_H
#define PHONETYPECOMBO_H

#include <QComboBox>

#include <KContacts/PhoneNumber>

/**
 * @short A combobox to select a phone number type.
 */
class PhoneTypeCombo : public QComboBox
{
  Q_OBJECT

  public:
    /**
     * Creates a phone type combo.
     *
     * @param parent The parent widget.
     */
    explicit PhoneTypeCombo( QWidget *parent = 0 );

    /**
     * Destroys the phone type combo.
     */
    ~PhoneTypeCombo();

    /**
     * Sets the phone number @p type that shall be selected.
     */
    void setType( KContacts::PhoneNumber::Type type );

    /**
     * Returns the selected phone number type.
     */
    KContacts::PhoneNumber::Type type() const;

  private:
    class Private;
    Private *const d;

  Q_PRIVATE_SLOT( d, void selected( int ) )
};

#endif
