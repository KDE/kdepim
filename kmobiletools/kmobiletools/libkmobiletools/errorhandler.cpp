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

#include <kmessagebox.h>

namespace KMobileTools {

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
    if( m_uniqueInstance != 0 )
        m_uniqueInstance = new ErrorHandler();
    m_mutex.unlock();

    return m_uniqueInstance;
}

void ErrorHandler::addError( Error* error ) {
    /// @TODO implement me
    m_mutex.lock();

    // don't add the same error twice in a row
    if( !m_errorStack.isEmpty() ) {
        if( *(m_errorStack.top()) != *error )
            m_errorStack.push( error );
    } else
        m_errorStack.push( error );

    QString priority;
    switch( error->priority() ) {
        case Error::Low:
            priority = "Low";
            break;

        case Error::Medium:
            priority = "Medium";
            break;

        case Error::High:
            priority = "High";
            break;
    }

    QString errorMessage = QString( "%1 priority error occured.\nFile: %2\nLine: %3" ).arg( priority )
                        .arg( error->fileName() ).arg( error->lineNumber() );
    KMessageBox::error( 0, errorMessage );

    m_mutex.unlock();
}

int ErrorHandler::errorCount() const {
    return m_errorStack.count();
}

}
