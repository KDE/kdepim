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

#include "mailcommon_debug.h"
#include "filterimporterprocmail.h"
#include <KLocalizedString>
#include "filter/filtermanager.h"
#include "filter/mailfilter.h"

#include <QFile>
#include <QDir>

using namespace MailCommon;

FilterImporterProcmail::FilterImporterProcmail(QFile *file)
    : FilterImporterAbstract(), mFilterCount(0)
{
    QTextStream stream(file);
    readStream(stream);
}

FilterImporterProcmail::FilterImporterProcmail(QString string)
    : FilterImporterAbstract(),
      mFilterCount(0)
{
    QTextStream stream(&string);
    readStream(stream);
}

FilterImporterProcmail::~FilterImporterProcmail()
{
}

void FilterImporterProcmail::readStream(QTextStream &stream)
{
    MailFilter *filter = 0;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        qCDebug(MAILCOMMON_LOG) << " line :" << line << " filter " << filter;
        filter = parseLine(stream, line, filter);
    }

    appendFilter(filter);
}

QString FilterImporterProcmail::defaultFiltersSettingsPath()
{
    return QDir::homePath();
}

QString FilterImporterProcmail::createUniqFilterName()
{
    return i18n("Procmail filter %1", ++mFilterCount);
}

MailCommon::MailFilter *FilterImporterProcmail::parseLine(QTextStream &stream,
        QString line,
        MailCommon::MailFilter *filter)
{
    Q_UNUSED(stream);
    if (line.isEmpty()) {
        //Empty line
        return filter;
    } else if (line.startsWith(QLatin1Char('#'))) {
        //Commented line
        return filter;
    } else if (line.startsWith(QStringLiteral(":0"))) {
        appendFilter(filter);
        filter = new MailFilter();
        const QString uniqName = createUniqFilterName();
        filter->pattern()->setName(uniqName);
        filter->setToolbarName(uniqName);

    } else if (line.startsWith(QStringLiteral("* "))) {
        line.remove(0, 2);
        QByteArray fieldName;
        SearchRule::Function functionName = SearchRule::FuncRegExp;
        if (line.startsWith(QStringLiteral("^From:"))) {
            line.remove(QStringLiteral("^From:"));
            fieldName = "from";
        } else if (line.startsWith(QStringLiteral("^Subject:"))) {
            line.remove(QStringLiteral("^Subject:"));
            fieldName = "subject";
        } else if (line.startsWith(QStringLiteral("^Sender:"))) {
            line.remove(QStringLiteral("^Sender:"));
        } else if (line.startsWith(QStringLiteral("^(To|Cc):"))) {
            line.remove(QStringLiteral("^(To|Cc):"));
            fieldName = "<recipients>";
        } else {
            qCDebug(MAILCOMMON_LOG) << " line condition not parsed :" << line;
        }
        SearchRule::Ptr rule = SearchRule::createInstance(fieldName, functionName, line);
        filter->pattern()->append(rule);
        //Condition
    } else if (line.startsWith(QLatin1Char('!'))) {
        line.remove(QLatin1Char('!'));
        //Redirect email
    } else if (line.startsWith(QLatin1Char('|'))) {
        //Shell
        const QString actionName(QStringLiteral("execute"));
        const QString value(line);
        createFilterAction(filter, actionName, value);
    } else if (line.startsWith(QLatin1Char('{'))) {
        //Block
    } else if (line.startsWith(QLatin1Char('}'))) {
        //End block
    } else {
        const QString actionName(QStringLiteral("transfer"));
        const QString value(line);
        createFilterAction(filter, actionName, value);
        //Folder
    }

    return filter;
}
