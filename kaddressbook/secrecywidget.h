/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef SECRECYWIDGET_H
#define SECRECYWIDGET_H

#include <qwidget.h>

namespace KABC { class Secrecy; }
class KComboBox;

class SecrecyWidget : public QWidget
{
  Q_OBJECT

  public:
    SecrecyWidget( QWidget *parent, const char *name = 0 );
    ~SecrecyWidget();

    void setSecrecy( const KABC::Secrecy &secrecy );
    KABC::Secrecy secrecy() const;

    void setReadOnly( bool readOnly );

  signals:
    void changed();

  private:
    KComboBox *mSecrecyCombo;
};

#endif
