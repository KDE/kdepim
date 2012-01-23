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
        const QString name = partFilter.attribute( "name" );
        qDebug()<<" parsePartAction name attribute :"<<name;
        if ( type == FilterImporterEvolution::PartType ) {
          if ( name == QLatin1String( "to" ) ) {
          } else if (  name == QLatin1String( "cc" ) ) {
          } else if (  name == QLatin1String( "bcc" ) ) {
          } else if (  name == QLatin1String( "senderto" ) ) {
          } else if (  name == QLatin1String( "subject" ) ) {
          } else if (  name == QLatin1String( "header" ) ) {
          } else if (  name == QLatin1String( "body" ) ) {
          } else if (  name == QLatin1String( "sexp" ) ) {
          } else if (  name == QLatin1String( "recv-date" ) ) {
          } else if (  name == QLatin1String( "label" ) ) {
          } else if (  name == QLatin1String( "score" ) ) {
          } else if (  name == QLatin1String( "size" ) ) {
          } else if (  name == QLatin1String( "status" ) ) {
          } else if (  name == QLatin1String( "follow-up" ) ) {
          } else if (  name == QLatin1String( "completed-on" ) ) {
          } else if (  name == QLatin1String( "attachments" ) ) {
          } else if (  name == QLatin1String( "mlist" ) ) {
          } else if (  name == QLatin1String( "regex" ) ) {
          } else if (  name == QLatin1String( "source" ) ) {
          } else if (  name == QLatin1String( "pipe" ) ) {
          } else if (  name == QLatin1String( "junk" ) ) {
          } else if (  name == QLatin1String( "all" ) ) {
            filter->pattern()->setOp(SearchPattern::OpAll);
          } else {
            qDebug()<<" parttype part : name : not implemented :"<<name;
          }
        } else if (  type == FilterImporterEvolution::ActionType ) {
          QString actionName;
          if ( name == QLatin1String( "stop" ) ) {
            filter->setStopProcessingHere(true);
          } else if ( name == QLatin1String( "move-to-folder" ) ) {
          } else if ( name == QLatin1String( "copy-to-folder" ) ) {
          } else if ( name == QLatin1String( "delete" ) ) {
          } else if ( name == QLatin1String( "label" ) ) {
          } else if ( name == QLatin1String( "colour" ) ) {
          } else if ( name == QLatin1String( "score" ) ) {
          } else if ( name == QLatin1String( "adj-score" ) ) {
          } else if ( name == QLatin1String( "set-status" ) ) {
          } else if ( name == QLatin1String( "unset-status" ) ) {
          } else if ( name == QLatin1String( "beep" ) ) {
          } else if ( name == QLatin1String( "play-sound" ) ) {
          } else if ( name == QLatin1String( "shell" ) ) {
          } else if ( name == QLatin1String( "pipe" ) ) {
          } else if ( name == QLatin1String( "forward" ) ) {
          } else {
            qDebug()<<" actiontype part : name : not implemented :"<<name;
          }

        }
      }

      for ( QDomElement valueFilter = partFilter.firstChildElement(); !valueFilter.isNull(); valueFilter = valueFilter.nextSiblingElement() ) {
        const QString valueTag = valueFilter.tagName();
        if ( valueTag == QLatin1String( "value" ) ) {
          if ( valueFilter.hasAttribute( "name" ) ) {
            const QString name = valueFilter.attribute( "name" );
            qDebug()<<" value filter name :"<<name;
          } else if ( valueFilter.hasAttribute( "type" ) ) {
            const QString name = valueFilter.attribute( "type" );
            qDebug()<<" value filter type :"<<name;

          } else if ( valueFilter.hasAttribute( "value" ) ) {
            const QString name = valueFilter.attribute( "value" );
            qDebug()<<" value filter value :"<<name;

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
        if ( attr == QLatin1String( "all" ) ) {
          filter->pattern()->setOp(SearchPattern::OpAnd);
        } else if ( attr == QLatin1String( "any" ) ) {
          filter->pattern()->setOp(SearchPattern::OpOr);
        }
    }
    if(e.hasAttribute("source"))
    {
        const QString attr = e.attribute("source");
        if ( attr == QLatin1String( "incoming" ) ) {
          filter->setApplyOnInbound( true );
        } else if ( attr == QLatin1String( "outgoing" ) ) {
          filter->setApplyOnInbound( false );
        }
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
