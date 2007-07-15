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

#include "kmobiletools_export.h"

class QString;

namespace KMobileTools {

/**
    @author Matthias Lechner <matthias@lmme.de>
*/
class ErrorLogPrivate;
class KMOBILETOOLS_EXPORT ErrorLog {
    friend class ErrorLogInstance;
public:
    /**
     * Returns an ErrorLog instance
     *
     * @return a ErrorLog instance
     */
    static ErrorLog* instance();

    /**
     * Adds an entry to the error log
     *
     * @param text the text to add
     */
    void write( const QString& text );

    /**
     * Returns the file name of the error log
     *
     * @return the error log file name
     */
    QString fileName() const;

    ~ErrorLog();

private:
    ErrorLog();
    // making copy constructor private since this is a singleton
    ErrorLog( const ErrorLog& );

    ErrorLogPrivate* const d;
};

}

#endif
