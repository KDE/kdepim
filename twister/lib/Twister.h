/*
    Twister - PIM app for KDE
    
    Copyright 2000
        Rik Hemsley <rik@kde.org>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef TWISTER_H
#define TWISTER_H

// Qt includes
#include <qstring.h>
#include <qobject.h>

#define twister Twister::getTwister()

/**
 * Twister is the controller class for Twister's kernel.
 *
 * You can't construct this. Use the static method Twister::create() instead.
*
 * @short Controller class
 * @author Rikkus
 */
class Twister : public QObject
{
    Q_OBJECT

    public:

        /** 
         * Creates a Twister object.
         */
        static void start();
        
        /**
         * Use this to kill off Twister. You should delete the UI first. This
         * allows you to bring down the UI quickly, before Twister dies.
         * Once this method returns, everything should have been cleaned
         * up, synced, and destructed. You may delete your KApplication.
         */
        void shutdown();
        
        /**
         * This must be called after the constructor. You can construct
         * a ui first, but don't try to access any anything important
         * as it's not initialised until you call this. @see start
         */
        void init();

        /**
         * In the style of KApplication and QApplication, this
         * saves you having to pass a pointer to the (single) object of
         * this controller class to every object in the system.
         *
         * There is a macro 'twister' defined that makes this even easier.
         *
         * @short Pointer to controller class.
         * @return Pointer to controller class.
         */
        static Twister * getTwister() { return TWISTER; }

        /**
         * @internal
         */
        static Twister * TWISTER;
        
    protected:

        Twister();
        ~Twister();
};

#endif

// vim:ts=4:sw=4:tw=78
