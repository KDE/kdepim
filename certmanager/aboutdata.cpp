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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "aboutdata.h"

#include <klocale.h>

static const char kleopatra_version[] = "0.40";
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
  { "David Faure",
    I18N_NOOP("Backend configuration framework, KIO integration"),
    "faure@kde.org", 0 },
  { "Michel Boyer de la Giroday",
    I18N_NOOP("Key-state dependant colors and fonts in the key list"),
    "michel@klaralvdalens-datakonsult.se", 0 },
  { "Daniel Molkentin",
    I18N_NOOP("Certificate Wizard KIOSK integration, infrastructure"),
    "molkentin@kde.org", 0 },
  { "Ralf Nolden",
    I18N_NOOP("Support for obsolete EMAIL RDN in Certificate Wizard"),
    "nolden@kde.org", 0 },
  { "Karl-Heinz Zimmer",
    I18N_NOOP("DN display ordering support, infrastructure"),
    "khz@kde.org", 0 },
};


AboutData::AboutData()
  : KAboutData( "kleopatra", I18N_NOOP("Kleopatra"),
		kleopatra_version, description, License_GPL,
		"(c) 2002 Steffen Hansen, Jesper Pedersen,\n"
		"Kalle Dalheimer, Klar\xC3\xA4lvdalens Datakonsult AB\n\n"
		"(c) 2004 Marc Mutz, Klar\xC3\xA4lvdalens Datakonsult AB" )
{
  using ::authors;
  using ::credits;
  for ( unsigned int i = 0 ; i < sizeof authors / sizeof *authors ; ++i )
    addAuthor( authors[i].name, authors[i].desc,
	       authors[i].email, authors[i].web );
  for ( unsigned int i = 0 ; i < sizeof credits / sizeof *credits ; ++i )
    addCredit( credits[i].name, credits[i].desc,
	       credits[i].email, credits[i].web );
}
