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

#ifndef CSVTEMPLATEFACTORY_H
#define CSVTEMPLATEFACTORY_H

// Qt includes
#include <qmap.h>
#include <qstringlist.h>

// forward declarations
class CSVTemplate;
class KConfigBase;

class CSVTemplateFactory
{
public:
    CSVTemplateFactory();
    ~CSVTemplateFactory();

    CSVTemplate* createTemplate(const QString& name);
    CSVTemplate* createCachedTemplate(const QString& name);

    QMap<QString, QString> templateNames();
    
private:
    QMap<QString, QString> m_templateNames;
    QMap<QString, QString> m_templateFiles;
    QMap<QString, CSVTemplate*> m_templates;

private:
    QString findTemplateFile(const QString& name) const;
    KConfigBase* loadTemplateConfig(const QString& filename) const;

    void addTemplateNames(const QString& directory);
        
private:
    CSVTemplateFactory(const CSVTemplateFactory&);
    CSVTemplateFactory& operator=(const CSVTemplateFactory&);
};

#endif

// End of file
