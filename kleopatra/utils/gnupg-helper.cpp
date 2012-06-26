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

#include "utils/hex.h"

#include <gpgme++/engineinfo.h>

#include <KDebug>
#include <KStandardDirs>

#include <QDir>
#include <QFile>
#include <QString>
#include <QProcess>
#include <QByteArray>

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

QString Kleo::gpg4winInstallPath() {
#ifndef _WIN32_WCE
    return gpgConfListDir( "bindir" );
#else
    HKEY hKey;
    TCHAR *lszValue;
    DWORD dwType=REG_SZ;
    DWORD dwSize;

    if ( ERROR_SUCCESS!=RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"Software\\GNU\\GnuPG", 0, KEY_READ, &hKey ) ){
      return QString();
    }

    if ( ERROR_SUCCESS!=RegQueryValueExW ( hKey, L"Install Directory", NULL, NULL, NULL, &dwSize ) ){
      return QString();
    }

    lszValue = new TCHAR[dwSize];

    if ( ERROR_SUCCESS!=RegQueryValueExW ( hKey,L"Install Directory", NULL, &dwType, ( LPBYTE ) lszValue, &dwSize ) ) {
        delete [] lszValue;
        return QString();
    }
    RegCloseKey ( hKey );

    QString res = QString::fromUtf16 ( ( const ushort* ) lszValue );
    delete [] lszValue;

    return res;
#endif
}

QString Kleo::gpgConfListDir( const char * which ) {
    if ( !which || !*which )
        return QString();
    const QString gpgConfPath = Kleo::gpgConfPath();
    if ( gpgConfPath.isEmpty() )
        return QString();
    QProcess gpgConf;
    kDebug() << "gpgConfListDir: starting " << qPrintable( gpgConfPath ) << " --list-dirs";
    gpgConf.start( gpgConfPath, QStringList() << QLatin1String( "--list-dirs" ) );
    if ( !gpgConf.waitForFinished() ) {
        kDebug() << "gpgConfListDir(): failed to execute gpgconf: " << qPrintable( gpgConf.errorString() );
        kDebug() << "output was:" << endl << gpgConf.readAllStandardError().constData();
        return QString();
    }
    const QList<QByteArray> lines = gpgConf.readAllStandardOutput().split( '\n' );
    Q_FOREACH( const QByteArray & line, lines )
        if ( line.startsWith( which ) && line[qstrlen(which)] == ':' ) {
            const int begin = qstrlen(which) + 1;
            int end = line.size();
            while ( end && ( line[end-1] == '\n' || line[end-1] == '\r' ) )
                --end;
            const QString result = QDir::fromNativeSeparators( QFile::decodeName( hexdecode( line.mid( begin, end - begin ) ) ) );
            kDebug() << "gpgConfListDir: found " << qPrintable( result )
                     << " for '" << which << "'entry";
            return result;
        }
    kDebug() << "gpgConfListDir(): didn't find '" << which << "'"
             << "entry in output:" << endl << gpgConf.readAllStandardError().constData();
    return QString();
}
