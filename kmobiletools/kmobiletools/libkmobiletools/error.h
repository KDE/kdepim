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

#ifndef KMOBILETOOLSERROR_H
#define KMOBILETOOLSERROR_H

#include "kmobiletools_export.h"

#include <QString>

namespace KMobileTools {

/**
    @author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT Error {
public:
    enum Priority {
        Low = 0,
        Medium = 5,
        High = 10
    };

    Error( const QString& fileName, int lineNumber );
    ~Error();

    bool operator==( Error& error ) const;
    bool operator!=( Error& error ) const;

    /**
     * Returns the file name where the error has occurred
     * 
     * @return the file name where the error has occurred
     */
    QString fileName() const;

    /**
     * Returns the line where the error has occurred
     *
     * @return the line where the error has occurred
     */
    int lineNumber() const;

    /**
     * Returns the error's priority
     *
     * @return the priority
     */
    Priority priority() const;

    /**
     * Returns a human-readable description of the error
     *
     * @return an error description
     */
    QString description() const;

protected:
    /**
     * Sets the error's priority
     *
     * Default: High
     *
     * @param priority the error's priority
     */
    void setPriority( Priority priority );

    /**
     * Sets a sensible description for the error
     *
     * @param errorDescription an error description
     */
    void setDescription( const QString& errorDescription );

private:
    QString m_fileName;
    int m_lineNumber;

    Priority m_priority;
    QString m_description;

};

}

#endif
