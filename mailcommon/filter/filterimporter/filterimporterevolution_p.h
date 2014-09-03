/*
  Copyright (c) 2012, 2013 Montel Laurent <montel@kde.org>

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

#ifndef MAILCOMMON_FILTERIMPORTER_FILTERIMPORTEREVOLUTION_P_H
#define MAILCOMMON_FILTERIMPORTER_FILTERIMPORTEREVOLUTION_P_H

#include "filterimporter/filterimporterabstract_p.h"

#include <QDomElement>

class QFile;

namespace MailCommon
{

class MailFilter;

class FilterImporterEvolution : public FilterImporterAbstract
{
public:
    explicit FilterImporterEvolution(QFile *file);
    ~FilterImporterEvolution();
    static QString defaultFiltersSettingsPath();
private:
    enum parseType {
        PartType = 0,
        ActionType = 1
    };
    void parseFilters(const QDomElement &e);
    void parsePartAction(const QDomElement &ruleFilter,
                         MailCommon::MailFilter *filter,
                         MailCommon::FilterImporterEvolution::parseType type);
};

}

#endif // FILTERIMPORTEREVOLUTION_H
