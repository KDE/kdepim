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

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstddirs.h>

// Local includes
#include "Twister.h"
#include "Empath.h"

Twister * Twister::TWISTER = 0L;

    void
Twister::start()
{
    if (0 == TWISTER)
        TWISTER = new Twister;
 
    Empath::start();
}
        
    void
Twister::shutdown()
{
    empath->shutdown();
    delete this;
}

Twister::Twister()
    :   QObject((QObject *)0L, "Twister")
{
    // Don't do dollar expansion by default.
    // Possible security hole.
    KGlobal::config()->setDollarExpansion(false);    
}

    void
Twister::init()
{
    empath->init();
}

Twister::~Twister()
{
}

// vim:ts=4:sw=4:tw=78
#include "Twister.moc"
