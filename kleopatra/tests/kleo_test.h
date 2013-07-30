/*
    This file is part of Kleopatra's test suite.
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#ifndef KLEO_TEST_H
#define KLEO_TEST_H

#include <qtest_kde.h>

#ifndef KLEO_TEST_GNUPGHOME
#error KLEO_TEST_GNUPGHOME not defined!
#endif

// based on qtest_kde.h
#define QTEST_KLEOMAIN(TestObject, flags) \
int main(int argc, char *argv[]) \
{ \
    setenv("GNUPGHOME", KLEO_TEST_GNUPGHOME, 1 ); \
    setenv("LC_ALL", "C", 1); \
    setenv("KDEHOME", QFile::encodeName( QDir::homePath() + QLatin1String("/.kde-unit-test") ), 1); \
    KAboutData aboutData( "qttest", 0, ki18n("qttest"), "version" );  \
    KDEMainFlags mainFlags = flags;                         \
    KComponentData cData(&aboutData); \
    QApplication app( argc, argv, (mainFlags & GUI) != 0 ); \
    app.setApplicationName( QLatin1String("qttest") ); \
    TestObject tc; \
    return QTest::qExec( &tc, argc, argv ); \
}

#endif
