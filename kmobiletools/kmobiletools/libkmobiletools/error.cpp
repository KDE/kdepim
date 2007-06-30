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

#include "error.h"
#include <klocale.h>


namespace KMobileTools {

Error::Error( const QString& fileName, int lineNumber ) {
    m_fileName = fileName;
    m_lineNumber = lineNumber;

    m_priority = High;
    m_description = i18n("No error description available.");
}


Error::~Error() {
}

bool Error::operator==( Error& error ) const {
    /// @TODO implement me
    return false;
}

bool Error::operator!=( Error& error ) const {
    /// @TODO implement me
    return true;
}

Error::Priority Error::priority() const {
    return m_priority;
}

QString Error::fileName() const {
    return m_fileName;
}

int Error::lineNumber() const {
    return m_lineNumber;
}

QString Error::description() const {
    return m_description;
}

void Error::setPriority( Priority priority ) {
    m_priority = priority;
}

void Error::setDescription( const QString& errorDescription ) {
    m_description = errorDescription;
}

}
