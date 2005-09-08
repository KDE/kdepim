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

// Qt includes
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes
#include <kconfig.h>
#include <kstandarddirs.h>

// local includes
#include "csvtemplate.h"
#include "csvtemplatefactory.h"

///////////////////////////////////////////////////////////////////////////////

CSVTemplateFactory::CSVTemplateFactory()
{
}

///////////////////////////////////////////////////////////////////////////////

CSVTemplateFactory::~CSVTemplateFactory()
{
    QMap<QString, CSVTemplate*>::iterator it    = m_templates.begin();
    QMap<QString, CSVTemplate*>::iterator endIt = m_templates.end();
    for (; it != endIt; ++it)
    {
        CSVTemplate* temp = it.data();
        delete temp;
    }
}

///////////////////////////////////////////////////////////////////////////////

CSVTemplate* CSVTemplateFactory::createTemplate(const QString& name)
{
    if (name.isEmpty()) return 0;

    QString filename = m_templateFiles[name];
    if (filename.isEmpty())
    {
        filename = findTemplateFile(name);
        m_templateFiles[name] = filename;
    }

    KConfigBase* templateConfig = loadTemplateConfig(filename);
    if (templateConfig == 0) return 0;

    return new CSVTemplate(templateConfig);
}

///////////////////////////////////////////////////////////////////////////////

CSVTemplate* CSVTemplateFactory::createCachedTemplate(const QString& name)
{
    if (name.isEmpty()) return 0;
    
    QMap<QString, CSVTemplate*>::iterator it = m_templates.find(name);
    CSVTemplate* temp = 0;
    if (it != m_templates.end()) temp = it.data();
    
    if (temp == 0)
    {
        temp = createTemplate(name);
        if (temp != 0)
        {
            m_templates[name] = temp;
        }
    }

    return temp;
}

///////////////////////////////////////////////////////////////////////////////////

QMap<QString, QString> CSVTemplateFactory::templateNames()
{
    if (m_templateNames.isEmpty())
    {
        addTemplateNames(QDir::currentDirPath());

        QStringList templateDirs =
            KGlobal::instance()->dirs()->findDirs("data", "kaddressbook/csv-templates");

        QStringList::const_iterator it    = templateDirs.begin();
        QStringList::const_iterator endIt = templateDirs.end();
        for (; it != endIt; ++it)
        {
            addTemplateNames(*it);
        }
    }

    return m_templateNames;
}

///////////////////////////////////////////////////////////////////////////////////

QString CSVTemplateFactory::findTemplateFile(const QString& name) const
{
    if (name.isEmpty()) return QString::null;

    QString filename = name + ".desktop";

    // check current working directory first
    QFileInfo fileInfo(filename);
    if (fileInfo.exists() && fileInfo.isReadable()) return fileInfo.absFilePath();
        
    return locate("data", "kaddressbook/csv-templates/" + filename);
}

///////////////////////////////////////////////////////////////////////////////

KConfigBase* CSVTemplateFactory::loadTemplateConfig(const QString& filename) const
{
    if (filename.isEmpty()) return 0;

    KConfig* config = new KConfig(filename);

    bool isTemplate = config->hasGroup("csv column map") &&
                      config->hasGroup("General") &&
                      config->hasGroup("Misc");
    if (!isTemplate)
    {
        delete config;
        config = 0;
    }
    
    return config;
}

///////////////////////////////////////////////////////////////////////////////

void CSVTemplateFactory::addTemplateNames(const QString& directory)
{
    if (directory.isEmpty()) return;

    QFileInfo dirInfo(directory);
    if (!dirInfo.isDir()) return;
    
    const QString extension = ".desktop";
    QDir dir(dirInfo.absFilePath());
    const QFileInfoList* fileInfos = dir.entryInfoList("*" + extension);
    QFileInfoListIterator it(*fileInfos);
    
    for (; it.current() != 0; ++it)
    {
        QFileInfo fileInfo = *(it.current());

        if (!fileInfo.isFile()) continue;
        if (!fileInfo.isReadable()) continue;
        
        QString name = fileInfo.fileName();
        name = name.left(name.length() - extension.length());

        if (name.isEmpty()) continue;
        if (!m_templateNames[name].isEmpty()) continue;
                    
        KConfigBase* templateConfig = loadTemplateConfig(fileInfo.absFilePath());
        if (templateConfig == 0) continue;

        templateConfig->setGroup("Misc");
        QString templateName = templateConfig->readEntry("Name");
        if (!templateName.isEmpty()) m_templateNames[name] = templateName;

        delete templateConfig;
    }
}

// End of file
