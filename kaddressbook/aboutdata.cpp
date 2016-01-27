/*
  This file is part of KAddressBook.

  Copyright (c) 2009-2016 Laurent Montel <montel@kde.org>

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

#include <KLocalizedString>

AboutData::AboutData()
    : KAboutData(QStringLiteral("kaddressbook"),
                 i18n("KAddressBook"),
                 QStringLiteral(KDEPIM_VERSION),
                 i18n("The KDE Address Book Application"),
                 KAboutLicense::GPL_V2,
                 i18n("Copyright © 2007–2015 KAddressBook authors"))
{
    addAuthor(i18n("Laurent Montel"), i18n("Current maintainer"),  QStringLiteral("montel@kde.org"));
    addAuthor(i18n("Tobias Koenig"),  i18n("Previous maintainer"), QStringLiteral("tokoe@kde.org"));
}

AboutData::~AboutData()
{
}
