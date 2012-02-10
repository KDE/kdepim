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

#ifndef MAILCOMMON_FILTERIMPORTER_FILTERIMPORTERABSTRACT_P_H
#define MAILCOMMON_FILTERIMPORTER_FILTERIMPORTERABSTRACT_P_H

#include <QDomDocument>
#include <QList>
#include <QStringList>

class QFile;

namespace MailCommon {

class MailFilter;

class FilterImporterAbstract
{
  public:
    explicit FilterImporterAbstract();
    ~FilterImporterAbstract();
    QList<MailFilter*> importFilter() const;
    QStringList emptyFilter() const;

  protected:
    void appendFilter( MailCommon::MailFilter *filter );
    void createFilterAction( MailCommon::MailFilter *filter,
                             const QString &actionName, const QString &value );
    bool loadDomElement( QDomDocument &doc, QFile *file );

    QList<MailFilter*> mListMailFilter;
    QStringList mEmptyFilter;
};

}

#endif // FILTERIMPORTERABSTRACT_H
