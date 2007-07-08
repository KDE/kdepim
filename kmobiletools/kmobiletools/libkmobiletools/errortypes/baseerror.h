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
#include <QDateTime>
#include <QHash>

#define ERROR_META_INFO __FILE__, __LINE__, QDateTime::currentDateTime(), __FUNCTION__

namespace KMobileTools {

// gcc 4.1.3 doesn't compile without using this typedef
typedef QHash<QString,QString> DebugHash;

class BaseErrorPrivate;

/**
    @author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT BaseError {
public:
    enum Priority {
        Low = 0,
        Medium = 5,
        High = 10
    };

    BaseError( const QString& fileName,
               int lineNumber,
               const QDateTime& dateTime,
               const QString& methodName,
               const DebugHash& customDebugInformation = DebugHash() );
    ~BaseError();

    bool operator==( const BaseError& error ) const;
    bool operator!=( const BaseError& error ) const;

    /**
     * Returns the file name where the error occurred
     *
     * @return the file name where the error occurred
     */
    QString fileName() const;

    /**
     * Returns the line where the error occurred
     *
     * @return the line where the error occurred
     */
    int lineNumber() const;

    /**
     * Returns the date and time when the error occurred
     *
     * @return the date and time
     */
    QDateTime dateTime() const;

    /**
     * Returns the name of the method in which the error occurred
     *
     * @return the method name
     */
    QString methodName() const;

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

    /**
     * Returns custom debug information as QString-QString hash.
     * This can be used by derived classes to retrieve context related information
     * which was before added in the constructor.
     *
     * @return custom debug information
     */
    DebugHash customDebugInformation() const;

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
    BaseErrorPrivate* d;
};

}

#endif
