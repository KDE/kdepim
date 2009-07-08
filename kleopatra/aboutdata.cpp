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
#include <version-kleopatra.h>

#include "aboutdata.h"

#include <klocale.h>
#include <kiconloader.h>

#include <QPixmap>
#include <QVariant>

#include <cassert>

static const char kleopatra_version[] = KLEOPATRA_VERSION_STRING;
static const char description[] = I18N_NOOP("Certificate Manager and Unified Crypto GUI");

struct about_data {
  const char * name;
  const char * desc;
  const char * email;
  const char * web;
};

static const about_data authors[] = {
  { "Marc Mutz", I18N_NOOP("Current Maintainer"), "mutz@kde.org", 0 },
  { "Steffen Hansen", I18N_NOOP("Former Maintainer"), "hansen@kde.org", 0 },
  { "Matthias Kalle Dalheimer", I18N_NOOP("Original Author"), "kalle@kde.org", 0 },
};


static const about_data credits[] = {
  { I18N_NOOP("David Faure"),
    I18N_NOOP("Backend configuration framework, KIO integration"),
    "faure@kde.org", 0 },
  { I18N_NOOP("Michel Boyer de la Giroday"),
    I18N_NOOP("Key-state dependant colors and fonts in the key list"),
    "michel@klaralvdalens-datakonsult.se", 0 },
  { I18N_NOOP("Thomas Moenicke"),
    I18N_NOOP("Artwork"),
    "tm@php-qt.org", 0 },
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
		ki18n("(c) 2002 Steffen\xC2\xA0Hansen, Matthias\xC2\xA0Kalle\xC2\xA0" "Dalheimer, Klar\xC3\xA4lvdalens\xC2\xA0" "Datakonsult\xC2\xA0" "AB\n"
		      "(c) 2004, 2007, 2008, 2009 Marc\xC2\xA0Mutz, Klar\xC3\xA4lvdalens\xC2\xA0" "Datakonsult\xC2\xA0" "AB") )
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


static const char gpg4win_description[] = I18N_NOOP( "Gpg4win is an installer package for Windows for EMail and "
                                                     "file encryption using the core component GnuPG for Windows. "
                                                     "Both relevant cryptography standards are supported, OpenPGP "
                                                     "and S/MIME. Gpg4win and the software included with Gpg4win "
                                                     "are Free Software.");
static const char gpg4win_version[] = "2.0.0"; // ### make this better come from somewhere...


static QPixmap UserIcon_nocached2( const char * name ) {
    // KIconLoader insists on caching all pixmaps. Since the splash
    // screen is a particularly large 'icon' and used only once,
    // caching is unneccesary and just hurts startup performance.
    KIconLoader * const il = KIconLoader::global();
    assert( il );
    const QString iconPath = il->iconPath( QLatin1String( name ), KIconLoader::User );
    return iconPath.isEmpty() ? il->unknown() : QPixmap( iconPath ) ;
}


AboutGpg4WinData::AboutGpg4WinData()
    : KAboutData( "gpg4win", 0, ki18n("Gpg4win"),
                  gpg4win_version, ki18n(gpg4win_description),
                  License_GPL, KLocalizedString(), KLocalizedString(), "http://www.gpg4win.de" )
{
    addAuthor( ki18n("Intevation GmbH (Project Management)"), KLocalizedString(), 0, "http://www.intevation.de" );
    addAuthor( ki18n("g\xC2\xB9\xC2\xBA" "code GmbH (Crypto Functionality, GpgOL, GpgEX, GPA)"), KLocalizedString(), 0, "http://www.g10code.de" );
    addAuthor( ki18n("KDAB (Kleopatra)"), KLocalizedString(), 0, "http://www.kdab.com" );
    setCustomAuthorText( ki18n("Gpg4win is being developed by the following companies:"),
                         ki18n("Gpg4win is being developed by the following companies:") );
    setProgramLogo( UserIcon_nocached2( "gpg4win" ) );
}
