/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "nepomukdebugutils.h"

QString PimCommon::NepomukDebugUtils::indentQuery(QString query)
{
    query = query.simplified();
    QString newQuery;
    int i = 0;
    int indent = 0;
    const int space = 4;

    while(i < query.size()) {
        newQuery.append(query[i]);
        if (query[i] != QLatin1Char('"') && query[i] != QLatin1Char('<') && query[i] != QLatin1Char('\'')) {
            if (query[i] == QLatin1Char('{')) {
                ++indent;
                newQuery.append(QLatin1Char('\n'));
                newQuery.append(QString().fill(QLatin1Char(' '), indent*space));
            } else if (query[i] == QLatin1Char('.')) {
                if(i+2<query.size()) {
                    if(query[i+1] == QLatin1Char('}')||query[i+2] == QLatin1Char('}')) {
                        newQuery.append(QLatin1Char('\n'));
                        newQuery.append(QString().fill(QLatin1Char(' '), (indent-1)*space));
                    } else {
                        newQuery.append(QLatin1Char('\n'));
                        newQuery.append(QString().fill(QLatin1Char(' '), indent*space));
                    }
                } else {
                    newQuery.append(QLatin1Char('\n'));
                    newQuery.append(QString().fill(QLatin1Char(' '), indent*space));
                }
            } else if (query[i] == QLatin1Char('}')) {
                indent--;
                if (i+2<query.size()) {
                    if (query[i+2] == QLatin1Char('.')||query[i+1] == QLatin1Char('.')) {
                        newQuery.append(QString().fill(QLatin1Char(' '), 1));
                    } else {
                        newQuery.append(QLatin1Char('\n'));
                        newQuery.append(QString().fill(QLatin1Char(' '), indent*space));
                    }
                } else {
                    newQuery.append(QLatin1Char('\n'));
                    newQuery.append(QString().fill(QLatin1Char(' '), indent*space));
                }
            }
        } else {
            ++i;
            while(i < query.size()) {
                if (query[i] == QLatin1Char('"') || query[i] == QLatin1Char('>') || query[i] == QLatin1Char('\'')) {
                    newQuery.append(query[i]);
                    break;
                }
                newQuery.append(query[i]);
                ++i;
            }
        }
        ++i;
    }
    return newQuery;
}
