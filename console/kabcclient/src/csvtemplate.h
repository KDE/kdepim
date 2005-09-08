//
//  Copyright (C) 2005 Kevin Krammer <kevin.krammer@gmx.at>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef CSVTEMPLATE_H
#define CSVTEMPLATE_H

// Qt includes
#include <qmap.h>

// forward declarations
class KConfigBase;
class QDateTime;

namespace KABC
{
    class Addressee;
}

class CSVTemplate
{
public:
    CSVTemplate(KConfigBase* config);

    inline int columns() const { return m_columns; }

    inline const QString& delimiter() const { return m_delimiter; }

    inline const QString& quote() const { return m_quote; }
        
    QString fieldText(int column, const KABC::Addressee& addressee) const;

    void setFieldText(int column, KABC::Addressee& addressee, const QString& text) const;

    static CSVTemplate* defaultTemplate();
    
private:
    int m_columns;
    QString m_delimiter;
    QString m_quote;
    QMap<int, int> m_columnToField;
    QString m_datePattern;
    QString m_dateFormat;

    static CSVTemplate* m_defaultTemplate;
    
private:
    CSVTemplate(const QString& datePattern);

    QString formatDate(const QDateTime& date) const;
    QDateTime parseDate(const QString& text) const;

    void createDateFormat();
};

#endif

// End of file
