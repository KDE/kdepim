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

#include "filterimporterclawsmail_p.h"
#include "filtermanager.h"
#include "mailfilter.h"

#include <KConfig>
#include <KConfigGroup>

#include <KDebug>

#include <QFile>

using namespace MailCommon;

FilterImporterClawsMails::FilterImporterClawsMails( QFile *file )
    :FilterImporterAbstract()
{
    QTextStream stream(file);
    MailFilter *filter = 0;
    while ( !stream.atEnd() ) {
        QString line = stream.readLine();
        kDebug() << " line :" << line << " filter " << filter;

        if (line.isEmpty()) {
            //Nothing
        } else if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            //TODO
        } else {
            appendFilter(filter);
            filter = parseLine( line );
        }
    }
    appendFilter(filter);
}

FilterImporterClawsMails::FilterImporterClawsMails(bool interactive)
    :FilterImporterAbstract()
{
    mInteractive = interactive;
}

FilterImporterClawsMails::~FilterImporterClawsMails()
{
}

QString FilterImporterClawsMails::defaultFiltersSettingsPath()
{
    return QString::fromLatin1( "%1/.claws-mail/matcherrc" ).arg( QDir::homePath() );
}

MailFilter * FilterImporterClawsMails::parseLine(const QString &line)
{
    MailFilter *filter = new MailFilter();
    QString tmp = line;
    //Enabled ?
    if (tmp.startsWith(QLatin1String("enabled"))) {
        filter->setEnabled(true);
        tmp.remove(QLatin1String("enabled "));
    }

    //Filter name
    if (tmp.startsWith(QLatin1String("rulename")) ) {
        tmp.remove(QLatin1String("rulename "));
        int pos;
        const QString name = extractString(tmp, pos);
        filter->pattern()->setName( name );
        filter->setToolbarName( name );

        tmp = tmp.mid(pos+2); //remove "\" "
        qDebug()<<" new tmp"<<tmp;
    }

    tmp = extractConditions( tmp, filter);

    tmp = extractActions(tmp, filter);
    //TODO
    return filter;
}

QString FilterImporterClawsMails::extractActions( const QString &line,MailFilter *filter)
{
    return line;
}

QString FilterImporterClawsMails::extractConditions( const QString &line,MailFilter *filter)
{
    QByteArray fieldName;
    //Action
    if (line.startsWith(QLatin1String("subject"))) {
        fieldName = "subject";
    } else if (line.startsWith(QLatin1String("age_lower"))) {

    }
    filter->pattern()->setOp( SearchPattern::OpAnd );
    //TODO
    return QString();
}

QString FilterImporterClawsMails::extractString( const QString & tmp, int & pos)
{
    QString name;
    QChar previousChar;
    int i = 0;
    for (; i <tmp.length(); ++i) {
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
    qDebug()<<" name "<<name;
    return name;
}

