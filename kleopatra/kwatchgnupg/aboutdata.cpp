/*
    aboutdata.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2004 Klar�vdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "aboutdata.h"

#include <KLocalizedString>

static const char kwatchgnupg_version[] = "1.0";
static const char description[] = I18N_NOOP("GnuPG log viewer");

struct about_data {
  const char * name;
  const char * desc;
  const char * email;
  const char * web;
};

static const about_data authors[] = {
  { I18N_NOOP("Steffen Hansen"), I18N_NOOP("Original Author"), "hansen@kde.org", 0 },
};

#if 0
// can't create zero size array - doesn't compile
static const about_data credits[] = {
  // PENDING(steffen) add stuff
};
#endif

AboutData::AboutData()
  : K4AboutData( "kwatchgnupg", 0, ki18n("KWatchGnuPG"),
                kwatchgnupg_version, ki18n(description), License_GPL,
                ki18n("(c) 2004 Klar\xC3\xA4lvdalens Datakonsult AB\n") )
{
  using ::authors;
  //using ::credits;
  for ( unsigned int i = 0 ; i < sizeof authors / sizeof *authors ; ++i )
    addAuthor( ki18n(authors[i].name), ki18n(authors[i].desc),
               authors[i].email, authors[i].web );
#if 0
  for ( unsigned int i = 0 ; i < sizeof credits / sizeof *credits ; ++i )
    addCredit( ki18n(credits[i].name), ki18n(credits[i].desc),
               credits[i].email, credits[i].web );
#endif
}
