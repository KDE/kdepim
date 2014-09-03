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

#include "filterimporterevolution_p.h"
#include "filtermanager.h"
#include "mailfilter.h"

#include <QDebug>

#include <QFile>
#include <QDir>

using namespace MailCommon;

FilterImporterEvolution::FilterImporterEvolution(QFile *file)
    : FilterImporterAbstract()
{
    QDomDocument doc;
    if (!loadDomElement(doc, file)) {
        return;
    }

    QDomElement filters = doc.documentElement();

    if (filters.isNull()) {
        qDebug() << "No filters defined";
        return;
    }
    filters = filters.firstChildElement(QLatin1String("ruleset"));
    for (QDomElement e = filters.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        const QString tag = e.tagName();
        if (tag == QLatin1String("rule")) {
            parseFilters(e);
        } else {
            qDebug() << " unknown tag " << tag;
        }
    }
}

FilterImporterEvolution::~FilterImporterEvolution()
{
}

QString FilterImporterEvolution::defaultFiltersSettingsPath()
{
    return QString::fromLatin1("%1/.config/evolution/mail/filters.xml").arg(QDir::homePath());
}

void FilterImporterEvolution::parsePartAction(const QDomElement &ruleFilter,
        MailCommon::MailFilter *filter,
        parseType type)
{
    for (QDomElement partFilter = ruleFilter.firstChildElement();
            !partFilter.isNull();
            partFilter = partFilter.nextSiblingElement()) {
        const QString nexttag = partFilter.tagName();
        if (nexttag == QLatin1String("part")) {
            if (partFilter.hasAttribute(QLatin1String("name"))) {
                const QString name = partFilter.attribute(QLatin1String("name"));
                qDebug() << " parsePartAction name attribute :" << name;
                if (type == FilterImporterEvolution::PartType) {
                    QByteArray fieldName;

                    if (name == QLatin1String("to")) {
                        fieldName = "to";
                    } else if (name == QLatin1String("sender")) {
                        fieldName = "from";
                    } else if (name == QLatin1String("cc")) {
                        fieldName = "cc";
                    } else if (name == QLatin1String("bcc")) {
                        fieldName = "bcc"; //Verify
                        //TODO
                    } else if (name == QLatin1String("senderto")) {
                        //TODO
                    } else if (name == QLatin1String("subject")) {
                        fieldName = "subject";
                    } else if (name == QLatin1String("header")) {
                        fieldName = "<any header>";
                    } else if (name == QLatin1String("body")) {
                        fieldName = "<body>";
                    } else if (name == QLatin1String("sexp")) {
                        //TODO
                    } else if (name == QLatin1String("sent-date")) {
                        //TODO
                    } else if (name == QLatin1String("recv-date")) {
                        fieldName = "<date>";
                    } else if (name == QLatin1String("label")) {
                        //TODO
                    } else if (name == QLatin1String("score")) {
                        //TODO
                    } else if (name == QLatin1String("size")) {
                        fieldName = "<size>";
                    } else if (name == QLatin1String("status")) {
                        fieldName = "<status>";
                    } else if (name == QLatin1String("follow-up")) {
                        //TODO
                    } else if (name == QLatin1String("completed-on")) {
                        //TODO
                    } else if (name == QLatin1String("attachments")) {
                        //TODO
                    } else if (name == QLatin1String("mlist")) {
                        fieldName = "list-id"; //Verify
                    } else if (name == QLatin1String("regex")) {
                        //TODO
                    } else if (name == QLatin1String("source")) {
                        //TODO
                    } else if (name == QLatin1String("pipe")) {
                        //TODO
                    } else if (name == QLatin1String("junk")) {
                        //TODO
                    } else if (name == QLatin1String("all")) {
                        filter->pattern()->setOp(SearchPattern::OpAll);
                        break;
                    } else {
                        qDebug() << " parttype part : name : not implemented :" << name;
                    }
                    if (fieldName.isEmpty()) {
                        qDebug() << " parttype part : name : not implemented :" << name;
                        continue;
                    }
                    QString contents;
                    SearchRule::Function functionName = SearchRule::FuncNone;

                    for (QDomElement valueFilter = partFilter.firstChildElement();
                            !valueFilter.isNull();
                            valueFilter = valueFilter.nextSiblingElement()) {
                        const QString valueTag = valueFilter.tagName();

                        if (valueTag == QLatin1String("value")) {

                            if (valueFilter.hasAttribute(QLatin1String("name"))) {
                                const QString name = valueFilter.attribute(QLatin1String("name"));
                                if (name == QLatin1String("flag")) {

                                    const QString flag = valueFilter.attribute(QLatin1String("value"));
                                    qDebug() << " flag :" << flag;
                                    if (flag == QLatin1String("Seen")) {
                                        contents = QLatin1String("Read");
                                    } else if (flag == QLatin1String("Answered")) {
                                        contents = QLatin1String("Sent");
                                    } else if (flag == QLatin1String("Draft")) {
                                        //FIXME
                                    } else if (flag == QLatin1String("Flagged")) { //Important
                                        contents = QLatin1String("Important");
                                    } else if (flag == QLatin1String("Junk")) {
                                        contents = QLatin1String("Spam");
                                    } else {
                                        qDebug() << " unknown status flags " << flag;
                                    }
                                }
                                qDebug() << " value filter name :" << name;
                            }
                            if (valueFilter.hasAttribute(QLatin1String("type"))) {
                                const QString name = valueFilter.attribute(QLatin1String("type"));
                                if (name == QLatin1String("option")) {
                                    //Nothing we will look at value
                                } else if (name == QLatin1String("string")) {
                                    QDomElement string = valueFilter.firstChildElement();
                                    contents = string.text();
                                } else if (name == QLatin1String("folder")) {
                                    QDomElement folder = valueFilter.firstChildElement();
                                    if (folder.hasAttribute(QLatin1String("uri"))) {
                                        contents = folder.attribute(QLatin1String("uri"));
                                        if (!contents.isEmpty()) {
                                            contents.remove(QLatin1String("folder://"));
                                        }
                                    }
                                } else if (name == QLatin1String("address")) {
                                    QDomElement address = valueFilter.firstChildElement();
                                    contents = address.text();
                                } else if (name == QLatin1String("integer")) {
                                    if (valueFilter.hasAttribute(QLatin1String("integer"))) {
                                        contents = valueFilter.attribute(QLatin1String("integer"));
                                        int val = contents.toInt();
                                        val = val * 1024; //store in Ko
                                        contents = QString::number(val);
                                    }
                                } else {
                                    qDebug() << " type not implemented " << name;
                                }

                            }
                            if (valueFilter.hasAttribute(QLatin1String("value"))) {
                                const QString value = valueFilter.attribute(QLatin1String("value"));
                                qDebug() << " value filter value :" << name;
                                if (value == QLatin1String("contains")) {
                                    functionName = SearchRule::FuncContains;
                                } else if (value == QLatin1String("not contains")) {
                                    functionName = SearchRule::FuncContainsNot;
                                } else if (value == QLatin1String("is not")) {
                                    functionName = SearchRule::FuncNotEqual;
                                } else if (value == QLatin1String("is")) {
                                    functionName = SearchRule::FuncEquals;
                                } else if (value == QLatin1String("exist")) {
                                    //TODO
                                } else if (value == QLatin1String("not exist")) {
                                    //TODO
                                } else if (value == QLatin1String("not starts with")) {
                                    functionName = SearchRule::FuncNotStartWith;
                                } else if (value == QLatin1String("ends with")) {
                                    functionName = SearchRule::FuncEndWith;
                                } else if (value == QLatin1String("not ends with")) {
                                    functionName = SearchRule::FuncNotEndWith;
                                } else if (value == QLatin1String("matches soundex")) {
                                    //TODO
                                } else if (value == QLatin1String("not match soundex")) {
                                    //TODO
                                } else if (value == QLatin1String("before")) {
                                    //TODO
                                } else if (value == QLatin1String("after")) {
                                    //TODO
                                } else if (value == QLatin1String("greater-than")) {
                                    functionName = SearchRule::FuncIsGreater;
                                } else if (value == QLatin1String("less-than")) {
                                    functionName = SearchRule::FuncIsLess;
                                } else if (value == QLatin1String("starts with")) {
                                    functionName = SearchRule::FuncStartWith;
                                }
                            }
                        }
                    }
                    SearchRule::Ptr rule = SearchRule::createInstance(fieldName, functionName, contents);
                    filter->pattern()->append(rule);

                } else if (type == FilterImporterEvolution::ActionType) {
                    QString actionName;
                    if (name == QLatin1String("stop")) {
                        filter->setStopProcessingHere(true);
                        break;
                    } else if (name == QLatin1String("move-to-folder")) {
                        actionName = QLatin1String("transfer");
                    } else if (name == QLatin1String("copy-to-folder")) {
                        actionName = QLatin1String("copy");
                    } else if (name == QLatin1String("delete")) {
                        actionName = QLatin1String("delete");
                    } else if (name == QLatin1String("label")) {
                        //TODO
                    } else if (name == QLatin1String("colour")) {
                        //TODO
                    } else if (name == QLatin1String("score")) {
                        //TODO
                    } else if (name == QLatin1String("adj-score")) {
                        //TODO
                    } else if (name == QLatin1String("set-status")) {
                        actionName = QLatin1String("set status");
                    } else if (name == QLatin1String("unset-status")) {
                        actionName = QLatin1String("unset status");
                    } else if (name == QLatin1String("beep")) {
                        actionName = QLatin1String("beep");
                    } else if (name == QLatin1String("play-sound")) {
                        actionName = QLatin1String("play sound");
                    } else if (name == QLatin1String("shell")) {
                        actionName = QLatin1String("execute");
                    } else if (name == QLatin1String("pipe")) {
                        actionName = QLatin1String("filter app");
                    } else if (name == QLatin1String("forward")) {
                        actionName = QLatin1String("forward");
                    }
                    if (actionName.isEmpty()) {
                        qDebug() << " actiontype part : name : not implemented :" << name;
                    }
                    QString value;
                    for (QDomElement valueFilter = partFilter.firstChildElement();
                            !valueFilter.isNull();
                            valueFilter = valueFilter.nextSiblingElement()) {
                        const QString valueTag = valueFilter.tagName();
                        if (valueTag == QLatin1String("value")) {
                            if (valueFilter.hasAttribute(QLatin1String("name"))) {
                                const QString name = valueFilter.attribute(QLatin1String("name"));
                                qDebug() << " value filter name :" << name;
                            }
                            if (valueFilter.hasAttribute(QLatin1String("type"))) {
                                const QString name = valueFilter.attribute(QLatin1String("type"));
                                qDebug() << " value filter type :" << name;
                                if (name == QLatin1String("option")) {
                                    //Nothing we will look at value
                                } else if (name == QLatin1String("string")) {
                                    //TODO
                                } else if (name == QLatin1String("folder")) {
                                    QDomElement folder = valueFilter.firstChildElement();

                                    if (folder.hasAttribute(QLatin1String("uri"))) {
                                        value = folder.attribute(QLatin1String("uri"));
                                        if (!value.isEmpty()) {
                                            value.remove(QLatin1String("folder://"));
                                        }
                                        qDebug() << " contents folder :" << value;
                                    }
                                } else if (name == QLatin1String("address")) {
                                    //TODO
                                }

                            }
                            if (valueFilter.hasAttribute(QLatin1String("value"))) {
                                const QString name = valueFilter.attribute(QLatin1String("value"));
                                qDebug() << " value filter value :" << name;
                                if (value == QLatin1String("contains")) {
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
    if (e.hasAttribute(QLatin1String("enabled"))) {
        const QString attr = e.attribute(QLatin1String("enabled"));
        if (attr == QLatin1String("false")) {
            filter->setEnabled(false);
        }
    }

    if (e.hasAttribute(QLatin1String("grouping"))) {
        const QString attr = e.attribute(QLatin1String("grouping"));
        if (attr == QLatin1String("all")) {
            filter->pattern()->setOp(SearchPattern::OpAnd);
        } else if (attr == QLatin1String("any")) {
            filter->pattern()->setOp(SearchPattern::OpOr);
        } else {
            qDebug() << " grouping not implemented: " << attr;
        }

    }

    if (e.hasAttribute(QLatin1String("source"))) {
        const QString attr = e.attribute(QLatin1String("source"));
        if (attr == QLatin1String("incoming")) {
            filter->setApplyOnInbound(true);
        } else if (attr == QLatin1String("outgoing")) {
            filter->setApplyOnInbound(false);
            filter->setApplyOnOutbound(true);
        } else {
            qDebug() << " source not implemented :" << attr;
        }
    }
    for (QDomElement ruleFilter = e.firstChildElement();
            !ruleFilter.isNull();
            ruleFilter = ruleFilter.nextSiblingElement()) {
        const QString nexttag = ruleFilter.tagName();
        if (nexttag == QLatin1String("title")) {
            filter->pattern()->setName(ruleFilter.text());
            filter->setToolbarName(ruleFilter.text());
        } else if (nexttag == QLatin1String("partset")) {
            parsePartAction(ruleFilter, filter, PartType);
        } else if (nexttag == QLatin1String("actionset")) {
            parsePartAction(ruleFilter, filter, ActionType);
        } else {
            qDebug() << " tag not implemented : " << nexttag;
        }
    }

    appendFilter(filter);
}
