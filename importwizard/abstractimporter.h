/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#ifndef AbstractImporter_H
#define AbstractImporter_H
#include "mailcommon/filter/filterimporterexporter.h"

class ImportWizard;

namespace MailImporter
{
class FilterInfo;
}

namespace MailCommon
{
class MailFilter;
class FilterImporterExporter;
}

class AbstractImporter
{
public:
    enum TypeSupportedOption {
        None = 0,
        Mails = 1,
        Settings = 2,
        Filters = 4,
        AddressBooks = 8,
        Calendars = 16
    };

    Q_DECLARE_FLAGS(TypeSupportedOptions, TypeSupportedOption)

    explicit AbstractImporter(ImportWizard *parent);
    virtual ~AbstractImporter();

    /**
    * Return true if mail found on system
    */
    virtual bool foundMailer() const = 0;

    /**
    * Return type of data that we can import
    */
    virtual TypeSupportedOptions supportedOption() = 0;
    /**
    * Return name for plugins
    */
    virtual QString name() const = 0;

    virtual bool importSettings();
    virtual bool importMails();
    virtual bool importFilters();
    virtual bool importAddressBook();
    virtual bool importCalendar();

protected:
    void appendFilters(const QList<MailCommon::MailFilter *> &filters);
    MailImporter::FilterInfo *initializeInfo();
    void addImportFilterInfo(const QString &log);
    void addImportFilterError(const QString &log);
    bool addFilters(const QString &filterPath, MailCommon::FilterImporterExporter::FilterType type);
    void addImportSettingsInfo(const QString &log);
    void addImportCalendarInfo(const QString &log);

    QString mPath;
    ImportWizard *mImportWizard;
};

#endif /* AbstractImporter_H */

