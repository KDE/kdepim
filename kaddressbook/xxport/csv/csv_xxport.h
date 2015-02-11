/*
  This file is part of KAddressBook.
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef CSV_XXPORT_H
#define CSV_XXPORT_H

#include "xxport.h"

class QFile;

class CsvXXPort : public XXPort
{
public:
    explicit CsvXXPort(QWidget *parent = Q_NULLPTR);

    KContacts::Addressee::List importContacts() const Q_DECL_OVERRIDE;
    bool exportContacts(const KContacts::Addressee::List &contacts, VCardExportSelectionWidget::ExportFields) const Q_DECL_OVERRIDE;

private:
    void exportToFile(QFile *, const KContacts::Addressee::List &) const;
};

#endif
