/*
    aboutdata.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include "aboutdata.h"

#include "resource.h"

namespace KNode
{
  struct about_authors {
    const char* name;
    const char* desc;
    const char* email;
  };

  static const about_authors authors[] = {
    { "Volker Krause", I18N_NOOP("Maintainer"), "volker.krause@rwth-aachen.de" },
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
    : KAboutData( "knode",
                  I18N_NOOP("KNode"),
                  KNODE_VERSION,
                  I18N_NOOP("A newsreader for KDE"),
                  KAboutData::License_GPL,
                  I18N_NOOP("Copyright (c) 1999-2005 the KNode authors"),
                  0,
                  "http://knode.sourceforge.net/" )
  {
    using KNode::authors;
    for ( unsigned int i = 0 ; i < sizeof authors / sizeof *authors ; ++i )
      addAuthor( authors[i].name, authors[i].desc, authors[i].email );

    addCredit( "Jakob Schroeter", 0, "js@camaya.net" );
  }

  AboutData::~AboutData()
  {
  }

} // namespace KNode
