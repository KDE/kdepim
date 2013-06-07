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
#ifndef BalsaImportData_H
#define BalsaImportData_H

#include "abstractimporter.h"
class ImportWizard;

class BalsaImportData : public AbstractImporter
{
public:
    explicit BalsaImportData(ImportWizard *parent);
    ~BalsaImportData();
    
    TypeSupportedOptions supportedOption();
    bool foundMailer() const;

    bool importMails();
    bool importAddressBook();
    bool importSettings();
    bool importFilters();
    QString name() const;

};

#endif /* BalsaImportData_H */

