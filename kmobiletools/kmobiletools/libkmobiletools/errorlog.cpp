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
#include <QMutex>
#include <QTextStream>
#include <KStandardDirs>
#include <KGlobal>
#include <KTemporaryFile>

#include <KDebug>

namespace KMobileTools {

class ErrorLogInstance {
public:
    ErrorLog m_uniqueInstance;
};

class ErrorLogPrivate {
public:
    static QMutex m_mutex;
    KTemporaryFile m_logFile;
};

QMutex ErrorLogPrivate::m_mutex;

K_GLOBAL_STATIC(ErrorLogInstance, errorLogInstance)

ErrorLog::ErrorLog()
: d( new ErrorLogPrivate )
{
    d->m_logFile.open();
}


ErrorLog::~ErrorLog()
{
    d->m_logFile.close();
}

ErrorLog* ErrorLog::instance() {
    return &errorLogInstance->m_uniqueInstance;
}

void ErrorLog::write( const QString& text ) {
    d->m_mutex.lock();

    QTextStream log( &d->m_logFile );
    log << text << "\n";

    d->m_mutex.unlock();
}

QString ErrorLog::fileName() const {
    return d->m_logFile.fileName();
}

}
