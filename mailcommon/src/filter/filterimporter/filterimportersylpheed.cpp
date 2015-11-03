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

#include "filterimportersylpheed.h"
#include "filter/filtermanager.h"
#include "filter/mailfilter.h"
#include "mailcommon_debug.h"

#include <QFile>
#include <QDir>

using namespace MailCommon;

FilterImporterSylpheed::FilterImporterSylpheed(QFile *file)
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

    for (QDomElement e = filters.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        const QString tag = e.tagName();
        if (tag == QLatin1String("rule")) {
            parseFilters(e);
        } else {
            qCDebug(MAILCOMMON_LOG) << " unknown tag " << tag;
        }
    }
}

FilterImporterSylpheed::~FilterImporterSylpheed()
{
}

QString FilterImporterSylpheed::defaultFiltersSettingsPath()
{
    return QStringLiteral("%1/.sylpheed-2.0/filter.xml").arg(QDir::homePath());
}

void FilterImporterSylpheed::parseConditions(const QDomElement &e, MailCommon::MailFilter *filter)
{
    if (e.hasAttribute(QStringLiteral("bool"))) {
        const QString attr = e.attribute(QStringLiteral("bool"));
        if (attr == QLatin1String("and")) {
            filter->pattern()->setOp(SearchPattern::OpAnd);
        } else if (attr == QLatin1String("or")) {
            filter->pattern()->setOp(SearchPattern::OpOr);
        } else {
            qCDebug(MAILCOMMON_LOG) << " bool not defined: " << attr;
        }
    }
    for (QDomElement ruleFilter = e.firstChildElement();
            !ruleFilter.isNull();
            ruleFilter = ruleFilter.nextSiblingElement()) {
        QString contentsName;
        QByteArray fieldName;
        SearchRule::Function functionName = SearchRule::FuncNone;

        const QString nexttag = ruleFilter.tagName();
        if (nexttag == QLatin1String("match-header")) {
            if (ruleFilter.hasAttribute(QStringLiteral("name"))) {
                const QString attr = ruleFilter.attribute(QStringLiteral("name"));
                if (attr == QLatin1String("From")) {
                    fieldName = "from";
                } else if (attr == QLatin1String("Cc")) {
                    fieldName = "cc";
                } else if (attr == QLatin1String("To")) {
                    fieldName = "to";
                } else if (attr == QLatin1String("Reply-To")) {
                    fieldName = "reply-to";
                } else if (attr == QLatin1String("Subject")) {
                    fieldName = "subject";
                } else if (attr == QLatin1String("List-Id")) {
                    fieldName = "list-id";
                } else if (attr == QLatin1String("X-ML-Name")) {
                    fieldName = "x-mailing-list";
                }
                if (fieldName.isEmpty()) {
                    qCDebug(MAILCOMMON_LOG) << " match-header not implemented " << attr;
                }
            }
            contentsName = ruleFilter.text();
        } else if (nexttag == QLatin1String("match-any-header")) {
            fieldName = "<any header>";
            contentsName = ruleFilter.text();
        } else if (nexttag == QLatin1String("match-to-or-cc")) {
            fieldName = "<recipients>";
            contentsName = ruleFilter.text();
        } else if (nexttag == QLatin1String("match-body-text")) {
            fieldName = "<body>";
            contentsName = ruleFilter.text();
        } else if (nexttag == QLatin1String("command-test")) {
            //TODO
            //Not implemented in kmail
        } else if (nexttag == QLatin1String("size")) {
            fieldName = "<size>";
            contentsName = QString::number(ruleFilter.text().toInt() * 1024); //Stored as kb
        } else if (nexttag == QLatin1String("age")) {
            fieldName = "<age in days>";
            contentsName = ruleFilter.text();
        } else if (nexttag == QLatin1String("unread")) {
            fieldName = "<status>";
            contentsName = QStringLiteral("Unread");
        } else if (nexttag == QLatin1String("mark")) {
            //TODO
        } else if (nexttag == QLatin1String("color-label")) {
            //TODO
        } else if (nexttag == QLatin1String("mime")) {
            //TODO
        } else if (nexttag == QLatin1String("account-id")) {
            //TODO
        } else if (nexttag == QLatin1String("target-folder")) {
            //TODO
        } else {
            qCDebug(MAILCOMMON_LOG) << " tag not recognize " << nexttag;
        }
        if (fieldName.isEmpty()) {
            qCDebug(MAILCOMMON_LOG) << " field not implemented " << nexttag;
        }

        if (ruleFilter.hasAttribute(QStringLiteral("type"))) {
            const QString attr = ruleFilter.attribute(QStringLiteral("type"));
            if (attr == QLatin1String("not-contain")) {
                functionName = SearchRule::FuncContainsNot;
            } else if (attr == QLatin1String("contains")) {
                functionName = SearchRule::FuncContains;
            } else if (attr == QLatin1String("is-not")) {
                functionName = SearchRule::FuncNotEqual;
            } else if (attr == QLatin1String("is")) {
                functionName = SearchRule::FuncEquals;
            } else if (attr == QLatin1String("not-regex")) {
                functionName = SearchRule::FuncNotRegExp;
            } else if (attr == QLatin1String("regex")) {
                functionName = SearchRule::FuncRegExp;
            } else if (attr == QLatin1String("not-in-addressbook")) {
                functionName = SearchRule::FuncIsNotInAddressbook;
            } else if (attr == QLatin1String("in-addressbook")) {
                functionName = SearchRule::FuncIsInAddressbook;
            } else if (attr == QLatin1String("gt")) {
                functionName = SearchRule::FuncIsGreater;
            } else if (attr == QLatin1String("lt")) {
                functionName = SearchRule::FuncIsLess;
            } else {
                qCDebug(MAILCOMMON_LOG) << " Attr type not implemented :" << attr;
            }
        }
        SearchRule::Ptr rule = SearchRule::createInstance(fieldName, functionName, contentsName);
        filter->pattern()->append(rule);
    }
}

void FilterImporterSylpheed::parseActions(const QDomElement &e,
        MailCommon::MailFilter *filter)
{
    for (QDomElement ruleFilter = e.firstChildElement();
            !ruleFilter.isNull();
            ruleFilter = ruleFilter.nextSiblingElement()) {
        QString actionName;
        const QString nexttag = ruleFilter.tagName();
        QString value = ruleFilter.text();
        if (nexttag == QLatin1String("move")) {
            actionName = QStringLiteral("transfer");
            value = ruleFilter.text();
        } else if (nexttag == QLatin1String("copy")) {
            actionName = QStringLiteral("copy");
            value = ruleFilter.text();
        } else if (nexttag == QLatin1String("not-receive")) {
            //TODO
        } else if (nexttag == QLatin1String("delete")) {
            actionName = QStringLiteral("delete");
        } else if (nexttag == QLatin1String("exec")) {
            actionName = QStringLiteral("execute");
            value = ruleFilter.text();
        } else if (nexttag == QLatin1String("exec-async")) {
            actionName = QStringLiteral("filter app");
            value = ruleFilter.text();
        } else if (nexttag == QLatin1String("mark")) {
            //FIXME add tag ?
        } else if (nexttag == QLatin1String("color-label")) {
            //TODO
        } else if (nexttag == QLatin1String("mark-as-read")) {
            actionName = QStringLiteral("set status");
            value = QStringLiteral("R");
        } else if (nexttag == QLatin1String("forward")) {
            actionName = QStringLiteral("forward");
            value = ruleFilter.text();
        } else if (nexttag == QLatin1String("forward-as-attachment")) {
            //TODO
        } else if (nexttag == QLatin1String("redirect")) {
            actionName = QStringLiteral("redirect");
            value = ruleFilter.text();
        } else if (nexttag == QLatin1String("stop-eval")) {
            filter->setStopProcessingHere(true);
            break;
        }

        if (actionName.isEmpty()) {
            qCDebug(MAILCOMMON_LOG) << " tag not recognize " << nexttag;
        }
        createFilterAction(filter, actionName, value);
    }
}

void FilterImporterSylpheed::parseFilters(const QDomElement &e)
{
    MailCommon::MailFilter *filter = new MailCommon::MailFilter();
    if (e.hasAttribute(QStringLiteral("enabled"))) {
        const QString attr = e.attribute(QStringLiteral("enabled"));
        if (attr == QLatin1String("false")) {
            filter->setEnabled(false);
        }
    }

    if (e.hasAttribute(QStringLiteral("name"))) {
        const QString attr = e.attribute(QStringLiteral("name"));
        filter->pattern()->setName(attr);
        filter->setToolbarName(attr);
    }

    if (e.hasAttribute(QStringLiteral("timing"))) {
        const QString attr = e.attribute(QStringLiteral("timing"));
        if (attr == QLatin1String("any")) {
            filter->setApplyOnInbound(true);
            filter->setApplyOnExplicit(true);
        } else if (attr == QLatin1String("receiver")) {
            filter->setApplyOnInbound(true);
        } else if (attr == QLatin1String("manual")) {
            filter->setApplyOnInbound(false);
            filter->setApplyOnExplicit(true);
        } else {
            qCDebug(MAILCOMMON_LOG) << " timing not defined: " << attr;
        }

    }
    for (QDomElement ruleFilter = e.firstChildElement();
            !ruleFilter.isNull();
            ruleFilter = ruleFilter.nextSiblingElement()) {
        const QString nexttag = ruleFilter.tagName();
        if (nexttag == QLatin1String("condition-list")) {
            parseConditions(ruleFilter, filter);
        } else if (nexttag == QLatin1String("action-list")) {
            parseActions(ruleFilter, filter);
        } else {
            qCDebug(MAILCOMMON_LOG) << " next tag not implemented " << nexttag;
        }
    }

    appendFilter(filter);
}
