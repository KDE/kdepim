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

#include "filterimporterevolution.h"
#include "filter/filtermanager.h"
#include "filter/mailfilter.h"

#include <QFile>
#include <QDir>
#include "mailcommon_debug.h"
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
        qCDebug(MAILCOMMON_LOG) << "No filters defined";
        return;
    }
    filters = filters.firstChildElement(QStringLiteral("ruleset"));
    for (QDomElement e = filters.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        const QString tag = e.tagName();
        if (tag == QLatin1String("rule")) {
            parseFilters(e);
        } else {
            qCDebug(MAILCOMMON_LOG) << " unknown tag " << tag;
        }
    }
}

FilterImporterEvolution::~FilterImporterEvolution()
{
}

QString FilterImporterEvolution::defaultFiltersSettingsPath()
{
    return QStringLiteral("%1/.config/evolution/mail/filters.xml").arg(QDir::homePath());
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
            if (partFilter.hasAttribute(QStringLiteral("name"))) {
                const QString name = partFilter.attribute(QStringLiteral("name"));
                qCDebug(MAILCOMMON_LOG) << " parsePartAction name attribute :" << name;
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
                        qCDebug(MAILCOMMON_LOG) << " parttype part : name : not implemented :" << name;
                    }
                    if (fieldName.isEmpty()) {
                        qCDebug(MAILCOMMON_LOG) << " parttype part : name : not implemented :" << name;
                        continue;
                    }
                    QString contents;
                    SearchRule::Function functionName = SearchRule::FuncNone;

                    for (QDomElement valueFilter = partFilter.firstChildElement();
                            !valueFilter.isNull();
                            valueFilter = valueFilter.nextSiblingElement()) {
                        const QString valueTag = valueFilter.tagName();

                        if (valueTag == QLatin1String("value")) {

                            if (valueFilter.hasAttribute(QStringLiteral("name"))) {
                                const QString name = valueFilter.attribute(QStringLiteral("name"));
                                if (name == QLatin1String("flag")) {

                                    const QString flag = valueFilter.attribute(QStringLiteral("value"));
                                    qCDebug(MAILCOMMON_LOG) << " flag :" << flag;
                                    if (flag == QLatin1String("Seen")) {
                                        contents = QStringLiteral("Read");
                                    } else if (flag == QLatin1String("Answered")) {
                                        contents = QStringLiteral("Sent");
                                    } else if (flag == QLatin1String("Draft")) {
                                        //FIXME
                                    } else if (flag == QLatin1String("Flagged")) { //Important
                                        contents = QStringLiteral("Important");
                                    } else if (flag == QLatin1String("Junk")) {
                                        contents = QStringLiteral("Spam");
                                    } else {
                                        qCDebug(MAILCOMMON_LOG) << " unknown status flags " << flag;
                                    }
                                }
                                qCDebug(MAILCOMMON_LOG) << " value filter name :" << name;
                            }
                            if (valueFilter.hasAttribute(QStringLiteral("type"))) {
                                const QString name = valueFilter.attribute(QStringLiteral("type"));
                                if (name == QLatin1String("option")) {
                                    //Nothing we will look at value
                                } else if (name == QLatin1String("string")) {
                                    QDomElement string = valueFilter.firstChildElement();
                                    contents = string.text();
                                } else if (name == QLatin1String("folder")) {
                                    QDomElement folder = valueFilter.firstChildElement();
                                    if (folder.hasAttribute(QStringLiteral("uri"))) {
                                        contents = folder.attribute(QStringLiteral("uri"));
                                        if (!contents.isEmpty()) {
                                            contents.remove(QStringLiteral("folder://"));
                                        }
                                    }
                                } else if (name == QLatin1String("address")) {
                                    QDomElement address = valueFilter.firstChildElement();
                                    contents = address.text();
                                } else if (name == QLatin1String("integer")) {
                                    if (valueFilter.hasAttribute(QStringLiteral("integer"))) {
                                        contents = valueFilter.attribute(QStringLiteral("integer"));
                                        int val = contents.toInt();
                                        val = val * 1024; //store in Ko
                                        contents = QString::number(val);
                                    }
                                } else {
                                    qCDebug(MAILCOMMON_LOG) << " type not implemented " << name;
                                }

                            }
                            if (valueFilter.hasAttribute(QStringLiteral("value"))) {
                                const QString value = valueFilter.attribute(QStringLiteral("value"));
                                qCDebug(MAILCOMMON_LOG) << " value filter value :" << name;
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
                        actionName = QStringLiteral("transfer");
                    } else if (name == QLatin1String("copy-to-folder")) {
                        actionName = QStringLiteral("copy");
                    } else if (name == QLatin1String("delete")) {
                        actionName = QStringLiteral("delete");
                    } else if (name == QLatin1String("label")) {
                        //TODO
                    } else if (name == QLatin1String("colour")) {
                        //TODO
                    } else if (name == QLatin1String("score")) {
                        //TODO
                    } else if (name == QLatin1String("adj-score")) {
                        //TODO
                    } else if (name == QLatin1String("set-status")) {
                        actionName = QStringLiteral("set status");
                    } else if (name == QLatin1String("unset-status")) {
                        actionName = QStringLiteral("unset status");
                    } else if (name == QLatin1String("play-sound")) {
                        actionName = QStringLiteral("play sound");
                    } else if (name == QLatin1String("shell")) {
                        actionName = QStringLiteral("execute");
                    } else if (name == QLatin1String("pipe")) {
                        actionName = QStringLiteral("filter app");
                    } else if (name == QLatin1String("forward")) {
                        actionName = QStringLiteral("forward");
                    }
                    if (actionName.isEmpty()) {
                        qCDebug(MAILCOMMON_LOG) << " actiontype part : name : not implemented :" << name;
                    }
                    QString value;
                    for (QDomElement valueFilter = partFilter.firstChildElement();
                            !valueFilter.isNull();
                            valueFilter = valueFilter.nextSiblingElement()) {
                        const QString valueTag = valueFilter.tagName();
                        if (valueTag == QLatin1String("value")) {
                            if (valueFilter.hasAttribute(QStringLiteral("name"))) {
                                const QString name = valueFilter.attribute(QStringLiteral("name"));
                                qCDebug(MAILCOMMON_LOG) << " value filter name :" << name;
                            }
                            if (valueFilter.hasAttribute(QStringLiteral("type"))) {
                                const QString name = valueFilter.attribute(QStringLiteral("type"));
                                qCDebug(MAILCOMMON_LOG) << " value filter type :" << name;
                                if (name == QLatin1String("option")) {
                                    //Nothing we will look at value
                                } else if (name == QLatin1String("string")) {
                                    //TODO
                                } else if (name == QLatin1String("folder")) {
                                    QDomElement folder = valueFilter.firstChildElement();

                                    if (folder.hasAttribute(QStringLiteral("uri"))) {
                                        value = folder.attribute(QStringLiteral("uri"));
                                        if (!value.isEmpty()) {
                                            value.remove(QStringLiteral("folder://"));
                                        }
                                        qCDebug(MAILCOMMON_LOG) << " contents folder :" << value;
                                    }
                                } else if (name == QLatin1String("address")) {
                                    //TODO
                                }

                            }
                            if (valueFilter.hasAttribute(QStringLiteral("value"))) {
                                const QString name = valueFilter.attribute(QStringLiteral("value"));
                                qCDebug(MAILCOMMON_LOG) << " value filter value :" << name;
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
    if (e.hasAttribute(QStringLiteral("enabled"))) {
        const QString attr = e.attribute(QStringLiteral("enabled"));
        if (attr == QLatin1String("false")) {
            filter->setEnabled(false);
        }
    }

    if (e.hasAttribute(QStringLiteral("grouping"))) {
        const QString attr = e.attribute(QStringLiteral("grouping"));
        if (attr == QLatin1String("all")) {
            filter->pattern()->setOp(SearchPattern::OpAnd);
        } else if (attr == QLatin1String("any")) {
            filter->pattern()->setOp(SearchPattern::OpOr);
        } else {
            qCDebug(MAILCOMMON_LOG) << " grouping not implemented: " << attr;
        }

    }

    if (e.hasAttribute(QStringLiteral("source"))) {
        const QString attr = e.attribute(QStringLiteral("source"));
        if (attr == QLatin1String("incoming")) {
            filter->setApplyOnInbound(true);
        } else if (attr == QLatin1String("outgoing")) {
            filter->setApplyOnInbound(false);
            filter->setApplyOnOutbound(true);
        } else {
            qCDebug(MAILCOMMON_LOG) << " source not implemented :" << attr;
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
            qCDebug(MAILCOMMON_LOG) << " tag not implemented : " << nexttag;
        }
    }

    appendFilter(filter);
}
