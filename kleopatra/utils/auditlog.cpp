/* -*- mode: c++; c-basic-offset:4 -*-
    utils/auditlog.cpp

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

#include "auditlog.h"

#include <kleo/job.h>

#include <KUrl>
#include <KDebug>
#include <KLocalizedString>

using namespace Kleo;

AuditLog AuditLog::fromJob( const Job * job ) {
    if ( job )
        return AuditLog( job->auditLogAsHtml(), job->auditLogError() );
    else
        return AuditLog();
}

QString AuditLog::formatLink( const KUrl & urlTemplate ) const {
    // more or less the same as
    // kmail/objecttreeparser.cpp:makeShowAuditLogLink(), so any bug
    // fixed here eqally applies there:
    if ( const unsigned int code = m_error.code() ) {
        if ( code == GPG_ERR_NOT_IMPLEMENTED ) {
            kDebug() << "not showing link (not implemented)";
            return QString();
        } else if ( code == GPG_ERR_NO_DATA ) {
            kDebug() << "not showing link (not available)";
            return i18n("No Audit Log available");
        } else {
            return i18n("Error Retrieving Audit Log: %1", QString::fromLocal8Bit( m_error.asString() ) );
        }
    }

    if ( !m_text.isEmpty() ) {
        KUrl url = urlTemplate;
        url.addQueryItem( QLatin1String("log"), m_text );
        return QLatin1String("<a href=\"") + url.url() + QLatin1String("\">") + i18nc("The Audit Log is a detailed error log from the gnupg backend", "Show Audit Log") + QLatin1String("</a>");
    }

    return QString();
}
