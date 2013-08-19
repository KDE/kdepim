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
#ifndef Evolutionv3ImportData_H
#define Evolutionv3ImportData_H

#include "abstractimporter.h"
class ImportWizard;

class Evolutionv3ImportData : public AbstractImporter
{
public:
    explicit Evolutionv3ImportData(ImportWizard *parent);
    ~Evolutionv3ImportData();
    
    TypeSupportedOptions supportedOption();
    bool foundMailer() const;

    bool importSettings();
    bool importMails();
    bool importFilters();
    bool importAddressBook();
    bool importCalendar();

    QString name() const;
};

#endif /* Evolutionv3ImportData_H */

