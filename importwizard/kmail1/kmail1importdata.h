/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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
#ifndef KMail1ImportData_H
#define KMail1ImportData_H

#include "abstractimporter.h"
class ImportWizard;

class KMail1ImportData : public AbstractImporter
{
public:
    explicit KMail1ImportData(ImportWizard *parent);
    ~KMail1ImportData();
    
    TypeSupportedOptions supportedOption();
    bool foundMailer() const;

    bool importMails();
    bool importSettings();
    bool importAddressBook();

    QString name() const;

};

#endif /* KMail1ImportData_H */

