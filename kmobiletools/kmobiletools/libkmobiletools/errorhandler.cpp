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

#include <KMessageBox>
#include <KPassivePopup>
#include <KGlobal>
#include <QMutex>

namespace KMobileTools {

class ErrorHandlerInstance {
public:
    ErrorHandler m_uniqueInstance;
};

class ErrorHandlerPrivate {
public:
    QMutex m_mutex;
    QStack<const BaseError*> m_errorStack;

    ~ErrorHandlerPrivate() {
        qDeleteAll( m_errorStack.begin(), m_errorStack.end() );
    }
};

K_GLOBAL_STATIC(ErrorHandlerInstance, errorHandlerInstance)

ErrorHandler::ErrorHandler()
: QObject( 0 ), d( new ErrorHandlerPrivate )
{
}


ErrorHandler::~ErrorHandler()
{
}

ErrorHandler* ErrorHandler::instance() {
    // instance is automatically created
    return &errorHandlerInstance->m_uniqueInstance;
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

    displayError( error );
    writeToLog( error );

    d->m_mutex.unlock();
}

int ErrorHandler::errorCount() const {
    return d->m_errorStack.count();
}

QStack<const BaseError*> ErrorHandler::errorStack() {
    return d->m_errorStack;
}

void ErrorHandler::displayError( const BaseError* error ) {
    if( receivers( SIGNAL( errorOccurred( const BaseError* ) ) ) > 0 )
        emit errorOccurred( error->description(), error->priority() );
    else
        KMessageBox::error( 0, error->description() );
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

#include "errorhandler.moc"
