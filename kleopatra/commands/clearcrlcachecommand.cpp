/* -*- mode: c++; c-basic-offset:4 -*-
    commands/clearcrlcachecommand.cpp

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

#include "clearcrlcachecommand.h"

#include <KLocalizedString>

using namespace Kleo;
using namespace Kleo::Commands;

ClearCrlCacheCommand::ClearCrlCacheCommand( KeyListController * c )
    : GnuPGProcessCommand( c )
{

}

ClearCrlCacheCommand::ClearCrlCacheCommand( QAbstractItemView * v, KeyListController * c )
    : GnuPGProcessCommand( v, c )
{

}

ClearCrlCacheCommand::~ClearCrlCacheCommand() {}

QStringList ClearCrlCacheCommand::arguments() const {
    return QStringList() << QLatin1String("dirmngr") << QLatin1String("--flush");
    //return QStringList() << "gpgsm" << "--call-dirmngr" << "flush";
}

QString ClearCrlCacheCommand::errorCaption() const {
    return i18n( "Clear CRL Cache Error" );
}

QString ClearCrlCacheCommand::successCaption() const {
    return i18n( "Clear CRL Cache Finished" );
}

QString ClearCrlCacheCommand::crashExitMessage( const QStringList & args ) const {
    return i18n( "The DirMngr process that tried to clear the CRL cache "
                 "ended prematurely because of an unexpected error. "
                 "Please check the output of %1 for details.", args.join( QLatin1String(" ") ) ) ;
}

QString ClearCrlCacheCommand::errorExitMessage( const QStringList & args ) const {
    return i18n( "An error occurred while trying to clear the CRL cache. "
                 "The output from %1 was:\n%2", args[0], errorString() );
}

QString ClearCrlCacheCommand::successMessage( const QStringList & ) const {
    return i18n( "CRL cache cleared successfully." );
}

