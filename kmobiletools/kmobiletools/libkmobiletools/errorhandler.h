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

#ifndef KMOBILETOOLSERRORHANDLER_H
#define KMOBILETOOLSERRORHANDLER_H

#include <QMutex>
#include <QStack>

#include "error.h"
#include "kmobiletools_export.h"

namespace KMobileTools {

/**
    @author Matthias Lechner <matthias@lmme.de>

    This an KMobileTools' error handler.

    In a typical use-case you would retrieve an instance and add
    an error via addError().

    ErrorHandler::instance()->addError( Error( ... ) );
*/
class KMOBILETOOLS_EXPORT ErrorHandler {
public:
    /**
     * Returns an ErrorHandler instance
     *
     * @return an error handler instance
     */
    static ErrorHandler* instance();

    /**
     * Adds an error to the error handler
     *
     * @param error the error which should be handled
     */
    void addError( Error* error );

    /**
     * Returns how many errors have been occured within the current session
     * 
     * @return the error count
     */
    int errorCount() const;

    ~ErrorHandler();

private:
    ErrorHandler();
    // making copy constructor private since this is a singleton
    ErrorHandler( const ErrorHandler& );

    static ErrorHandler* m_uniqueInstance;
    static QMutex m_mutex;
    static QStack<Error*> m_errorStack;

    /**
     * SafeGuard is used to delete the ErrorHandler instance
     * on destruction.
     */
    class SafeGuard {
         public: ~SafeGuard() {
           if( ErrorHandler::m_uniqueInstance )
             delete ErrorHandler::m_uniqueInstance;
         }
     };
     friend class SafeGuard;

};

ErrorHandler* ErrorHandler::m_uniqueInstance = 0;
QMutex ErrorHandler::m_mutex;
QStack<Error*> ErrorHandler::m_errorStack;

}

#endif
