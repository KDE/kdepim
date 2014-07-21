/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#ifndef STYLESHEETLOADER_H
#define STYLESHEETLOADER_H

#include "mobileui_export.h"
#include <QString>
class QApplication;
class QWidget;

/** Applies style sheets to widgets embedded in QML */
namespace StyleSheetLoader
{
  /**
   * Applies the style sheet to @p widget, if it hasn't been set globally anyway.
   */
  MOBILEUI_EXPORT void applyStyle( QWidget *widget );

  /**
   * Globally apply the style sheet.
   * Only use on platforms that don't have a sensible native widget style, such as Maemo >= 6.
   */
  MOBILEUI_EXPORT void applyStyle( QApplication *app );

  /**
   * Returns the style sheet.
   * @internal
   */
  QString styleSheet();
}

#endif
