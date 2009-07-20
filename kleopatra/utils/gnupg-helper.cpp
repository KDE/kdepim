/* -*- mode: c++; c-basic-offset:4 -*-
    utils/gnupg-helper.cpp

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

#include "gnupg-helper.h"

#include <gpgme++/engineinfo.h>

#include <KStandardDirs>

#include <QDir>
#include <QFile>
#include <QString>

#include <gpg-error.h>

#ifdef Q_OS_WIN
#include "gnupg-registry.h"
#endif // Q_OS_WIN

QString Kleo::gnupgHomeDirectory()
{
#ifdef Q_OS_WIN
    return QFile::decodeName( default_homedir() );
#else
    const QByteArray gnupgHome = qgetenv( "GNUPGHOME" );
    if ( !gnupgHome.isEmpty() )
        return QFile::decodeName( gnupgHome );
    else
        return QDir::homePath() + "/.gnupg";
#endif
}

int Kleo::makeGnuPGError( int code ) {
    return gpg_error( static_cast<gpg_err_code_t>( code ) );
}

static QString findGpgExe( GpgME::Engine engine, const char * exe ) {
    const GpgME::EngineInfo info = GpgME::engineInfo( engine );
    return info.fileName() ? QFile::decodeName( info.fileName() ) : KStandardDirs::findExe( exe ) ;
}

QString Kleo::gpgConfPath() {
    return findGpgExe( GpgME::GpgConfEngine, "gpgconf" );
}

QString Kleo::gpgSmPath() {
    return findGpgExe( GpgME::GpgSMEngine, "gpgsm" );
}

QString Kleo::gpgPath() {
    return findGpgExe( GpgME::GpgEngine, "gpg" );
}

QStringList Kleo::gnupgFileBlacklist() {
    return QStringList()
        << "dirmngr-cache.d"
        << "S.uiserver"
        << "S.gpg-agent"
        << "random_seed"
        << "*~"
        << "*.bak"
        << "*.lock"
        << "*.tmp"
        << "reader_*.status"
        ;
}
