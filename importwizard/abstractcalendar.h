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
    void addEvenViewConfig( const QString& groupName, const QString& key, const QString& value);
    void addImportInfo( const QString& log );
    void addImportError( const QString& log );

private:
    ImportWizard *mImportWizard;
};

#endif // ABSTRACTCALENDAR_H
