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

#include "filterimporterevolution_p.h"
#include "mailfilter.h"
#include "filtermanager.h"

#include <kdebug.h>

#include <QDomDocument>
#include <QFile>

using namespace MailCommon;

FilterImporterEvolution::FilterImporterEvolution(QFile *file)
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
    filters = filters.firstChildElement("ruleset");
    for ( QDomElement e = filters.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
        const QString tag = e.tagName();
        if ( tag == QLatin1String( "rule" ) ) {
            parseFilters(e);
        } else {
            qDebug()<<" unknown tag "<<tag;
        }

    }
}

FilterImporterEvolution::~FilterImporterEvolution()
{
}

void FilterImporterEvolution::parsePartAction(const QDomElement &ruleFilter, MailCommon::MailFilter *filter, parseType type )
{
  for ( QDomElement partFilter = ruleFilter.firstChildElement(); !partFilter.isNull(); partFilter = partFilter.nextSiblingElement() )
  {
    const QString nexttag = partFilter.tagName();
    if ( nexttag == QLatin1String( "part" ) ) {
      if ( partFilter.hasAttribute( "name" ) ) {
        //TODO
      }
      
      for ( QDomElement valueFilter = partFilter.firstChildElement(); !valueFilter.isNull(); valueFilter = valueFilter.nextSiblingElement() ) {
        const QString valueTag = valueFilter.tagName();
        if ( valueTag == QLatin1String( "value" ) ) {
          if ( valueFilter.hasAttribute( "name" ) ) {
            
          } else if ( valueFilter.hasAttribute( "type" ) ) {
            
          } else if ( valueFilter.hasAttribute( "value" ) ) {
            
          }
        }
      }
    }
  }
}


void FilterImporterEvolution::parseFilters(const QDomElement &e)
{
    MailCommon::MailFilter *filter = new MailCommon::MailFilter();
    if( e.hasAttribute("enabled"))
    {
        const QString attr = e.attribute("enabled");
        if( attr == QLatin1String("false"))
            filter->setEnabled(false);
    }
    if( e.hasAttribute("grouping"))
    {
        const QString attr = e.attribute("grouping");
        //TODO
    }
    if(e.hasAttribute("source"))
    {
        const QString attr = e.attribute("source");
        //TODO
    }
    for ( QDomElement ruleFilter = e.firstChildElement(); !ruleFilter.isNull(); ruleFilter = ruleFilter.nextSiblingElement() )
    {
        const QString nexttag = ruleFilter.tagName();
        qDebug()<<" nexttag "<<nexttag;
        if(nexttag == QLatin1String("title")){
          filter->pattern()->setName(ruleFilter.text());
          filter->setToolbarName(ruleFilter.text());
        } else if( nexttag == QLatin1String("partset")) {
            parsePartAction(ruleFilter, filter, PartType);
        } else if( nexttag == QLatin1String("actionset")) {
            parsePartAction(ruleFilter, filter, ActionType);
        }
    }


    mListMailFilter.append( filter );
}
