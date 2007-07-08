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

#include "baseerror.h"
#include <klocale.h>


namespace KMobileTools {

BaseError::BaseError( const QString& fileName, int lineNumber, const QDateTime& dateTime,
                      const QString& methodName, const DebugHash& customDebugInformation ) {
    m_fileName = fileName;
    m_lineNumber = lineNumber;
    m_dateTime = dateTime;
    m_methodName = methodName;
    m_customDebugInformation = customDebugInformation;

    m_priority = High;
    m_description = i18n("No error description available.");
}


BaseError::~BaseError() {
}

bool BaseError::operator==( BaseError& error ) const {
    if( error.fileName() == m_fileName &&
        error.lineNumber() == m_lineNumber &&
        error.dateTime() == m_dateTime )
        return true;

    return false;
}

bool BaseError::operator!=( BaseError& error ) const {
    if( error.fileName() == m_fileName &&
        error.lineNumber() == m_lineNumber &&
        error.dateTime() == m_dateTime )
        return false;

    return true;
}

BaseError::Priority BaseError::priority() const {
    return m_priority;
}

QString BaseError::fileName() const {
    return m_fileName;
}

int BaseError::lineNumber() const {
    return m_lineNumber;
}

QDateTime BaseError::dateTime() const {
    return m_dateTime;
}

QString BaseError::methodName() const {
    return m_methodName;
}

QString BaseError::description() const {
    return m_description;
}

void BaseError::setPriority( Priority priority ) {
    m_priority = priority;
}

void BaseError::setDescription( const QString& errorDescription ) {
    m_description = errorDescription;
}

}
