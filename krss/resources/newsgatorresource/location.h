/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KRSS_NEWSGATORRESOURCE_LOCATION_H
#define KRSS_NEWSGATORRESOURCE_LOCATION_H

#include <QtCore/QString>

struct Location
{
    Location() : id( -1 ) {}
    int id;
    QString name;
    bool isPublic;
    bool isApmlPublic;
    bool contentOnline;
    bool autoAddSubs;
};

#endif // KRSS_NEWSGATORRESOURCE_LOCATION_H
