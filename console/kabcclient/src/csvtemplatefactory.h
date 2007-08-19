//
//  Copyright (C) 2005 - 2006 Kevin Krammer <kevin.krammer@gmx.at>
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef CSVTEMPLATEFACTORY_H
#define CSVTEMPLATEFACTORY_H

// Qt includes
#include <QMap>
#include <QStringList>

// forward declarations
class CSVTemplate;
class KConfigBase;

/**
* @brief Factory for creation CSV template handlers
*
* KAddressBook supports configurable CSV (comma separated values) input/output
* through so-called "templates", i.e. KDE style (INI style) config files, which
* describe how columns of the CSV input are to be mapped from/to contact fields.
* See @ref csvhandling for details.
*
* This class is used by the CSVInput parser and CSVOutput formatter to create and
* configure a template handler based on the name specified by the user.
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
* @see CSVTemplate
*/
class CSVTemplateFactory
{
public:
    /**
    * @brief Creates the factory instance
    */
    CSVTemplateFactory();

    /**
    * @brief Destroys the instance
    *
    * Autodeletes the cached template instances created by createCachedTemplate()
    */
    ~CSVTemplateFactory();

    /**
    * @brief Creates a template handler for a given name
    *
    * The template's name is the basic file name (without the .desktop extension)
    * of the template configuration file.
    * It is the key of the respective entry in the map returned by templateNames()
    *
    * @note The caller becomes the owner of the returned instance
    *
    * @param name the name of the CSV template to create a handler for
    *
    * @return a CSV handler instance or @c 0 if the @p name was empty or there
    *         is no template with such a name
    *
    * @see createCachedTemplate()
    * @see findTemplateFile()
    */
    CSVTemplate* createTemplate(const QString& name);

    /**
    * @brief Creates a template handler for a given name and caches the instance
    *
    * Basically equal to createTemplate(), but only creates an instance on the
    * first call for each name, i.e. keeps one instance per name.
    *
    * @warning Cached instances are owned by the factory and will be deleted on its
    *          destruction
    *
    * @param name the name of the CSV template to create a handler for
    *
    * @return a CSV handler instance or @c 0 if the @p name was empty or there
    *         is no template with such a name
    *
    * @see templateNames()
    * @see findTemplateFile()
    */
    CSVTemplate* createCachedTemplate(const QString& name);

    /**
    * @brief Returns a set of available templates
    *
    * On the first call this method will check the current working directory
    * and all of KAddressBook's data directories, as returned by
    * KStandardDirs::findDirs(), for template configuration files.
    *
    * The resulting map's keys are the identifiers used to create template
    * handlers through createTemplate() or createCachedTemplate().
    *
    * The map's values are the potentially localized name specified in the
    * template configuration file itself.
    *
    * @note Only queries the filesystem on the first call and later on just
    * returns the cached information of this first call
    *
    * @return map of template identifiers to templates' UI names
    */
    QMap<QString, QString> templateNames();

private:
    /**
    * Cached result of the first call to templateNames().
    * Mapping from "name" (identifier) to a localized name/short description
    * as specified in the template's file. See @ref csv-handling
    */
    QMap<QString, QString> m_templateNames;

    /**
    * Cached mapping of template identifier to the configuration file's path.
    * Value is cached on first call to either of the two creation methods.
    */
    QMap<QString, QString> m_templateFiles;

    /**
    * Cached handler instances created by createCachedTemplate()
    * Mapping from template identifier to instance pointer.
    * Each instance will be deleted at the factory's destruction.
    */
    QMap<QString, CSVTemplate*> m_templates;

private:
    /**
    * @brief Helper method for finding the file of a given template identifier
    *
    * Checks first for the respectively named file (name + ".desktop") in the
    * program's working directory. If there isn't such a file it uses
    * KStandardDirs::locate() to find the respectively named .desktop file
    * in KAddressBook's data directories, sub directory "csv-templates", e.g.
    * KDEPREFIX/share/apps/kaddressbook/csv-templates/
    *
    * @param name the file name / identifier of a CSV template configuration
    *
    * @return the path of the template configuration file or @c QString()
    *         if name is empty or no such file could found
    */
    QString findTemplateFile(const QString& name) const;

    /**
    * @brief Helper method for loading a template configuration file
    *
    * Plausibility check by asserting that the file has the three groups
    * the CVSTemplate is expecting.
    *
    * @param filename the file path of the configuration file to load
    *
    * @return a config instance for accessing the file's data
    */
    KConfigBase* loadTemplateConfig(const QString& filename) const;

    /**
    * @brief Helper method for processing a directory for template files
    *
    * Lists all ".desktop" files in the given directory and extracts the
    * localized name / short description from each found template configuration
    * file.
    *
    * @param directory path of the directory to check for templates
    *
    * @see loadTemplateConfig()
    */
    void addTemplateNames(const QString& directory);

private:
    /**
    * @brief Hidden copy constructor
    */
    CSVTemplateFactory(const CSVTemplateFactory&);

    /**
    * @brief Hidden assignment operator
    */
    CSVTemplateFactory& operator=(const CSVTemplateFactory&);
};

#endif

// End of file
