/*
 * This file is part of the krss library
 *
 * Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef KRSS_ENCLOSURE_H
#define KRSS_ENCLOSURE_H

#include "krss_export.h"

#include <QtCore/QSharedDataPointer>

class QString;

typedef unsigned int uint;

namespace KRss {

class KRSS_EXPORT Enclosure
{
public:
    Enclosure();
    Enclosure( const Enclosure& other );
    ~Enclosure();

    bool isNull() const;

    QString url() const;
    void setUrl( const QString& url );

    QString title() const;
    void setTitle( const QString& title );

    QString type() const;
    void setType( const QString& type );

    uint length() const;
    void setLength( uint length );

    uint duration() const;
    void setDuration( uint duration );

    void swap( Enclosure& other );
    Enclosure& operator=( const Enclosure& other );
    bool operator==( const Enclosure& other ) const;
    bool operator!=( const Enclosure& other ) const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

} // namespace KRss

#endif // KRSS_ENCLOSURE_H
