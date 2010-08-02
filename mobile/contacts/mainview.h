/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "kdeclarativemainview.h"

namespace Akonadi
{
  class Item;
}

class MainView : public KDeclarativeMainView
{
  Q_OBJECT
  public:
    explicit MainView( QWidget *parent = 0 );

  public Q_SLOTS:
    void newContact();
    void editContact( const Akonadi::Item &item );

    void newContactGroup();
    void editContactGroup( const Akonadi::Item &item );

  protected slots:
    virtual void delayedInit();
};

#endif // MAINVIEW_H
