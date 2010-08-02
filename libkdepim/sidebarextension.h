/*
   This file is part of libkdepim.

   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>

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
#ifndef  SIDEBAREXTENSION_H
#define  SIDEBAREXTENSION_H

#include <tqobject.h>
#include <kdepimmacros.h>

class TQWidget;

namespace KParts
{

  class ReadOnlyPart;

  /**
   * Provides a way to export a widget which will be displayed in Kontacts
   * stackview at the left
   **/
  class KDE_EXPORT SideBarExtension : public QObject
  {
    Q_OBJECT

    public:
      /**
       * Constucts a SideBarExtension.
       *
       * @param exported A TQWidget derived widget that will be provided for the
       *                 users of SideBarExtension.
       * @param parent   The parent widget.
       * @param name     The name of the class.
       **/
      SideBarExtension(TQWidget *exported, KParts::ReadOnlyPart *parent, const char* name);
      ~SideBarExtension();

      /**
       * Retrieve a pointer to the widget. May be 0 if 0 was passed in the constructor
       **/
      TQWidget* widget() const { return m_exported; }

    private:
      TQWidget* m_exported;

      class SideBarExtensionPrivate;
      SideBarExtensionPrivate *d;
  };
}
#endif // SIDEBAREXTENSION_H

// vim: ts=2 sw=2 et
