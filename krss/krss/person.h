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

#ifndef KRSS_PERSON_H
#define KRSS_PERSON_H

#include "krss_export.h"

#include <QtCore/QSharedDataPointer>

class QString;

namespace KRss {

class KRSS_EXPORT Person
{
public:
    Person();
    Person( const Person& other );
    ~Person();

    QString name() const;
    void setName( const QString& name );

    QString uri() const;
    void setUri( const QString& uri );

    QString email() const;
    void setEmail( const QString& email );

    void swap( Person& other );
    Person& operator=( const Person& other );
    bool operator==( const Person& other ) const;
    bool operator!=( const Person& other ) const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

} // namespace KRss

#endif // KRSS_PERSON_H
