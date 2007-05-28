/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "aboutdata.h"

#include <qstring.h>
#include <klocale.h>



AboutData::AboutData()
    : KAboutData("kmobiletools", I18N_NOOP("KMobileTools"), KMOBILETOOLS_VERSION, I18N_NOOP("A KDE Mobile Phone Management Tool"),
                 License_GPL, I18N_NOOP("(C) 2007 KMobileTools developers"), 0,
                     "http://www.kmobiletools.org/")
{
    setVersion(KMOBILETOOLS_VERSION);
    addAuthor( "Marco Gulino", I18N_NOOP("Maintainer, core developer, AT engine and coordinator"), "marco@kmobiletools.org" );
    addAuthor( "Stefan Bogner", I18N_NOOP("Gammu engine"), "bochi@kmobiletools.org" );

    addCredit( "Pino Toscano", I18N_NOOP("Help porting to KDE4"), "toscano.pino@tiscali.it" );
    addCredit( "Alexander Rensmann", I18N_NOOP("AT engine, Siemens support"), "zerraxys@gmx.net" );
    addCredit( "Lee Olson", I18N_NOOP("KMobileTools main icon, images, artwork"), "clearbeast@gmail.com" );
}

AboutData::~AboutData()
{
}
