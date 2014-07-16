/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2010 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "aboutdata.h"

#include "resource.h"

#include <klocale.h>

namespace KNode
{
  struct about_authors {
    const char* name;
    const char* desc;
    const char* email;
  };

  static const about_authors authors[] = {
    { "Olivier Trichet", I18N_NOOP("Maintainer"), "nive@nivalis.org" },
    { "Volker Krause", I18N_NOOP("Former maintainer"), "vkrause@kde.org" },
    { "Roberto Selbach Teixeira", I18N_NOOP("Former maintainer"), "roberto@kde.org" },
    { "Christian Gebauer", 0, "gebauer@kde.org" },
    { "Christian Thurner", 0, "cthurner@web.de" },
    { "Dirk Mueller", 0, "mueller@kde.org" },
    { "Marc Mutz", 0, "mutz@kde.org" },
    { "Mathias Waack", 0, "mathias@atoll-net.de" },
    { "Laurent Montel", 0, "montel@kde.org" },
    { "Stephan Johach", 0, "lucardus@onlinehome.de" },
    { "Matthias Kalle Dalheimer", 0, "kalle@kde.org" },
    { "Zack Rusin", 0, "zack@kde.org" }
  };

  AboutData::AboutData()
    : KAboutData( "knode", 0,
                  ki18n("KNode"),
                  KNODE_VERSION,
                  ki18n("A newsreader for KDE"),
                  KAboutData::License_GPL,
                  ki18n("Copyright © 1999–2014 KNode authors"),
                  KLocalizedString(),
                  "http://userbase.kde.org/KNode" )
  {
    setOrganizationDomain( "kde.org" );
    using KNode::authors;
    for ( unsigned int i = 0 ; i < sizeof authors / sizeof *authors ; ++i )
      addAuthor( ki18n(authors[i].name), ki18n(authors[i].desc), authors[i].email );

    addCredit( ki18n("Jakob Schroeter"), KLocalizedString(), "js@camaya.net" );
  }

  AboutData::~AboutData()
  {
  }

} // namespace KNode
