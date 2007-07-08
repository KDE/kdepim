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
#include <KLocale>


namespace KMobileTools {

class BaseErrorPrivate {
public:
    QString m_fileName;
    int m_lineNumber;
    QDateTime m_dateTime;
    QString m_methodName;
    DebugHash m_customDebugInformation;

    BaseError::Priority m_priority;
    QString m_description;
};

BaseError::BaseError( const QString& fileName, int lineNumber, const QDateTime& dateTime,
                      const QString& methodName, const DebugHash& customDebugInformation ) {
    d = new BaseErrorPrivate;

    d->m_fileName = fileName;
    d->m_lineNumber = lineNumber;
    d->m_dateTime = dateTime;
    d->m_methodName = methodName;
    d->m_customDebugInformation = customDebugInformation;

    d->m_priority = High;
    d->m_description = i18n("No error description available.");
}


BaseError::~BaseError() {
    delete d;
}

bool BaseError::operator==( const BaseError& error ) const {
    if( error.fileName() == d->m_fileName &&
        error.lineNumber() == d->m_lineNumber &&
        error.dateTime() == d->m_dateTime )
        return true;

    return false;
}

bool BaseError::operator!=( const BaseError& error ) const {
    if( error.fileName() == d->m_fileName &&
        error.lineNumber() == d->m_lineNumber &&
        error.dateTime() == d->m_dateTime )
        return false;

    return true;
}

BaseError::Priority BaseError::priority() const {
    return d->m_priority;
}

QString BaseError::fileName() const {
    return d->m_fileName;
}

int BaseError::lineNumber() const {
    return d->m_lineNumber;
}

QDateTime BaseError::dateTime() const {
    return d->m_dateTime;
}

QString BaseError::methodName() const {
    return d->m_methodName;
}

QString BaseError::description() const {
    return d->m_description;
}

DebugHash BaseError::customDebugInformation() const {
    return d->m_customDebugInformation;
}

void BaseError::setPriority( Priority priority ) {
    d->m_priority = priority;
}

void BaseError::setDescription( const QString& errorDescription ) {
    d->m_description = errorDescription;
}

}
