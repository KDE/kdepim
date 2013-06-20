/*
  This file is part of KJots.

  Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#include "aboutdata.h"
#include <kdepim-version.h>
#include <klocale.h>

AboutData::AboutData()
    : KAboutData( "kjots", 0, ki18n( "KJots" ),
                  KDEPIM_VERSION,
                  ki18n( "KDE note taking utility" ),
                  KAboutData::License_GPL,
                  ki18n("Copyright © 1997–2010 KJots authors" ),
                  KLocalizedString() )
{

    addAuthor(ki18n("Stephen Kelly"), ki18n("Current maintainer"), "steveire@gmail.com");
    addAuthor(ki18n("Pradeepto K. Bhattacharya"), KLocalizedString(), "pradeepto@kde.org");
    addAuthor(ki18n("Jaison Lee"), KLocalizedString(), "lee.jaison@gmail.com");
    addAuthor(ki18n("Aaron J. Seigo"), KLocalizedString(), "aseigo@kde.org");
    addAuthor(ki18n("Stanislav Kljuhhin"), KLocalizedString(), "crz@starman.ee");
    addAuthor(ki18n("Christoph Neerfeld"), ki18n("Original author"), "chris@kde.org");
    addAuthor(ki18n("Laurent Montel"), KLocalizedString(), "montel@kde.org");
}
