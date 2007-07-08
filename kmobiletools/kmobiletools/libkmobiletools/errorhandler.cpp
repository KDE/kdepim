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

#include <kmessagebox.h>
#include <QStack>

namespace KMobileTools {

ErrorHandler* ErrorHandler::m_uniqueInstance = 0;
QMutex ErrorHandler::m_mutex;
QStack<BaseError*> ErrorHandler::m_errorStack;

ErrorHandler::ErrorHandler()
{
}


ErrorHandler::~ErrorHandler()
{
    qDeleteAll( m_errorStack.begin(), m_errorStack.end() );
}

ErrorHandler* ErrorHandler::instance() {
    /// @TODO locking can be optimized here
    m_mutex.lock();
    if( ErrorHandler::m_uniqueInstance == 0 )
        ErrorHandler::m_uniqueInstance = new ErrorHandler();
    m_mutex.unlock();

    return ErrorHandler::m_uniqueInstance;
}

void ErrorHandler::addError( BaseError* error ) {
    /// @TODO implement me
    m_mutex.lock();

    // don't add the same error twice in a row
    if( !m_errorStack.isEmpty() ) {
        if( *(m_errorStack.top()) != *error )
            m_errorStack.push( error );
    } else
        m_errorStack.push( error );

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

    ErrorLog::instance()->write( QString( "===============================" ) );
    ErrorLog::instance()->write( QString( "Occurred on %1")
                                 .arg( error->dateTime().toString( Qt::ISODate ) ) );
    ErrorLog::instance()->write( QString( "Location:    %1:%2" ).arg( error->fileName() )
                                 .arg( QString::number( error->lineNumber() ) ) );
    ErrorLog::instance()->write( QString( "Method:      %1()" ).arg( error->methodName() ) );
    ErrorLog::instance()->write( QString( "Priority:    %1" ).arg( error->priority() ) );
    ErrorLog::instance()->write( QString( "Description: %1" ).arg( error->description() ) );

    KMessageBox::error( 0, errorMessage + priority );

    m_mutex.unlock();
}

int ErrorHandler::errorCount() const {
    return m_errorStack.count();
}

}
