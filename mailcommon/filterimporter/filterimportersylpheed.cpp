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

void FilterImporterSylpheed::parseConditions(const QDomElement &e, MailCommon::MailFilter *filter)
{
  if ( e.hasAttribute( "bool" ) ) {
    const QString attr = e.attribute("bool");
    if ( attr == QLatin1String( "and" ) ) {
      filter->pattern()->setOp(SearchPattern::OpAnd);
    } else if ( attr == QLatin1String( "or" ) ) {
      filter->pattern()->setOp(SearchPattern::OpOr);
    } else {
      qDebug()<<" bool not defined: "<< attr;
    }
  }
  for ( QDomElement ruleFilter = e.firstChildElement(); !ruleFilter.isNull(); ruleFilter = ruleFilter.nextSiblingElement() )
  {
      QString value;
      QString contents;
      SearchRule::Function functionName = SearchRule::FuncNone;

      const QString nexttag = ruleFilter.tagName();
      if(nexttag == QLatin1String("match-header")){
          //TODO
      } else if( nexttag == QLatin1String("match-any-header")) {
      } else if( nexttag == QLatin1String("match-to-or-cc")) {
      } else if( nexttag == QLatin1String("match-body-text")) {
      } else if( nexttag == QLatin1String("command-test")) {
      } else if( nexttag == QLatin1String("size")) {
      } else if( nexttag == QLatin1String("age")) {
      } else if( nexttag == QLatin1String("unread")) {
      } else if( nexttag == QLatin1String("mark")) {
      } else if( nexttag == QLatin1String("color-label")) {
      } else if( nexttag == QLatin1String("mime")) {
      } else if( nexttag == QLatin1String("account-id")) {
      } else if( nexttag == QLatin1String("target-folder")) {

      } else {
          qDebug()<<" tag not recognize "<<nexttag;
      }
  }

}

void FilterImporterSylpheed::parseActions(const QDomElement &e, MailCommon::MailFilter *filter)
{
    for ( QDomElement ruleFilter = e.firstChildElement(); !ruleFilter.isNull(); ruleFilter = ruleFilter.nextSiblingElement() )
    {
        QString actionName;
        QString value;
        const QString nexttag = ruleFilter.tagName();
        if(nexttag == QLatin1String("move")){
            actionName = QLatin1String( "transfer" );
        } else if( nexttag == QLatin1String("copy")) {
            actionName = QLatin1String( "copy" );
        } else if( nexttag == QLatin1String("not-receive")) {
        } else if( nexttag == QLatin1String("delete")) {
            actionName = QLatin1String( "delete" );
        } else if( nexttag == QLatin1String("exec")) {
        } else if( nexttag == QLatin1String("exec-async")) {
        } else if( nexttag == QLatin1String("mark")) {
        } else if( nexttag == QLatin1String("color-label")) {
        } else if( nexttag == QLatin1String("mark-as-read")) {
        } else if( nexttag == QLatin1String("forward")) {
        } else if( nexttag == QLatin1String("forward-as-attachment")) {
        } else if( nexttag == QLatin1String("redirect")) {
        } else if( nexttag == QLatin1String("stop-eval")) {
            filter->setStopProcessingHere(true);
            break;
        } else {
            qDebug()<<" tag not recognize "<<nexttag;
        }
        createFilterAction(filter, actionName, value);
    }

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
        if ( attr == QLatin1String( "any" ) ) {
            filter->setApplyOnInbound( true );
            filter->setApplyOnExplicit( true );
        } else if(attr == QLatin1String("receiver")) {
            filter->setApplyOnInbound( true );
        } else if(attr == QLatin1String("manual")) {
            filter->setApplyOnInbound( false );
            filter->setApplyOnExplicit( true );
        } else {
          qDebug()<<" timing not defined: "<< attr;
        }

    }
    for ( QDomElement ruleFilter = e.firstChildElement(); !ruleFilter.isNull(); ruleFilter = ruleFilter.nextSiblingElement() )
    {
        const QString nexttag = ruleFilter.tagName();
        qDebug()<<" nexttag "<<nexttag;
        if(nexttag == QLatin1String("condition-list")){
          parseConditions(ruleFilter, filter);
        } else if( nexttag == QLatin1String("action-list")) {
          parseActions(ruleFilter, filter);
        }
    }

    appendFilter(filter);
}
