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

#ifndef KDECLARATIVEAPPLICATION_H
#define KDECLARATIVEAPPLICATION_H

#include "mobileui_export.h"
#include <kuniqueapplication.h>

class MOBILEUI_EXPORT KDeclarativeApplication : public KApplication
{
  Q_OBJECT
  public:
    KDeclarativeApplication();

    /** Sets up some stuff. Only needs to be called (before the
        KApplication constructor) if you don't use
        KDeclarativeApplication as your KApplication
    */
    static void preApplicationSetup();

    KDE_DEPRECATED static void initCmdLine() { preApplicationSetup(); }

    /** Sets up some other stuff. Only needs to be called (after the
        KApplication constructor) if you don't use
        KDeclarativeApplication as your KApplication */
    static void postApplicationSetup();
};

#endif
