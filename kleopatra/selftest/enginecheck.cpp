/* -*- mode: c++; c-basic-offset:4 -*-
    selftest/enginecheck.cpp

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

#include "enginecheck.h"

#include "implementation_p.h"

#include <gpgme++/global.h>
#include <gpgme++/engineinfo.h>
#include <gpgme++/error.h>

#include <gpg-error.h>

#include <KLocalizedString>
#include <QDebug>

#include <QFile>
#include <QRegExp>

#include <boost/shared_ptr.hpp>
#include <boost/range.hpp>

#include <algorithm>
#include <cassert>

using namespace Kleo;
using namespace Kleo::_detail;
using namespace GpgME;
using namespace boost;

static QString engine_name(GpgME::Engine eng)
{
    static const char *engines[] = {
        "gpg", "gpgsm", "gpgconf"
    };
    return QString::fromLatin1(engines[eng]);
}

static QString test_name(GpgME::Engine eng)
{
    static const char *names[] = {
        I18N_NOOP2("@title", "GPG (OpenPGP Backend) installation"),
        I18N_NOOP2("@title", "GpgSM (S/MIME Backend) installation"),
        I18N_NOOP2("@title", "GpgConf (Configuration) installation"),
    };
    return i18nc("@title", names[eng]);
}

namespace
{

class EngineCheck : public SelfTestImplementation
{
public:
    explicit EngineCheck(GpgME::Engine eng)
        : SelfTestImplementation(test_name(eng))
    {
        runTest(eng);
    }

    void runTest(GpgME::Engine eng)
    {
        const Error err = GpgME::checkEngine(eng);
        assert(!err.code() || err.code() == GPG_ERR_INV_ENGINE);

        m_passed = !err;

        if (m_passed) {
            return;
        }

        m_explaination = xi18nc("@info",
                                "<para>A problem was detected with the <application>%1</application> backend.</para>",
                                engine_name(eng));

        const EngineInfo ei = engineInfo(eng);
        if (ei.isNull()) {
            m_error = i18n("not supported");
            m_explaination += xi18nc("@info",
                                     "<para>It seems that the <icode>gpgme</icode> library was compiled without "
                                     "support for this backend.</para>");
            m_proposedFix += xi18nc("@info",
                                    "<para>Replace the <icode>gpgme</icode> library with a version compiled "
                                    "with <application>%1</application> support.</para>",
                                    engine_name(eng));
        } else if (ei.fileName() && !ei.version()) {
            m_error = i18n("not properly installed");
            m_explaination += xi18nc("@info",
                                     "<para>Backend <command>%1</command> is not installed properly.</para>", QFile::decodeName(ei.fileName()));
            m_proposedFix += xi18nc("@info",
                                    "<para>Please check the output of <command>%1 --version</command> manually.</para>",
                                    QFile::decodeName(ei.fileName()));
        } else if (ei.fileName() && ei.version() && ei.requiredVersion()) {
            m_error = i18n("too old");
            m_explaination += xi18nc("@info",
                                     "<para>Backend <command>%1</command> is installed in version %2, "
                                     "but at least version %3 is required.</para>",
                                     QFile::decodeName(ei.fileName()),
                                     QString::fromUtf8(ei.version()),
                                     QString::fromUtf8(ei.requiredVersion()));
            m_proposedFix += xi18nc("@info",
                                    "<para>Install <application>%1</application> version %2 or higher.</para>",
                                    engine_name(eng), QString::fromUtf8(ei.requiredVersion()));
        } else {
            m_error = m_explaination = i18n("unknown problem");
            m_proposedFix += xi18nc("@info",
                                    "<para>Make sure <application>%1</application> is installed and "
                                    "in <envar>PATH</envar>.</para>",
                                    engine_name(eng));
        }
    }

};
}

shared_ptr<SelfTest> Kleo::makeGpgEngineCheckSelfTest()
{
    return shared_ptr<SelfTest>(new EngineCheck(GpgME::GpgEngine));
}

shared_ptr<SelfTest> Kleo::makeGpgSmEngineCheckSelfTest()
{
    return shared_ptr<SelfTest>(new EngineCheck(GpgME::GpgSMEngine));
}

shared_ptr<SelfTest> Kleo::makeGpgConfEngineCheckSelfTest()
{
    return shared_ptr<SelfTest>(new EngineCheck(GpgME::GpgConfEngine));
}

//
// SelfTestImplementation (parts)
//

static bool is_version(const char *actual, int major, int minor, int patch)
{
    QRegExp rx(QLatin1String("(\\d+)\\.(\\d+)\\.(\\d+)(?:-svn\\d+)?.*"));
    if (!rx.exactMatch(QString::fromUtf8(actual))) {
        qDebug() << "Can't parse version " << actual;
        return false;
    }
    bool ok;
    int actual_version[3];
    for (int i = 0 ; i < 3 ; ++i) {
        ok = false;
        actual_version[i] = rx.cap(i + 1).toUInt(&ok);
        assert(ok);
    }

    qDebug() << "Parsed" << actual << "as: "
             << actual_version[0] << '.'
             << actual_version[1] << '.'
             << actual_version[2] << '.' ;

    const int required_version[] = { major, minor, patch };

    // return ! ( actual_version < required_version )
    ok = !std::lexicographical_compare(begin(actual_version), end(actual_version),
                                       begin(required_version), end(required_version));
    if (ok) {
        qDebug()  << QString::fromLatin1("%1.%2.%3").arg(major).arg(minor).arg(patch) << "<=" << actual ;
    } else {
        qDebug() << QString::fromLatin1("%1.%2.%3").arg(major).arg(minor).arg(patch) << ">" << actual ;
    }
    return ok;
}

bool SelfTestImplementation::ensureEngineVersion(GpgME::Engine engine, int major, int minor, int patch)
{
    const Error err = GpgME::checkEngine(engine);
    assert(!err || err.code() == GPG_ERR_INV_ENGINE);

    const EngineInfo ei = GpgME::engineInfo(engine);

    m_skipped = err || !is_version(ei.version(), major, minor, patch);

    if (!m_skipped) {
        return true;
    }

    if (!err && ei.version()) {
        // properly installed, but too old
        m_explaination = xi18nc("@info",
                                "<para><application>%1</application> v%2.%3.%4 is required for this test, but only %5 is installed.</para>",
                                engine_name(engine), major, minor, patch, QString::fromUtf8(ei.version()));
        m_proposedFix += xi18nc("@info",
                                "<para>Install <application>%1</application> version %2 or higher.</para>",
                                engine_name(engine), QString::fromLatin1("%1.%2.%3").arg(major).arg(minor).arg(patch));
    } else {
        // not properly installed
        m_explaination = xi18nc("@info",
                                "<para><application>%1</application> is required for this test, but does not seem available.</para>"
                                "<para>See tests further up for more information.</para>",
                                engine_name(engine));
        m_proposedFix = xi18nc("@info %1: test name",
                               "<para>See \"%1\" above.</para>", test_name(engine));
    }

    return false;
}

