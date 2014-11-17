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

#include "filterimporterprocmail_p.h"

#include "filtermanager.h"
#include "mailfilter.h"

#include <QDebug>

#include <QFile>
#include <QDir>

using namespace MailCommon;

FilterImporterProcmail::FilterImporterProcmail(QFile *file)
    : FilterImporterAbstract(), mFilterCount(0)
{
    QTextStream stream(file);
    MailFilter *filter = 0;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        qDebug() << " line :" << line << " filter " << filter;
        filter = parseLine(stream, line, filter);
    }

    appendFilter(filter);
}

FilterImporterProcmail::~FilterImporterProcmail()
{
}

QString FilterImporterProcmail::defaultFiltersSettingsPath()
{
    return QDir::homePath();
}

QString FilterImporterProcmail::createUniqFilterName()
{
    return QStringLiteral("Procmail filter %1").arg(mFilterCount++);
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
    } else if (line.startsWith(QLatin1String(":0"))) {
        appendFilter(filter);
        filter = new MailFilter();
        const QString uniqName = createUniqFilterName();
        filter->pattern()->setName(uniqName);
        filter->setToolbarName(uniqName);

    } else if (line.startsWith(QLatin1String("* "))) {
        line.remove(0, 2);
        QByteArray fieldName;
        SearchRule::Function functionName = SearchRule::FuncRegExp;
        if (line.startsWith(QLatin1String("^From:"))) {
            line.remove(QLatin1String("^From:"));
            fieldName = "from";
        } else if (line.startsWith(QLatin1String("^Subject:"))) {
            line.remove(QLatin1String("^Subject:"));
            fieldName = "subject";
        } else if (line.startsWith(QLatin1String("^Sender:"))) {
            line.remove(QLatin1String("^Sender:"));
        } else if (line.startsWith(QLatin1String("^(To|Cc):"))) {
            line.remove(QLatin1String("^(To|Cc):"));
            fieldName = "<recipients>";
        } else {
            qDebug() << " line condition not parsed :" << line;
        }
        SearchRule::Ptr rule = SearchRule::createInstance(fieldName, functionName, line);
        filter->pattern()->append(rule);
        //Condition
    } else if (line.startsWith(QLatin1Char('!'))) {
        line.remove(QLatin1Char('!'));
        //Redirect email
    } else if (line.startsWith(QLatin1Char('|'))) {
        //Shell
        const QString actionName(QLatin1String("execute"));
        const QString value(line);
        createFilterAction(filter, actionName, value);
    } else if (line.startsWith(QLatin1Char('{'))) {
        //Block
    } else if (line.startsWith(QLatin1Char('}'))) {
        //End block
    } else {
        const QString actionName(QLatin1String("transfer"));
        const QString value(line);
        createFilterAction(filter, actionName, value);
        //Folder
    }

    return filter;
}
