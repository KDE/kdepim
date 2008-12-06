/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef KAB_CONFIGUREWIDGET_H
#define KAB_CONFIGUREWIDGET_H

#include <QtGui/QWidget>

#include "kaddressbook_export.h"

namespace KABC {
class AddressBook;
}

class KConfigGroup;

namespace KAB {

class KABINTERFACES_EXPORT ConfigureWidget : public QWidget
{
  public:
    ConfigureWidget( KABC::AddressBook *ab, QWidget *parent );
    ~ConfigureWidget();

    /**
      This method is called before the configure dialog is shown.
      The widget should reimplement it and fill the GUI with the
      values from the config file.
      Important: Don't change the group of cfg!
     */
    virtual void restoreSettings( const KConfigGroup &cfg );

    /**
      This method is called after the user clicked the 'Ok' button.
      The widget should reimplement it and save all values from
      the GUI to the config file.
      Important: Don't change the group of cfg!
     */
    virtual void saveSettings( KConfigGroup &cfg );


    /**
      Returns a pointer to the address book of this widget.
     */
    KABC::AddressBook *addressBook() const;

  private:
    KABC::AddressBook *mAddressBook;
};

}

#endif
