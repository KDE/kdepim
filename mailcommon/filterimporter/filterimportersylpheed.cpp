/*
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filterimportersylpheed_p.h"
#include "mailfilter.h"
#include "filtermanager.h"

#include <kdebug.h>

#include <QDomDocument>
#include <QFile>

using namespace MailCommon;

FilterImporterSylpheed::FilterImporterSylpheed(QFile *file)
    :FilterImporterAbstract()
{
    QDomDocument doc;
    QString errorMsg;
    int errorRow;
    int errorCol;
    if ( !doc.setContent( file, &errorMsg, &errorRow, &errorCol ) ) {
        kDebug() << "Unable to load document.Parse error in line " << errorRow << ", col " << errorCol << ": " << errorMsg << endl;
        return;
    }
    QDomElement filters = doc.documentElement();

    if ( filters.isNull() ) {
        kDebug() << "No filters defined" << endl;
        return;
    }
    for ( QDomElement e = filters.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
        const QString tag = e.tagName();
        if ( tag == QLatin1String( "rule" ) ) {
            parseFilters(e);
        } else {
            qDebug()<<" unknown tag "<<tag;
        }
    }
}

FilterImporterSylpheed::~FilterImporterSylpheed()
{
}

void FilterImporterSylpheed::parseFilters(const QDomElement &e)
{
    MailCommon::MailFilter *filter = new MailCommon::MailFilter();
    if( e.hasAttribute("enabled"))
    {
        const QString attr = e.attribute("enabled");
        if( attr == QLatin1String("false"))
            filter->setEnabled(false);
    }
    if( e.hasAttribute("name"))
    {
        const QString attr = e.attribute("name");
        filter->pattern()->setName(attr);
        filter->setToolbarName(attr);
    }
    if( e.hasAttribute("timing"))
    {
        const QString attr = e.attribute("timing");
        if ( attr == QLatin1String( "all" ) ) {
          filter->pattern()->setOp(SearchPattern::OpAnd);
        } else if ( attr == QLatin1String( "any" ) ) {
          filter->pattern()->setOp(SearchPattern::OpOr);
        } else {
          qDebug()<<" timing not defined: "<< attr;
        }

    }
    for ( QDomElement ruleFilter = e.firstChildElement(); !ruleFilter.isNull(); ruleFilter = ruleFilter.nextSiblingElement() )
    {
        const QString nexttag = ruleFilter.tagName();
        qDebug()<<" nexttag "<<nexttag;
        if(nexttag == QLatin1String("condition-list")){
          //TODO
        } else if( nexttag == QLatin1String("action-list")) {
          //TODO
        }
    }

    appendFilter(filter);
}
