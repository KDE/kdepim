/*
  This file is part of KAddressBook.

  Copyright (c) 2009, 2012 Laurent Montel <montel@kde.org>

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
*/

#include "aboutdata.h"

#include "kdepim-version.h"

#include <KLocale>

AboutData::AboutData()
  : KAboutData( "kaddressbook", 0, ki18n( "KAddressBook" ),
                KDEPIM_VERSION, ki18n( "The KDE Address Book Application" ),
                KAboutData::License_GPL_V2,
                ki18n( "Copyright © 2007–2012 KAddressBook authors" ) )
{
  addAuthor( ki18n( "Tobias Koenig" ), ki18n( "Current maintainer" ), "tokoe@kde.org" );
  addAuthor( ki18n( "Laurent Montel" ), ki18n( "Kontact integration" ), "montel@kde.org" );
}

AboutData::~AboutData()
{
}
