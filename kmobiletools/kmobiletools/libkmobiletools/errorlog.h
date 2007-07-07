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

#ifndef KMOBILETOOLSERRORLOG_H
#define KMOBILETOOLSERRORLOG_H

#include <KTemporaryFile>
#include <QMutex>

#include "kmobiletools_export.h"

namespace KMobileTools {

/**
    @author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT ErrorLog {
public:
    /**
     * Returns an ErrorLog instance
     *
     * @return a ErrorLog instance
     */
    static ErrorLog* instance();
    static void write( const QString& text );
    ~ErrorLog();

private:
    ErrorLog();
    // making copy constructor private since this is a singleton
    ErrorLog( const ErrorLog& );

    static ErrorLog* m_uniqueInstance;
    static QMutex m_mutex;
    static KTemporaryFile* m_logFile;

    /**
     * SafeGuard is used to delete the ErrorLog instance
     * on destruction.
     */
    class SafeGuard {
         public: ~SafeGuard() {
           if( ErrorLog::m_uniqueInstance )
             delete ErrorLog::m_uniqueInstance;
         }
     };
     friend class SafeGuard;
};

}

#endif
