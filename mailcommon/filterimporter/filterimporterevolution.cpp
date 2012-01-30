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
                    QByteArray fieldName;

                    if ( name == QLatin1String( "to" ) ) {
                        fieldName = "to";
                    } else if (  name == QLatin1String( "cc" ) ) {
                        fieldName = "cc";
                    } else if (  name == QLatin1String( "bcc" ) ) {
                    } else if (  name == QLatin1String( "senderto" ) ) {
                    } else if (  name == QLatin1String( "subject" ) ) {
                        fieldName = "subject";
                    } else if (  name == QLatin1String( "header" ) ) {
                    } else if (  name == QLatin1String( "body" ) ) {
                        fieldName = "<body>";
                    } else if (  name == QLatin1String( "sexp" ) ) {
                    } else if (  name == QLatin1String( "recv-date" ) ) {
                    } else if (  name == QLatin1String( "label" ) ) {
                    } else if (  name == QLatin1String( "score" ) ) {
                    } else if (  name == QLatin1String( "size" ) ) {
                        fieldName = "<size>";
                    } else if (  name == QLatin1String( "status" ) ) {
                        fieldName = "<status>";
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
                        break;
                    } else {
                        qDebug()<<" parttype part : name : not implemented :"<<name;
                    }
                    QString value;
                    QString contents;
                    SearchRule::Function functionName = SearchRule::FuncNone;

                    for ( QDomElement valueFilter = partFilter.firstChildElement(); !valueFilter.isNull(); valueFilter = valueFilter.nextSiblingElement() ) {
                        const QString valueTag = valueFilter.tagName();
                        if ( valueTag == QLatin1String( "value" ) ) {
                            if ( valueFilter.hasAttribute( "name" ) ) {
                                const QString name = valueFilter.attribute( "name" );
                                qDebug()<<" value filter name :"<<name;
                            } else if ( valueFilter.hasAttribute( "type" ) ) {
                                const QString name = valueFilter.attribute( "type" );
                                qDebug()<<" value filter type :"<<name;
                                if(name == QLatin1String("option")){
                                    //Nothing we will look at value
                                } else if( name == QLatin1String("string")) {
                                    //TODO
                                } else if( name == QLatin1String("folder")) {
                                    //TODO
                                } else if( name == QLatin1String("address") ) {
                                    //TODO
                                }

                            } else if ( valueFilter.hasAttribute( "value" ) ) {
                                const QString name = valueFilter.attribute( "value" );
                                qDebug()<<" value filter value :"<<name;
                                if(value == QLatin1String("contains")) {
                                    functionName = SearchRule::FuncContains;
                                } else if(value == QLatin1String("not contains")) {
                                    functionName = SearchRule::FuncContainsNot;
                                } else if(value == QLatin1String("is not")) {
                                    functionName = SearchRule::FuncNotEqual;
                                } else if(value == QLatin1String("is")) {
                                    functionName = SearchRule::FuncEquals;
                                } else if(value == QLatin1String("exist")) {
                                } else if(value == QLatin1String("not exist")) {
                                } else if(value == QLatin1String("not starts with")) {
                                    functionName = SearchRule::FuncNotStartWith;
                                } else if(value == QLatin1String("ends with")) {
                                    functionName = SearchRule::FuncEndWith;
                                } else if(value == QLatin1String("not ends with")) {
                                    functionName = SearchRule::FuncNotEndWith;
                                } else if(value == QLatin1String("matches soundex")) {
                                } else if(value == QLatin1String("not match soundex")) {
                                } else if(value == QLatin1String("before")) {
                                } else if(value == QLatin1String("after")) {
                                } else if(value == QLatin1String("greater-than")) {
                                } else if(value == QLatin1String("less-than")) {
                                } else if(value == QLatin1String("starts with")) {
                                    functionName = SearchRule::FuncStartWith;
                                }
                            }
                        }
                    }
                    SearchRule::Ptr rule = SearchRule::createInstance( fieldName, functionName,contents );
                    filter->pattern()->append( rule );

                } else if (  type == FilterImporterEvolution::ActionType ) {
                    QString actionName;
                    if ( name == QLatin1String( "stop" ) ) {
                        filter->setStopProcessingHere(true);
                        break;
                    } else if ( name == QLatin1String( "move-to-folder" ) ) {
                        actionName = QLatin1String( "transfer" );
                    } else if ( name == QLatin1String( "copy-to-folder" ) ) {
                        actionName = QLatin1String( "copy" );
                    } else if ( name == QLatin1String( "delete" ) ) {
                        actionName = QLatin1String( "delete" );
                    } else if ( name == QLatin1String( "label" ) ) {
                    } else if ( name == QLatin1String( "colour" ) ) {
                    } else if ( name == QLatin1String( "score" ) ) {
                    } else if ( name == QLatin1String( "adj-score" ) ) {
                    } else if ( name == QLatin1String( "set-status" ) ) {
                        actionName = QLatin1String("set status");
                    } else if ( name == QLatin1String( "unset-status" ) ) {
                    } else if ( name == QLatin1String( "beep" ) ) {
                        actionName = QLatin1String("beep");
                    } else if ( name == QLatin1String( "play-sound" ) ) {
                        actionName = QLatin1String("play sound");
                    } else if ( name == QLatin1String( "shell" ) ) {
                        actionName = QLatin1String("execute");
                    } else if ( name == QLatin1String( "pipe" ) ) {
                        actionName = QLatin1String("filter app");
                    } else if ( name == QLatin1String( "forward" ) ) {
                        actionName = QLatin1String( "forward" );
                    }
                    if( actionName.isEmpty() ){
                        qDebug()<<" actiontype part : name : not implemented :"<<name;
                    }
                    QString value;
                    for ( QDomElement valueFilter = partFilter.firstChildElement(); !valueFilter.isNull(); valueFilter = valueFilter.nextSiblingElement() ) {
                        const QString valueTag = valueFilter.tagName();
                        if ( valueTag == QLatin1String( "value" ) ) {
                            if ( valueFilter.hasAttribute( "name" ) ) {
                                const QString name = valueFilter.attribute( "name" );
                                qDebug()<<" value filter name :"<<name;
                            } else if ( valueFilter.hasAttribute( "type" ) ) {
                                const QString name = valueFilter.attribute( "type" );
                                qDebug()<<" value filter type :"<<name;
                                qDebug()<<" value filter type :"<<name;
                                if(name == QLatin1String("option")){
                                    //Nothing we will look at value
                                } else if( name == QLatin1String("string")) {
                                    //TODO
                                } else if( name == QLatin1String("folder")) {
                                    //TODO
                                } else if( name == QLatin1String("address") ) {
                                    //TODO
                                }

                            } else if ( valueFilter.hasAttribute( "value" ) ) {
                                const QString name = valueFilter.attribute( "value" );
                                qDebug()<<" value filter value :"<<name;
                                if(value == QLatin1String("")) {
                                  //TODO
                                }

                            }
                        }
                    }
                    createFilterAction(filter, actionName, value);
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
        } else {
          qDebug()<<" grouping not defined: "<< attr;
        }
          
    }
    if(e.hasAttribute("source"))
    {
        const QString attr = e.attribute("source");
        if ( attr == QLatin1String( "incoming" ) ) {
          filter->setApplyOnInbound( true );
        } else if ( attr == QLatin1String( "outgoing" ) ) {
          filter->setApplyOnInbound( false );
          filter->setApplyOnOutbound(true);
        } else {
          qDebug()<<" source not implemented :"<<attr;
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

    appendFilter(filter);
}
