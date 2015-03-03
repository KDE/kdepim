/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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


#include "filterimportergmail.h"
#include "mailcommon_debug.h"
#include <QFile>
#include <QDir>
#include "mailfilter.h"
using namespace MailCommon;

FilterImporterGmail::FilterImporterGmail( QFile *file )
    :FilterImporterAbstract()
{
    QDomDocument doc;
    if ( !loadDomElement( doc, file ) ) {
        return;
    }

    QDomElement filters = doc.documentElement();

    if ( filters.isNull() ) {
        qCDebug(MAILCOMMON_LOG) << "No filters defined";
        return;
    }
    for ( QDomElement e = filters.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
        const QString tag = e.tagName();
        if ( tag == QLatin1String( "entry" ) ) {
            qCDebug(MAILCOMMON_LOG) << " filter found !";
            parseFilters(e);
        }
    }
}

FilterImporterGmail::~FilterImporterGmail()
{

}

QString FilterImporterGmail::defaultFiltersSettingsPath()
{
    return QDir::homePath();
}

void FilterImporterGmail::parseFilters( const QDomElement &e )
{
    MailCommon::MailFilter *filter = new MailCommon::MailFilter();
    filter->setAutoNaming(true);
    filter->setEnabled( true );
    for ( QDomElement ruleFilter = e.firstChildElement();
          !ruleFilter.isNull();
          ruleFilter = ruleFilter.nextSiblingElement() )
    {
        const QString tagName = ruleFilter.tagName();
        if (tagName == QLatin1String("category")) {
            if (ruleFilter.hasAttribute(QLatin1String("term"))) {
                if (ruleFilter.attribute(QLatin1String("term")) != QLatin1String("filter")) {
                    continue;
                }
            }
        } else if (tagName == QLatin1String("apps:property")) {
            if (ruleFilter.hasAttribute(QLatin1String("name"))) {
                const QString criteriaProperty = ruleFilter.attribute(QLatin1String("name"));
                qDebug()<<" ruleFilter.attribute"<<criteriaProperty;
                // Criterial
                if (criteriaProperty == QLatin1String("from")) {

                } else if (criteriaProperty == QLatin1String("to")) {

                } else if (criteriaProperty == QLatin1String("subject")) {

                } else if (criteriaProperty == QLatin1String("hasTheWord")) {

                } else if (criteriaProperty == QLatin1String("doesNotHaveTheWord")) {

                } else if (criteriaProperty == QLatin1String("hasAttachment")) {

                }
                //Action
                else if (criteriaProperty == QLatin1String("shouldArchive")) {

                } else if (criteriaProperty == QLatin1String("shouldMarkAsRead")) {

                } else if (criteriaProperty == QLatin1String("shouldStar")) {

                } else if (criteriaProperty == QLatin1String("label")) {

                } else if (criteriaProperty == QLatin1String("forwardTo")) {

                } else if (criteriaProperty == QLatin1String("shouldTrash")) {

                } else if (criteriaProperty == QLatin1String("neverSpam")) {

                }
            }
        }
    }
    appendFilter(filter);
}
