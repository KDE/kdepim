/*
  Copyright (c) 2011-2015 Montel Laurent <montel@kde.org>

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

#include "filterimporterthunderbird.h"
#include "filter/mailfilter.h"
#include "mailcommon_debug.h"
#include <QUrl>

#include <QFile>
#include <QDir>

using namespace MailCommon;

FilterImporterThunderbird::FilterImporterThunderbird(QFile *file, bool interactive)
    : FilterImporterAbstract(interactive)
{
    QTextStream stream(file);
    readStream(stream);
}

FilterImporterThunderbird::FilterImporterThunderbird(QString string, bool interactive)
    : FilterImporterAbstract(interactive)
{
    QTextStream stream(&string);
    readStream(stream);
}

FilterImporterThunderbird::~FilterImporterThunderbird()
{
}

void FilterImporterThunderbird::readStream(QTextStream &stream)
{
    MailFilter *filter = Q_NULLPTR;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        qCDebug(MAILCOMMON_LOG) << " line :" << line << " filter " << filter;
        filter = parseLine(stream, line, filter);
    }
    //TODO show limit of action/condition
    appendFilter(filter);
}

QString FilterImporterThunderbird::defaultIcedoveFiltersSettingsPath()
{
    return QStringLiteral("%1/.icedove/").arg(QDir::homePath());
}

QString FilterImporterThunderbird::defaultThunderbirdFiltersSettingsPath()
{
    return QStringLiteral("%1/.thunderbird/").arg(QDir::homePath());
}

MailCommon::MailFilter *FilterImporterThunderbird::parseLine(QTextStream &stream,
        QString line,
        MailCommon::MailFilter *filter)
{
    if (line.startsWith(QStringLiteral("name="))) {
        appendFilter(filter);
        filter = new MailFilter();
        line = cleanArgument(line, QStringLiteral("name="));
        filter->pattern()->setName(line);
        filter->setToolbarName(line);
    } else if (line.startsWith(QStringLiteral("action="))) {
        line = cleanArgument(line, QStringLiteral("action="));
        QString value;
        QString actionName = extractActions(line, filter, value);
        if (!stream.atEnd()) {
            line = stream.readLine();
            if (line.startsWith(QStringLiteral("actionValue="))) {
                value = cleanArgument(line, QStringLiteral("actionValue="));
                //change priority
                if (actionName == QLatin1String("Change priority")) {
                    QStringList lstValue;
                    lstValue << QStringLiteral("X-Priority");
                    if (value == QLatin1String("Highest")) {
                        value = QStringLiteral("1 (Highest)");
                    } else if (value == QLatin1String("High")) {
                        value = QStringLiteral("2 (High)");
                    } else if (value == QLatin1String("Normal")) {
                        value = QStringLiteral("3 (Normal)");
                    } else if (value == QLatin1String("Low")) {
                        value = QStringLiteral("4 (Low)");
                    } else if (value == QLatin1String("Lowest")) {
                        value = QStringLiteral("5 (Lowest)");
                    }
                    lstValue << value;
                    value = lstValue.join(QStringLiteral("\t"));
                    actionName = QStringLiteral("add header");
                } else if (actionName == QLatin1String("copy") || actionName == QLatin1String("transfer")) {
                    QUrl url = QUrl::fromLocalFile(value);
                    if (url.isValid()) {
                        QString path = url.path();
                        if (path.startsWith(QLatin1Char('/'))) {
                            path.remove(0, 1); //Remove '/'
                        }
                        value = path;
                    }
                }
                createFilterAction(filter, actionName, value);
            } else {
                createFilterAction(filter, actionName, value);
                filter = parseLine(stream, line, filter);
            }
        } else {
            createFilterAction(filter, actionName, value);
        }
    } else if (line.startsWith(QStringLiteral("enabled="))) {
        line = cleanArgument(line, QStringLiteral("enabled="));
        if (line == QLatin1String("no")) {
            filter->setEnabled(false);
        }
    } else if (line.startsWith(QStringLiteral("condition="))) {
        line = cleanArgument(line, QStringLiteral("condition="));
        extractConditions(line, filter);
    } else if (line.startsWith(QStringLiteral("type="))) {
        line = cleanArgument(line, QStringLiteral("type="));
        extractType(line, filter);
    } else if (line.startsWith(QStringLiteral("version="))) {
        line = cleanArgument(line, QStringLiteral("version="));
        if (line.toInt() != 9) {
            qCDebug(MAILCOMMON_LOG) << " thunderbird filter version different of 9 need to look at if it changed";
        }
    } else if (line.startsWith(QStringLiteral("logging="))) {
        line = cleanArgument(line, QStringLiteral("logging="));
        if (line == QLatin1String("no")) {
            //TODO
        } else if (line == QLatin1String("yes")) {
            //TODO
        } else {
            qCDebug(MAILCOMMON_LOG) << " Logging option not implemented " << line;
        }
    } else {
        qCDebug(MAILCOMMON_LOG) << "unknown tag : " << line;
    }
    return filter;
}

void FilterImporterThunderbird::extractConditions(const QString &line,
        MailCommon::MailFilter *filter)
{
    if (line.startsWith(QStringLiteral("AND"))) {
        filter->pattern()->setOp(SearchPattern::OpAnd);
        const QStringList conditionsList = line.split(QStringLiteral("AND "));
        const int numberOfCond(conditionsList.count());
        for (int i = 0; i < numberOfCond; ++i) {
            if (!conditionsList.at(i).trimmed().isEmpty()) {
                splitConditions(conditionsList.at(i), filter);
            }
        }
    } else if (line.startsWith(QStringLiteral("OR"))) {
        filter->pattern()->setOp(SearchPattern::OpOr);
        const QStringList conditionsList = line.split(QStringLiteral("OR "));
        const int numberOfCond(conditionsList.count());
        for (int i = 0; i < numberOfCond; ++i) {
            if (!conditionsList.at(i).trimmed().isEmpty()) {
                splitConditions(conditionsList.at(i), filter);
            }
        }
    } else if (line.startsWith(QStringLiteral("ALL"))) {
        filter->pattern()->setOp(SearchPattern::OpAll);
    } else {
        qCDebug(MAILCOMMON_LOG) << " missing extract condition" << line;
    }
}

bool FilterImporterThunderbird::splitConditions(const QString &cond,
        MailCommon::MailFilter *filter)
{
    /*
    *    {nsMsgSearchAttrib::Subject,    "subject"},
    {nsMsgSearchAttrib::Sender,     "from"},
    {nsMsgSearchAttrib::Body,       "body"},
    {nsMsgSearchAttrib::Date,       "date"},
    {nsMsgSearchAttrib::Priority,   "priority"},
    {nsMsgSearchAttrib::MsgStatus,  "status"},
    {nsMsgSearchAttrib::To,         "to"},
    {nsMsgSearchAttrib::CC,         "cc"},
    {nsMsgSearchAttrib::ToOrCC,     "to or cc"},
    {nsMsgSearchAttrib::AllAddresses, "all addresses"},
    {nsMsgSearchAttrib::AgeInDays,  "age in days"},
    {nsMsgSearchAttrib::Label,      "label"},
    {nsMsgSearchAttrib::Keywords,   "tag"},
    {nsMsgSearchAttrib::Size,       "size"},
    // this used to be nsMsgSearchAttrib::SenderInAddressBook
    // we used to have two Sender menuitems
    // for backward compatibility, we can still parse
    // the old style.  see bug #179803
    {nsMsgSearchAttrib::Sender,     "from in ab"},
    {nsMsgSearchAttrib::JunkStatus, "junk status"},
    {nsMsgSearchAttrib::JunkPercent, "junk percent"},
    {nsMsgSearchAttrib::JunkScoreOrigin, "junk score origin"},
    {nsMsgSearchAttrib::HasAttachmentStatus, "has attachment status"},

    */

    QString str = cond.trimmed();
    str.remove(QLatin1Char('('));
    str.remove(str.length() - 1, 1);   //remove last )

    const QStringList listOfCond = str.split(QLatin1Char(','));
    if (listOfCond.count() < 3) {
        qCDebug(MAILCOMMON_LOG) << "We have a pb in cond:" << cond;
        return false;
    }
    const QString field = listOfCond.at(0);
    const QString function = listOfCond.at(1);
    const QString contents = listOfCond.at(2);

    QByteArray fieldName;
    if (field == QLatin1String("subject")) {
        fieldName = "subject";
    } else if (field == QLatin1String("from")) {
        fieldName = "from";
    } else if (field == QLatin1String("body")) {
        fieldName = "<body>";
    } else if (field == QLatin1String("date")) {
        fieldName = "<date>";
    } else if (field == QLatin1String("priority")) {
        //TODO
    } else if (field == QLatin1String("status")) {
        fieldName = "<status>";
    } else if (field == QLatin1String("to")) {
        fieldName = "to";
    } else if (field == QLatin1String("cc")) {
        fieldName = "cc";
    } else if (field == QLatin1String("to or cc")) {
        fieldName = "<recipients>";
    } else if (field == QLatin1String("all addresses")) {
        fieldName = "<recipients>";
    } else if (field == QLatin1String("age in days")) {
        fieldName = "<age in days>";
    } else if (field == QLatin1String("label")) {
        //TODO
    } else if (field == QLatin1String("tag")) {
        fieldName = "<tag>";
    } else if (field == QLatin1String("size")) {
        fieldName = "<size>";
    } else if (field == QLatin1String("from in ab")) {
        //TODO
    } else if (field == QLatin1String("junk status")) {
        //TODO
    } else if (field == QLatin1String("junk percent")) {
        //TODO
    } else if (field == QLatin1String("junk score origin")) {
        //TODO
    } else if (field == QLatin1String("has attachment status")) {
        //TODO
    }

    if (fieldName.isEmpty()) {
        qCDebug(MAILCOMMON_LOG) << " Field not implemented: " << field;
    }
    /*
    {nsMsgSearchOp::Contains, "contains"},
    {nsMsgSearchOp::DoesntContain,"doesn't contain"},
    {nsMsgSearchOp::Is,"is"},
    {nsMsgSearchOp::Isnt,  "isn't"},
    {nsMsgSearchOp::IsEmpty, "is empty"},
    {nsMsgSearchOp::IsntEmpty, "isn't empty"},
    {nsMsgSearchOp::IsBefore, "is before"},
    {nsMsgSearchOp::IsAfter, "is after"},
    {nsMsgSearchOp::IsHigherThan, "is higher than"},
    {nsMsgSearchOp::IsLowerThan, "is lower than"},
    {nsMsgSearchOp::BeginsWith, "begins with"},
    {nsMsgSearchOp::EndsWith, "ends with"},
    {nsMsgSearchOp::IsInAB, "is in ab"},
    {nsMsgSearchOp::IsntInAB, "isn't in ab"},
    {nsMsgSearchOp::IsGreaterThan, "is greater than"},
    {nsMsgSearchOp::IsLessThan, "is less than"},
    {nsMsgSearchOp::Matches, "matches"},
    {nsMsgSearchOp::DoesntMatch, "doesn't match"}
    */
    SearchRule::Function functionName = SearchRule::FuncNone;

    if (function == QLatin1String("contains")) {
        functionName = SearchRule::FuncContains;
    } else if (function == QLatin1String("doesn't contain")) {
        functionName = SearchRule::FuncContainsNot;
    } else if (function == QLatin1String("is")) {
        functionName = SearchRule::FuncEquals;
    } else if (function == QLatin1String("isn't")) {
        functionName = SearchRule::FuncNotEqual;
    } else if (function == QLatin1String("is empty")) {
        //TODO
    } else if (function == QLatin1String("isn't empty")) {
        //TODO
    } else if (function == QLatin1String("is before")) {
        functionName = SearchRule::FuncIsLess;
    } else if (function == QLatin1String("is after")) {
        functionName = SearchRule::FuncIsGreater;
    } else if (function == QLatin1String("is higher than")) {
        functionName = SearchRule::FuncIsGreater;
    } else if (function == QLatin1String("is lower than")) {
        functionName = SearchRule::FuncIsLess;
    } else if (function == QLatin1String("begins with")) {
        functionName = SearchRule::FuncStartWith;
    } else if (function == QLatin1String("ends with")) {
        functionName = SearchRule::FuncEndWith;
    } else if (function == QLatin1String("is in ab")) {
        functionName = SearchRule::FuncIsInAddressbook;
    } else if (function == QLatin1String("isn't in ab")) {
        functionName = SearchRule::FuncIsNotInAddressbook;
    } else if (function == QLatin1String("is greater than")) {
        functionName = SearchRule::FuncIsGreater;
    } else if (function == QLatin1String("is less than")) {
        functionName = SearchRule::FuncIsLess;
    } else if (function == QLatin1String("matches")) {
        functionName = SearchRule::FuncEquals;
    } else if (function == QLatin1String("doesn't match")) {
        functionName = SearchRule::FuncNotEqual;
    }

    if (functionName == SearchRule::FuncNone) {
        qCDebug(MAILCOMMON_LOG) << " functionName not implemented: " << function;
    }
    QString contentsName;
    if (fieldName == "<status>") {
        if (contents == QLatin1String("read")) {
            contentsName = QStringLiteral("Read");
        } else if (contents == QLatin1String("unread")) {
            contentsName = QStringLiteral("Unread");
        } else if (contents == QLatin1String("new")) {
            contentsName = QStringLiteral("New");
        } else if (contents == QLatin1String("forwarded")) {
            contentsName = QStringLiteral("Forwarded");
        } else {
            qCDebug(MAILCOMMON_LOG) << " contents for status not implemented " << contents;
        }
    } else if (fieldName == "<size>") {
        int value = contents.toInt();
        value = value * 1024; //Ko
        contentsName = QString::number(value);
    } else if (fieldName == "<date>") {
        QLocale locale(QLocale::C);
        const QDate date = locale.toDate(contents, QStringLiteral("dd-MMM-yyyy"));
        contentsName = date.toString(Qt::ISODate);
    } else {
        contentsName = contents;
    }

    SearchRule::Ptr rule = SearchRule::createInstance(fieldName, functionName, contentsName);
    filter->pattern()->append(rule);
    //qCDebug(MAILCOMMON_LOG) << " field :" << field << " function :" << function
    //         << " contents :" << contents << " cond :" << cond;
    return true;
}

QString FilterImporterThunderbird::extractActions(const QString &line,
        MailCommon::MailFilter *filter,
        QString &value)
{
    /*
    { nsMsgFilterAction::MoveToFolder,            "Move to folder"},
    { nsMsgFilterAction::CopyToFolder,            "Copy to folder"},
    { nsMsgFilterAction::ChangePriority,          "Change priority"},
    { nsMsgFilterAction::Delete,                  "Delete"},
    { nsMsgFilterAction::MarkRead,                "Mark read"},
    { nsMsgFilterAction::KillThread,              "Ignore thread"},
    { nsMsgFilterAction::KillSubthread,           "Ignore subthread"},
    { nsMsgFilterAction::WatchThread,             "Watch thread"},
    { nsMsgFilterAction::MarkFlagged,             "Mark flagged"},
    { nsMsgFilterAction::Label,                   "Label"},
    { nsMsgFilterAction::Reply,                   "Reply"},
    { nsMsgFilterAction::Forward,                 "Forward"},
    { nsMsgFilterAction::StopExecution,           "Stop execution"},
    { nsMsgFilterAction::DeleteFromPop3Server,    "Delete from Pop3 server"},
    { nsMsgFilterAction::LeaveOnPop3Server,       "Leave on Pop3 server"},
    { nsMsgFilterAction::JunkScore,               "JunkScore"},
    { nsMsgFilterAction::FetchBodyFromPop3Server, "Fetch body from Pop3Server"},
    { nsMsgFilterAction::AddTag,                  "AddTag"},
    { nsMsgFilterAction::Custom,                  "Custom"},
     */

    QString actionName;
    if (line == QLatin1String("Move to folder")) {
        actionName = QStringLiteral("transfer");
    } else if (line == QLatin1String("Forward")) {
        actionName = QStringLiteral("forward");
    } else if (line == QLatin1String("Mark read")) {
        actionName = QStringLiteral("set status");
        value = QStringLiteral("R");
    } else if (line == QLatin1String("Mark unread")) {
        actionName = QStringLiteral("set status");
        value = QStringLiteral("U");   //TODO verify
    } else if (line == QLatin1String("Copy to folder")) {
        actionName = QStringLiteral("copy");
    } else if (line == QLatin1String("AddTag")) {
        actionName = QStringLiteral("add tag");
    } else if (line == QLatin1String("Delete")) {
        actionName = QStringLiteral("delete");
    } else if (line == QLatin1String("Change priority")) {
        actionName = QStringLiteral("Change priority"); //Doesn't exist in kmail but we help us to importing
    } else if (line == QLatin1String("Ignore thread")) {
    } else if (line == QLatin1String("Ignore subthread")) {
    } else if (line == QLatin1String("Watch thread")) {
    } else if (line == QLatin1String("Mark flagged")) {
    } else if (line == QLatin1String("Label")) {
    } else if (line == QLatin1String("Reply")) {
        actionName = QStringLiteral("set Reply-To");
    } else if (line == QLatin1String("Stop execution")) {
        filter->setStopProcessingHere(true);
        return QString();
    } else if (line == QLatin1String("Delete from Pop3 server")) {
    } else if (line == QLatin1String("JunkScore")) {
    } else if (line == QLatin1String("Fetch body from Pop3Server")) {
    } else if (line == QLatin1String("Custom")) {
    }
    if (actionName.isEmpty()) {
        qCDebug(MAILCOMMON_LOG) << QStringLiteral(" missing convert method: %1").arg(line);
    }
    return actionName;
}

void FilterImporterThunderbird::extractType(const QString &line, MailCommon::MailFilter *filter)
{
    const int value = line.toInt();
    if (value == 1) {
        filter->setApplyOnInbound(true);
        filter->setApplyOnExplicit(false);
        //Checking mail
    } else if (value == 16) {
        filter->setApplyOnInbound(false);
        filter->setApplyOnExplicit(true);
        //Manual mail
    } else if (value == 17) {
        filter->setApplyOnInbound(true);
        filter->setApplyOnExplicit(true);
        //Checking mail or manual
    } else if (value == 32) {
        filter->setApplyOnExplicit(false);
        filter->setApplyOnOutbound(true);
        filter->setApplyOnInbound(false);
        //checking mail after classification
    } else if (value == 48) {
        filter->setApplyOnExplicit(true);
        filter->setApplyOnOutbound(true);
        filter->setApplyOnInbound(false);
        //checking mail after classification or manual check
    } else {
        qCDebug(MAILCOMMON_LOG) << " type value is not valid :" << value;
    }
}

QString FilterImporterThunderbird::cleanArgument(const QString &line, const QString &removeStr)
{
    QString str = line;
    str.remove(removeStr);
    str.remove(QStringLiteral("\""));
    str.remove(str.length(), 1);   //remove last "
    return str;
}
