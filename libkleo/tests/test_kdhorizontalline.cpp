/*
    test_kdhorizontalline.cpp

    This file is part of libkleopatra's test suite.
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
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

#include "libkleo/ui/kdhorizontalline.h"


#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
int main( int argc, char * argv[] ) {

    QApplication app( argc, argv );

    QWidget w;
    QGridLayout glay( &w );

    KDHorizontalLine hl1( "Foo", &w );
    glay.addWidget( &hl1, 0, 0, 1, 2 );

    QLabel lb1( "Foo 1:", &w );
    glay.addWidget( &lb1, 1, 0 );
    QLineEdit le1( &w );
    glay.addWidget( &le1, 1, 1 );

    glay.setColumnStretch( 1, 1 );
    glay.setRowStretch( 2, 1 );

    w.show();

    return app.exec();
}
