/* -*- mode: c++; c-basic-offset:4 -*-
    selftest/registrycheck.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include "registrycheck.h"

#include "implementation_p.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QSettings>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace Kleo::_detail;
using namespace boost;

static QString gnupg_path = QLatin1String("HKEY_LOCAL_MACHINE\\Software\\GNU\\GnuPG");
static QString gnupg_key = QLatin1String("gpgProgram");

namespace {

    class RegistryCheck : public SelfTestImplementation {
    public:
        explicit RegistryCheck()
            : SelfTestImplementation( i18nc("@title", "Windows Registry") )
        {
            runTest();
        }

        void runTest() {

            m_passed = !QSettings( gnupg_path, QSettings::NativeFormat ).contains( gnupg_key );

            if ( m_passed )
                return;

            m_error = i18n("Obsolete registry entries found");

            m_explaination
                = i18nc( "@info",
                         "<para>Kleopatra detected an obsolete registry key (<resource>%1\\%2</resource>), "
                         "added by either a previous <application>Gpg4win</application> version or "
                         "applications such as <application>WinPT</application> or <application>EnigMail</application>.</para>"
                         "<para>Keeping the entry might lead to an old GnuPG backend being used.</para>",
                         gnupg_path, gnupg_key );
            m_proposedFix = i18nc( "@info",
                                   "<para>Delete registry key <resource>%1\\%2</resource>.</para>",
                                   gnupg_path, gnupg_key );

        }

        /* reimp */ bool canFixAutomatically() const { return true; }

        /* reimp */ bool fix() {

            QSettings settings( gnupg_path, QSettings::NativeFormat );
            if ( !settings.contains( gnupg_key ) )
                return true;

            settings.remove( gnupg_key );
            settings.sync();

            if ( settings.status() != QSettings::NoError ) {
                KMessageBox::error(
                    0,
                    i18nc("@info",
                          "Could not delete the registry key <resource>%1\\%2</resource>",
                          gnupg_path, gnupg_key ),
                    i18nc("@title", "Error Deleting Registry Key") );
                return false;
            }

            m_passed = true;
            m_error.clear();
            m_explaination.clear();
            m_proposedFix.clear();
            return true;
        }

    };
}

shared_ptr<SelfTest> Kleo::makeGpgProgramRegistryCheckSelfTest() {
    return shared_ptr<SelfTest>( new RegistryCheck );
}
