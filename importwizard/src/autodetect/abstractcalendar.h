/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef ABSTRACTCALENDAR_H
#define ABSTRACTCALENDAR_H

class ImportWizard;
#include "abstractbase.h"

#include <QString>

class AbstractCalendar : public AbstractBase
{
public:
    explicit AbstractCalendar(ImportWizard *parent);
    virtual ~AbstractCalendar();

protected:
    void addEvenViewConfig(const QString &groupName, const QString &key, const QString &value);
    void addImportInfo(const QString &log) Q_DECL_OVERRIDE;
    void addImportError(const QString &log) Q_DECL_OVERRIDE;

private:
    ImportWizard *mImportWizard;
};

#endif // ABSTRACTCALENDAR_H
