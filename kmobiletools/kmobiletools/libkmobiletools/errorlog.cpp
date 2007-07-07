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

#include "errorlog.h"
#include <QTextStream>

namespace KMobileTools {

ErrorLog* ErrorLog::m_uniqueInstance = 0;
QMutex ErrorLog::m_mutex;
KTemporaryFile* ErrorLog::m_logFile;

ErrorLog::ErrorLog()
{
    m_logFile = new KTemporaryFile();
}


ErrorLog::~ErrorLog()
{
    delete m_logFile;
}

ErrorLog* ErrorLog::instance() {
    /// @TODO locking can be optimized here
    m_mutex.lock();
    if( ErrorLog::m_uniqueInstance != 0 )
        ErrorLog::m_uniqueInstance = new ErrorLog();
    m_mutex.unlock();

    return ErrorLog::m_uniqueInstance;
}

void ErrorLog::write( const QString& text ) {
    m_mutex.lock();

    QTextStream log( m_logFile );
    log << text << "\n";

    m_mutex.unlock();
}

}
