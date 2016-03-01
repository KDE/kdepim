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

#include <utils/gnupg-helper.h>

#include <KLocalizedString>
#include <kiconloader.h>

#include <QPixmap>
#include <QFile>
#include <QVariant>

#include <cassert>

using namespace Kleo;

static const char kleopatra_version[] = KLEOPATRA_VERSION_STRING;
static const char description[] = I18N_NOOP("Certificate Manager and Unified Crypto GUI");

struct about_data {
    const char *name;
    const char *desc;
    const char *email;
    const char *web;
};

static const about_data authors[] = {
    { "Andre Heinecke", I18N_NOOP("Current Maintainer"), "aheinecke@gnupg.org", 0 },
    { "Marc Mutz", I18N_NOOP("Former Maintainer"), "mutz@kde.org", 0 },
    { "Steffen Hansen", I18N_NOOP("Former Maintainer"), "hansen@kde.org", 0 },
    { "Matthias Kalle Dalheimer", I18N_NOOP("Original Author"), "kalle@kde.org", 0 },
};

static const about_data credits[] = {
    {
        I18N_NOOP("David Faure"),
        I18N_NOOP("Backend configuration framework, KIO integration"),
        "faure@kde.org", 0
    },
    {
        I18N_NOOP("Michel Boyer de la Giroday"),
        I18N_NOOP("Key-state dependent colors and fonts in the certificates list"),
        "michel@klaralvdalens-datakonsult.se", 0
    },
    {
        I18N_NOOP("Thomas Moenicke"),
        I18N_NOOP("Artwork"),
        "tm@php-qt.org", 0
    },
    {
        I18N_NOOP("Frank Osterfeld"),
        I18N_NOOP("Resident gpgme/win wrangler, UI Server commands and dialogs"),
        "osterfeld@kde.org", 0
    },
    {
        I18N_NOOP("Karl-Heinz Zimmer"),
        I18N_NOOP("DN display ordering support, infrastructure"),
        "khz@kde.org", 0
    },
    {
        I18N_NOOP("Laurent Montel"),
        I18N_NOOP("Qt5 port, general code maintenance"),
        "montel@kde.org", 0
    },
};

AboutData::AboutData()
    : KAboutData(QStringLiteral("kleopatra"), i18n("Kleopatra"),
                 QLatin1String(kleopatra_version), i18n(description), KAboutLicense::GPL,
                 i18n("(c) 2002 Steffen\xC2\xA0Hansen, Matthias\xC2\xA0Kalle\xC2\xA0" "Dalheimer, Klar\xC3\xA4lvdalens\xC2\xA0" "Datakonsult\xC2\xA0" "AB\n"
                      "(c) 2004, 2007, 2008, 2009 Marc\xC2\xA0Mutz, Klar\xC3\xA4lvdalens\xC2\xA0" "Datakonsult\xC2\xA0" "AB") +
                 "\n" + i18n("(c) 2010-2016 The Kleopatra developers"))
{
    using ::authors;
    using ::credits;
    for (unsigned int i = 0; i < sizeof authors / sizeof * authors; ++i)
        addAuthor(i18n(authors[i].name), i18n(authors[i].desc),
                  QLatin1String(authors[i].email), QLatin1String(authors[i].web));
    for (unsigned int i = 0; i < sizeof credits / sizeof * credits; ++i)
        addCredit(i18n(credits[i].name), i18n(credits[i].desc),
                  QLatin1String(credits[i].email), QLatin1String(credits[i].web));
}

static const char gpg4win_description[] = I18N_NOOP("Gpg4win is an installer package for Windows for EMail and "
        "file encryption using the core component GnuPG for Windows. "
        "Both relevant cryptography standards are supported, OpenPGP "
        "and S/MIME. Gpg4win and the software included with Gpg4win "
        "are Free Software.");

static const char gpg4win_credits_description[] = I18N_NOOP("This Free Software product was developed mostly as part of "
        "commercial contracts by the following companies:"
        "<ul><li><a href='http://intevation.net'>Intevation GmbH</a> "
        "(Projekt coordination, QA, Compendium)</li>"
        "<li><a href='http://www.g10code.com'>g10 Code GmbH</a> "
        "(Crypto Funktionality, GpgOL, GpgEX, GPA)</li> "
        "<li><a href='http://www.kdab.com'>KDAB</a> (Kleopatra)</li></ul>"
        "Contributors:<br><br>");

static const char gpg4win_credits_persons[] = I18N_NOOP("Till Adam<br>"
        "Marcus Brinkmann<br>"
        "Brigitte Hamilton<br>"
        "Andre Heinecke<br>"
        "Bernhard Herzog<br>"
        "Werner Koch<br>"
        "Colin Leroy<br>"
        "Marc Mutz<br>"
        "Marcel Newmann<br>"
        "Frank Osterfeld<br>"
        "Bernhard Reiter<br>"
        "Florian v. Samson<br>"
        "Emanuel Schütze<br>"
        "Dr. Jan-Oliver Wagner");

static QString gpg4win_version()
{
    QFile versionFile(gpg4winInstallPath() + QStringLiteral("/../VERSION"));
    if (!versionFile.open(QIODevice::ReadOnly)) {
        // No need to translate this should only be the case in development
        // builds.
        return QStringLiteral("Unknown (no VERSION file found)");
    }
    const QString g4wTag = QString::fromUtf8(versionFile.readLine());
    if (!g4wTag.startsWith(QStringLiteral("gpg4win"))) {
        // Hu? Something unknown
        return QStringLiteral("Unknown (invalid VERSION file found)");
    }
    // Next line is version.
    return QString::fromUtf8(versionFile.readLine());
}

static QPixmap UserIcon_nocached2(const char *name)
{
    // KIconLoader insists on caching all pixmaps. Since the splash
    // screen is a particularly large 'icon' and used only once,
    // caching is unneccesary and just hurts startup performance.
    KIconLoader *const il = KIconLoader::global();
    assert(il);
    const QString iconPath = il->iconPath(QLatin1String(name), KIconLoader::User);
    return iconPath.isEmpty() ? il->unknown() : QPixmap(iconPath);
}

AboutGpg4WinData::AboutGpg4WinData()
    : KAboutData(QStringLiteral("gpg4win"), i18n("Gpg4win"),
                 gpg4win_version(), i18n(gpg4win_description),
                 KAboutLicense::GPL, QString(), QString(), QStringLiteral("http://www.gpg4win.de"))
{
    setCustomAuthorText(i18n(gpg4win_credits_description),
                        i18n(gpg4win_credits_description) + i18n(gpg4win_credits_persons));
    // We need at least one Author even if it is empty to get the Author tab
    // in the about dialog.
    addAuthor(QString(), QString(), QString(), QString());
    setProgramLogo(UserIcon_nocached2("gpg4win"));
}
