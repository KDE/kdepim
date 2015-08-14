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

#include "filterimporterclawsmail.h"
#include "filter/filtermanager.h"
#include "filter/mailfilter.h"
#include "mailcommon_debug.h"

#include <QFile>
#include <QDir>

using namespace MailCommon;

FilterImporterClawsMails::FilterImporterClawsMails(QFile *file)
    : FilterImporterAbstract()
{
    QTextStream stream(file);
    readStream(stream);
}

FilterImporterClawsMails::FilterImporterClawsMails(QString string)
    : FilterImporterAbstract()
{
    QTextStream stream(&string);
    readStream(stream);
}

FilterImporterClawsMails::FilterImporterClawsMails(bool interactive)
    : FilterImporterAbstract(interactive)
{
}

FilterImporterClawsMails::~FilterImporterClawsMails()
{
}

void FilterImporterClawsMails::readStream(QTextStream &stream)
{
    MailFilter *filter = 0;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        qCDebug(MAILCOMMON_LOG) << " line :" << line << " filter " << filter;

        if (line.isEmpty()) {
            //Nothing
        } else if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            //TODO
        } else {
            appendFilter(filter);
            filter = parseLine(line);
        }
    }
    appendFilter(filter);
}

QString FilterImporterClawsMails::defaultFiltersSettingsPath()
{
    return QStringLiteral("%1/.claws-mail/matcherrc").arg(QDir::homePath());
}

MailFilter *FilterImporterClawsMails::parseLine(const QString &line)
{
    MailFilter *filter = new MailFilter();
    QString tmp = line;
    //Enabled ?
    if (tmp.startsWith(QStringLiteral("enabled"))) {
        filter->setEnabled(true);
        tmp.remove(QStringLiteral("enabled "));
    }

    //Filter name
    if (tmp.startsWith(QStringLiteral("rulename"))) {
        tmp.remove(QStringLiteral("rulename "));
        int pos;
        const QString name = extractString(tmp, pos);
        filter->pattern()->setName(name);
        filter->setToolbarName(name);

        tmp = tmp.mid(pos + 2); //remove "\" "
        qCDebug(MAILCOMMON_LOG) << " new tmp" << tmp;
    }

    tmp = extractConditions(tmp, filter);

    tmp = extractActions(tmp, filter);
    //TODO
    return filter;
}

QString FilterImporterClawsMails::extractActions(const QString &line, MailFilter *filter)
{
    return line;
}

QString FilterImporterClawsMails::extractConditions(const QString &line, MailFilter *filter)
{
    QByteArray fieldName;
    //Action
    if (line.startsWith(QStringLiteral("subject"))) {
        fieldName = "subject";
    } else if (line.startsWith(QStringLiteral("age_lower"))) {

    }
    filter->pattern()->setOp(SearchPattern::OpAnd);
    //TODO
    return QString();
}

QString FilterImporterClawsMails::extractString(const QString &tmp, int &pos)
{
    QString name;
    QChar previousChar;
    int i = 0;
    for (; i < tmp.length(); ++i) {
        const QChar currentChar = tmp.at(i);
        if (i == 0 && (currentChar.isSpace() || currentChar == QLatin1Char('"'))) {

        } else {
            if (currentChar != QLatin1Char('"')) {
                if (currentChar != QLatin1Char('\\')) {
                    name += currentChar;
                }
            } else {
                if (previousChar == QLatin1Char('\\')) {
                    name += currentChar;
                } else {
                    break;
                }
            }
            previousChar = currentChar;
        }
    }
    pos = i;
    qCDebug(MAILCOMMON_LOG) << " name " << name;
    return name;
}

