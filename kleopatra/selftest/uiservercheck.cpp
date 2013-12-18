/* -*- mode: c++; c-basic-offset:4 -*-
    selftest/uiservercheck.cpp

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

#include "uiservercheck.h"

#include "implementation_p.h"

#include <utils/getpid.h>

#include <libkleopatraclient/core/command.h>

#include <QTextDocument> // for Qt::escape
#include <QEventLoop>

#include <KLocale>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace Kleo::_detail;
using namespace boost;

namespace {

    class UiServerCheck : public SelfTestImplementation {
    public:
        explicit UiServerCheck()
            : SelfTestImplementation( i18nc("@title", "UiServer Connectivity") )
        {
            runTest();
        }

        void runTest() {

            KleopatraClientCopy::Command command;

            {
                QEventLoop loop;
                loop.connect( &command, SIGNAL(finished()), SLOT(quit()) );
                QMetaObject::invokeMethod( &command, "start", Qt::QueuedConnection );
                loop.exec();
            }

            if ( command.error() ) {
                m_passed = false;
                m_error = i18n("not reachable");
                m_explaination = i18nc("@info",
                                       "Could not connect to UiServer: <message>%1</message>",
                                       Qt::escape( command.errorString() ) );
                m_proposedFix = i18nc("@info",
                                      "<para>Check that your firewall is not set to block local connections "
                                      "(allow connections to <resource>localhost</resource> or <resource>127.0.0.1</resource>).</para>");
            } else if ( command.serverPid() != mygetpid() ) {
                m_passed = false;
                m_error = i18n("multiple instances");
                m_explaination = i18nc("@info",
                                       "It seems another <application>Kleopatra</application> is running (with process-id %1)",
                                       command.serverPid() );
                m_proposedFix = i18nc("@info",
                                      "Quit any other running instances of <application>Kleopatra</application>.");
            } else {
                m_passed = true;
            }

        }

    };
}

shared_ptr<SelfTest> Kleo::makeUiServerConnectivitySelfTest() {
    return shared_ptr<SelfTest>( new UiServerCheck );
}
