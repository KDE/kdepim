/* -*- mode: c++; c-basic-offset:4 -*-
    selftest/gpgagentcheck.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#include "gpgagentcheck.h"

#include "implementation_p.h"

#include <utils/getpid.h>

#include <gpgme++/context.h>
#include <gpgme++/assuanresult.h>
#include <gpgme++/gpgagentgetinfoassuantransaction.h>

#include <QTextDocument> // for Qt::escape

#include <KLocalizedString>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace Kleo::_detail;
using namespace boost;
using namespace GpgME;

namespace
{

class GpgAgentCheck : public SelfTestImplementation
{
public:
    explicit GpgAgentCheck()
        : SelfTestImplementation(i18nc("@title", "Gpg-Agent Connectivity"))
    {
        runTest();
    }

    void runTest()
    {

        m_skipped = true;

        if (!hasFeature(AssuanEngineFeature)) {
            m_error = i18n("GpgME library too old");
            m_explaination = i18nc("@info",
                                   "Either the GpgME library itself is too old, "
                                   "or the GpgME++ library was compiled against "
                                   "an older GpgME that did not support connecting to gpg-agent.");
            m_proposedFix = xi18nc("@info",
                                   "Upgrade to <application>gpgme</application> 1.2.0 or higher, "
                                   "and ensure that gpgme++ was compiled against it.");
        } else {

            Error error;
            const std::auto_ptr<Context> ctx = Context::createForEngine(AssuanEngine, &error);
            if (!ctx.get()) {
                m_error = i18n("GpgME does not support gpg-agent");
                m_explaination = xi18nc("@info",
                                        "<para>The <application>GpgME</application> library is new "
                                        "enough to support <application>gpg-agent</application>, "
                                        "but does not seem to do so in this installation.</para>"
                                        "<para>The error returned was: <message>%1</message>.</para>",
                                        QString::fromLocal8Bit(error.asString()).toHtmlEscaped());
                // PENDING(marc) proposed fix?
            } else {

                m_skipped = false;

                const AssuanResult result = ctx->assuanTransact("GETINFO version");
                if (result.error()) {
                    m_passed = false;
                    m_error = i18n("not reachable");
                    m_explaination = xi18nc("@info",
                                            "Could not connect to GpgAgent: <message>%1</message>",
                                            QString::fromLocal8Bit(result.error().asString()).toHtmlEscaped());
                    m_proposedFix = xi18nc("@info",
                                           "<para>Check that gpg-agent is running and that the "
                                           "<environment>GPG_AGENT_INFO</environment> variable is set and up-to-date.</para>");
                } else if (result.assuanError()) {
                    m_passed = false;
                    m_error = i18n("unexpected error");
                    m_explaination = xi18nc("@info",
                                            "<para>Unexpected error while asking <application>gpg-agent</application> "
                                            "for its version.</para>"
                                            "<para>The error returned was: <message>%1</message>.</para>",
                                            QString::fromLocal8Bit(result.assuanError().asString()).toHtmlEscaped());
                    // PENDING(marc) proposed fix?
                } else {
                    m_passed = true;
                }
            }
        }
    }

};
}

shared_ptr<SelfTest> Kleo::makeGpgAgentConnectivitySelfTest()
{
    return shared_ptr<SelfTest>(new GpgAgentCheck);
}
