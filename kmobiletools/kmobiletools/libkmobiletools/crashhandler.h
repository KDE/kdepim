/***************************************************************************
 *   Copyright (C) 2007 Sérgio Gomes <sergiomdgomes@gmail.com>             *
 *   Original work by Max Howell <max.howell@methylblue.com> in Amarok     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CRASH_H
#define CRASH_H

#include <kcrash.h> //for main.cpp

namespace KMobileTools
{
    /**
     * @short Adapted from the Amarok crash-handler
     *
     * I'm not entirely sure why this had to be inside a class, but it
     * wouldn't work otherwise *shrug*
     */
    class Crash
    {
        public:
            static void crashHandler( int signal );
    };
}

#endif
