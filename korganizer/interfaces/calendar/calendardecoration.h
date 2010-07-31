/*
    This file is part of the KOrganizer interfaces.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORG_CALENDARDECORATION_H
#define KORG_CALENDARDECORATION_H

#include <tqstring.h>
#include <tqdatetime.h>
#include <tqpixmap.h>

#include <klibloader.h>

#include "plugin.h"

namespace KOrg {

/**
  This class provides the interface for a date dependent decoration.

  It provides entities like texts and pictures for a given date. Implementations
  can implement all functions or only a subset.
*/
class CalendarDecoration : public Plugin
{
  public:
    static int interfaceVersion() { return 2; }
    static TQString serviceType() { return "Calendar/Decoration"; }

    typedef TQPtrList<CalendarDecoration> List;

    CalendarDecoration() {}
    virtual ~CalendarDecoration() {}

    /**
      Return a short text for a given date, ususally only a few words.
    */
    virtual TQString shortText( const TQDate & ) { return TQString::null; }
    /**
      Return along text for a given date. This text can be of any length, but
      usually it will have one or a few paragraphs.
    */
    virtual TQString longText( const TQDate & ) { return TQString::null; }

    /**
      Return a small pixmap. The size should be something like 30x30 pixels.
    */
    virtual TQPixmap smallPixmap( const TQDate &) { return TQPixmap(); }
    /**
      Return a large pixmap. The size should be something like 300x300 pixels.
    */
    virtual TQPixmap largePixmap( const TQDate &) { return TQPixmap(); }

    /**
      Return a small widget. It should have the size of a pushbutton.
    */
    virtual TQWidget *smallWidget( TQWidget *, const TQDate & ) { return 0; }
};

class CalendarDecorationFactory : public PluginFactory
{
  public:
    virtual CalendarDecoration *create() = 0;
};

}

#endif
