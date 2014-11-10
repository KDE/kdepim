/*
  This file is part of KOrganizer.

  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

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

#ifndef CALENDARSUPPORT_MESSAGEWIDGET_H
#define CALENDARSUPPORT_MESSAGEWIDGET_H

#include "calendarsupport_export.h"

#include <KMessageWidget>

namespace CalendarSupport
{

/**
 * Even less instrusive message dialog.
 * This one goes away when you click somewhere, doesn't need a close button.
 */
class CALENDARSUPPORT_EXPORT MessageWidget : public KMessageWidget
{
    Q_OBJECT
public:
    explicit MessageWidget(QWidget *parent = 0);
    ~MessageWidget();
    /**reimp*/ bool eventFilter(QObject *watched, QEvent *event);
    /**reimp*/ void showEvent(QShowEvent *event);
    /**reimp*/ void hideEvent(QHideEvent *event);
};
}

#endif
