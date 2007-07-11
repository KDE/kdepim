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

#include "errorhandler.h"
#include "errorlog.h"
#include "errortypes/baseerror.h"

#include <KMessageBox>
#include <KGlobal>
#include <QStack>
#include <QMutex>

namespace KMobileTools {

class ErrorHandlerPrivate {
public:
    ErrorHandler* m_uniqueInstance;
    static QMutex m_mutex;
    static QStack<const BaseError*> m_errorStack;
};

QMutex ErrorHandlerPrivate::m_mutex;
QStack<const BaseError*> ErrorHandlerPrivate::m_errorStack;

K_GLOBAL_STATIC(ErrorHandlerPrivate, d)

ErrorHandler::ErrorHandler()
{
}


ErrorHandler::~ErrorHandler()
{
    qDeleteAll( d->m_errorStack.begin(), d->m_errorStack.end() );
}

ErrorHandler* ErrorHandler::instance() {
    // instance is automatically created
    return d->m_uniqueInstance;
}

void ErrorHandler::addError( const BaseError* error ) {
    /// @TODO implement me
    d->m_mutex.lock();

    // don't add the same error twice in a row
    if( !d->m_errorStack.isEmpty() ) {
        if( *(d->m_errorStack.top()) != *error )
            d->m_errorStack.push( error );
    } else
        d->m_errorStack.push( error );

    QString errorMessage = QString( "An error has just occurred:\nFile: %1\nLine: %2\n"
                                    "Description: %3\nDate/Time: %4\n Method: %5\nPriority: " )
                                    .arg( error->fileName() ).arg( error->lineNumber() )
                                    .arg( error->description() ).arg( error->dateTime().toString() )
                                    .arg( error->methodName() );

    QString priority;
    switch( error->priority() ) {
        case BaseError::Low:
            priority = "Low";
            break;

        case BaseError::Medium:
            priority = "Medium";
            break;

        case BaseError::High:
            priority = "High";
            break;
    }

    writeToLog( error );

    KMessageBox::error( 0, errorMessage + priority );

    d->m_mutex.unlock();
}

int ErrorHandler::errorCount() const {
    return d->m_errorStack.count();
}

void ErrorHandler::writeToLog( const BaseError* error ) {
    ErrorLog* errorLog = ErrorLog::instance();

    errorLog->write( QString( "===============================" ) );
    errorLog->write( QString( "Occurred on %1").arg( error->dateTime().toString( Qt::ISODate ) ) );
    errorLog->write( QString( "Location:    %1:%2" ).arg( error->fileName() )
                                                    .arg( QString::number( error->lineNumber() ) ) );
    errorLog->write( QString( "Method:      %1()" ).arg( error->methodName() ) );
    errorLog->write( QString( "Priority:    %1" ).arg( error->priority() ) );
    errorLog->write( QString( "Description: %1" ).arg( error->description() ) );
}

}
