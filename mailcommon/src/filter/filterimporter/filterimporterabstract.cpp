/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "mailcommon_debug.h"
#include "filterimporterabstract.h"
#include "filter/filteractions/filteractiondict.h"
#include "filter/filtermanager.h"
#include "filter/mailfilter.h"

#include <QFile>

using namespace MailCommon;

FilterImporterAbstract::FilterImporterAbstract(bool interactive)
    : mInteractive(interactive)
{
}

FilterImporterAbstract::~FilterImporterAbstract()
{
}

QList<MailFilter *> FilterImporterAbstract::importFilter() const
{
    return mListMailFilter;
}

QStringList FilterImporterAbstract::emptyFilter() const
{
    return mEmptyFilter;
}

void FilterImporterAbstract::appendFilter(MailCommon::MailFilter *filter)
{
    if (!filter) {
        return;
    }

    //TODO verify number of condition
    filter->purify();
    if (!filter->isEmpty()) {
        // the filter is valid:
        mListMailFilter << filter;
    } else {
        mEmptyFilter << filter->name();
        // the filter is invalid:
        qCDebug(MAILCOMMON_LOG) << " Empty filter";
        delete filter;
    }
}

void FilterImporterAbstract::createFilterAction(MailCommon::MailFilter *filter,
        const QString &actionName,
        const QString &value)
{
    if (!actionName.isEmpty()) {
        FilterActionDesc *desc = MailCommon::FilterManager::filterActionDict()->value(actionName);
        if (desc) {
            FilterAction *fa = desc->create();
            //...create an instance...
            if (mInteractive) {
                fa->argsFromStringInteractive(value, filter->name());
            } else {
                fa->argsFromString(value);
            }

            //...check if it's empty and...
            if (!fa->isEmpty()) {
                //...append it if it's not and...
                filter->actions()->append(fa);
            } else {
                //...delete is else.
                delete fa;
            }
        }
    }
}

bool FilterImporterAbstract::loadDomElement(QDomDocument &doc, QFile *file)
{
    QString errorMsg;
    int errorRow;
    int errorCol;
    if (!doc.setContent(file, &errorMsg, &errorRow, &errorCol)) {
        qCDebug(MAILCOMMON_LOG) << "Unable to load document.Parse error in line " << errorRow
                                << ", col " << errorCol << ": " << errorMsg;
        return false;
    }
    return true;
}
