/*
    aboutdata.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

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

#include <klocale.h>

static const char kleopatra_version[] = KLEOPATRA_VERSION_STRING;
static const char description[] = I18N_NOOP("KDE Key Manager");

struct about_data {
  const char * name;
  const char * desc;
  const char * email;
  const char * web;
};

static const about_data authors[] = {
  { "Marc Mutz", I18N_NOOP("Current Maintainer"), "mutz@kde.org", 0 },
  { "Steffen Hansen", I18N_NOOP("Former Maintainer"), "hansen@kde.org", 0 },
  { "Kalle Dalheimer", I18N_NOOP("Original Author"), "kalle@kde.org", 0 },
  { "Jesper Petersen", I18N_NOOP("Original Author"), "blackie@kde.org", 0 },
};


static const about_data credits[] = {
  { I18N_NOOP("Till Adam"),
    I18N_NOOP("UI Server commands and dialogs"),
    "adam@kde.org", 0 },
  { I18N_NOOP("David Faure"),
    I18N_NOOP("Backend configuration framework, KIO integration"),
    "faure@kde.org", 0 },
  { I18N_NOOP("Michel Boyer de la Giroday"),
    I18N_NOOP("Key-state dependant colors and fonts in the key list"),
    "michel@klaralvdalens-datakonsult.se", 0 },
  { I18N_NOOP("Volker Krause"),
    I18N_NOOP("UI Server dialogs"),
    "vkrause@kde.org", 0 },
  { I18N_NOOP("Thomas Moenicke"),
    I18N_NOOP("Artwork"),
    "tm@php-qt.org", 0 },
  { I18N_NOOP("Daniel Molkentin"),
    I18N_NOOP("Certificate Wizard KIOSK integration, infrastructure"),
    "molkentin@kde.org", 0 },
  { I18N_NOOP("Ralf Nolden"),
    I18N_NOOP("Support for obsolete EMAIL RDN in Certificate Wizard"),
    "nolden@kde.org", 0 },
  { I18N_NOOP("Frank Osterfeld"),
    I18N_NOOP("Resident gpgme/win wrangler, UI Server commands and dialogs"),
    "osterfeld@kde.org", 0 },
  { I18N_NOOP("Karl-Heinz Zimmer"),
    I18N_NOOP("DN display ordering support, infrastructure"),
    "khz@kde.org", 0 },
};


AboutData::AboutData()
  : KAboutData( "kleopatra", 0, ki18n("Kleopatra"),
		kleopatra_version, ki18n(description), License_GPL,
		ki18n("(c) 2002 Steffen Hansen, Jesper Pedersen,\n"
		      "Kalle Dalheimer, Klar\xC3\xA4lvdalens Datakonsult AB\n\n"
		      "(c) 2004, 2007 Marc Mutz, Klar\xC3\xA4lvdalens Datakonsult AB") )
{
  using ::authors;
  using ::credits;
  for ( unsigned int i = 0 ; i < sizeof authors / sizeof *authors ; ++i )
    addAuthor( ki18n(authors[i].name), ki18n(authors[i].desc),
	       authors[i].email, authors[i].web );
  for ( unsigned int i = 0 ; i < sizeof credits / sizeof *credits ; ++i )
    addCredit( ki18n(credits[i].name), ki18n(credits[i].desc),
	       credits[i].email, credits[i].web );
}
