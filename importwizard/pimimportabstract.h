/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#ifndef PIMIMPORTABSTRACT_H
#define PIMIMPORTABSTRACT_H
#include <QObject>
#include "mailcommon/filter/filterimporterexporter.h"

class ImportWizard;

namespace MailImporter {
    class FilterInfo;
}

namespace MailCommon {
class MailFilter;
class FilterImporterExporter;
}

class PimImportAbstract
{
public:
  enum TypeSupportedOption
  {
    None = 0, 
    Mails = 1,
    Settings = 2,
    Filters = 4,
    AddressBook = 8
  };

  Q_DECLARE_FLAGS(TypeSupportedOptions, TypeSupportedOption )

  explicit PimImportAbstract(ImportWizard *parent);
  virtual ~PimImportAbstract();

  virtual bool foundMailer() const= 0;
  
  virtual TypeSupportedOptions supportedOption() = 0;
  virtual QString name() const = 0;
  virtual bool importSettings();
  virtual bool importMails();
  virtual bool importFilters();
  virtual bool importAddressBook();

protected:
  void appendFilters( const QList<MailCommon::MailFilter*>& filters );
  MailImporter::FilterInfo* initializeInfo();
  void addFilterImportInfo( const QString& log );
  bool addFilters( const QString& filterPath, MailCommon::FilterImporterExporter::FilterType type );


  QString mPath;
  ImportWizard *mImportWizard;
};


#endif /* PIMIMPORTABSTRACT_H */

