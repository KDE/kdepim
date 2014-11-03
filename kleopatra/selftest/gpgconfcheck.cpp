/* -*- mode: c++; c-basic-offset:4 -*-
    selftest/gpgconfcheck.cpp

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

#include "gpgconfcheck.h"

#include "implementation_p.h"

#include <utils/gnupg-helper.h>
#include <utils/hex.h>

#include <QDebug>
#include <KLocalizedString>

#include <QProcess>
#include <QDir>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace Kleo::_detail;
using namespace boost;

namespace
{

class GpgConfCheck : public SelfTestImplementation
{
    QString m_component;
public:
    explicit GpgConfCheck(const char *component)
        : SelfTestImplementation(i18nc("@title", "%1 Configuration Check", component  &&*component ? QLatin1String(component) : QLatin1String("gpgconf"))),
          m_component(QLatin1String(component))
    {
        runTest();
    }

    QStringList arguments() const
    {
        if (m_component.isEmpty()) {
            return QStringList() << QLatin1String("--check-config");
        } else {
            return QStringList() << QLatin1String("--check-options") << m_component ;
        }
    }

    bool canRun()
    {
        if (!ensureEngineVersion(GpgME::GpgConfEngine, 2, 0, 10)) {
            return false;
        }

        if (!m_component.isEmpty()) {
            return true;
        }

        QProcess gpgconf;
        gpgconf.setReadChannel(QProcess::StandardOutput);
        gpgconf.start(gpgConfPath(), QStringList() << QLatin1String("--list-dirs"), QIODevice::ReadOnly);
        gpgconf.waitForFinished();
        if (gpgconf.exitStatus() != QProcess::NormalExit || gpgconf.exitCode() != 0) {
            qDebug() << "GpgConfCheck: \"gpgconf --list-dirs\" gives error, disabling";
            return false;
        }
        const QList<QByteArray> lines = gpgconf.readAll().split('\n');
        Q_FOREACH (const QByteArray &line, lines)
            if (line.startsWith("sysconfdir:"))     //krazy:exclude=strings
                try {
                    return QDir(QFile::decodeName(hexdecode(line.mid(strlen("sysconfdir:"))))).exists(QLatin1String("gpgconf.conf"));
                } catch (...) {
                    return false;
                }
        qDebug() << "GpgConfCheck: \"gpgconf --list-dirs\" has no sysconfdir entry";
        return false;
    }

    void runTest()
    {

        if (!canRun()) {
            if (!m_skipped) {
                m_passed = true;
            }
            return;
        }

        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);

        process.start(gpgConfPath(), arguments(), QIODevice::ReadOnly);

        process.waitForFinished();

        const QString output = QString::fromUtf8(process.readAll());
        const QString message = process.exitStatus() == QProcess::CrashExit ? i18n("The process terminated prematurely") : process.errorString() ;

        if (process.exitStatus() != QProcess::NormalExit ||
                process.error()      != QProcess::UnknownError) {
            m_passed = false;
            m_error = i18nc("self-test did not pass", "Failed");
            m_explaination =
                i18n("There was an error executing the GnuPG configuration self-check for %2:\n"
                     "  %1\n"
                     "You might want to execute \"gpgconf %3\" on the command line.\n",
                     message, m_component.isEmpty() ? QLatin1String("GnuPG") : m_component, arguments().join(QLatin1String(" ")));
            if (!output.trimmed().isEmpty()) {
                m_explaination += QLatin1Char('\n') + i18n("Diagnostics:") + QLatin1Char('\n') + output ;
            }

            m_proposedFix.clear();
        } else if (process.exitCode()) {
            m_passed = false;
            m_error = i18nc("self-check did not pass", "Failed");
            m_explaination = !output.trimmed().isEmpty()
                             ? i18nc("Self-test did not pass",
                                     "The GnuPG configuration self-check failed.\n"
                                     "\n"
                                     "Error code: %1\n"
                                     "Diagnostics:", process.exitCode()) + QLatin1Char('\n') + output
                             : i18nc("self-check did not pass",
                                     "The GnuPG configuration self-check failed with error code %1.\n"
                                     "No output was received.", process.exitCode());
            m_proposedFix.clear();
        } else {
            m_passed = true;
        }
    }

};
}

shared_ptr<SelfTest> Kleo::makeGpgConfCheckConfigurationSelfTest(const char *component)
{
    return shared_ptr<SelfTest>(new GpgConfCheck(component));
}
