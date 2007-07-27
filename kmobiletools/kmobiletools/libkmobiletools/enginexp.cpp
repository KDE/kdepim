/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "enginexp.h"

#include <KLibFactory>

#include <libkmobiletools/errorhandler.h>

namespace KMobileTools {

EngineXP::EngineXP( QObject *parent )
 : QObject( parent )
{
}


EngineXP::~EngineXP()
{
}


QString EngineXP::shortDescription() const
{
    /// @TODO implement me
    return QString();
}

QString EngineXP::longDescription() const
{
    /// @TODO implement me
    return QString();
}

bool EngineXP::implements( const QString& interfaceName ) {
    QString qualifiedInterfaceName = QString( "KMobileTools::Ifaces::%1" ).arg( interfaceName );
    if( inherits( qualifiedInterfaceName.toUtf8() ) )
        return true;

    return false;
}

EngineXP* EngineXP::load( QObject* parent, const QString& libname ) {
    KLibFactory *factory = KLibLoader::self()->factory( libname.toUtf8() );

    if( !factory ) {
        /// @TODO replace BaseError with appropriate error type
        // error message can be obtained by calling KLibLoader::self()->lastErrorMessage()
        ErrorHandler::instance()->addError( new BaseError(ERROR_META_INFO) );
        return 0;
    }

    EngineXP *engine = static_cast<KMobileTools::EngineXP*>( factory->create( parent, "KMobileTools::EngineXP" ) );

    return engine;
}

}

#include "enginexp.moc"

