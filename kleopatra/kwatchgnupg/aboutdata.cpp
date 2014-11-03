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

struct about_data {
    const char *name;
    const char *desc;
    const char *email;
    const char *web;
};

static const about_data authors[] = {
    { I18N_NOOP("Steffen Hansen"), I18N_NOOP("Original Author"), "hansen@kde.org", 0 },
};

AboutData::AboutData()
    : KAboutData(QLatin1String("kwatchgnupg"), i18n("KWatchGnuPG"),
                 QLatin1String(kwatchgnupg_version), i18n("GnuPG log viewer"), KAboutLicense::GPL,
                 i18n("(c) 2004 Klar\xC3\xA4lvdalens Datakonsult AB\n"))
{
    using ::authors;
    //using ::credits;
    for (unsigned int i = 0 ; i < sizeof authors / sizeof * authors ; ++i)
        addAuthor(i18n(authors[i].name), i18n(authors[i].desc),
                  QLatin1String(authors[i].email), QLatin1String(authors[i].web));
}
