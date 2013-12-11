/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "filterimporterbalsa_p.h"
#include "filtermanager.h"
#include "mailfilter.h"

#include <KConfig>
#include <KConfigGroup>

#include <KDebug>

#include <QFile>
#include <QDir>

using namespace MailCommon;

FilterImporterBalsa::FilterImporterBalsa( QFile *file )
    :FilterImporterAbstract()
{
    KConfig config( file->fileName() );
    const QStringList filterList = config.groupList().filter( QRegExp( QLatin1String("filter-\\d+") ) );
    Q_FOREACH(const QString &filter, filterList) {
        KConfigGroup grp = config.group(filter);
        addFilter(grp);
    }
}

FilterImporterBalsa::~FilterImporterBalsa()
{
}

QString FilterImporterBalsa::defaultFiltersSettingsPath()
{
    return QString::fromLatin1( "%1/.balsa/config" ).arg( QDir::homePath() );
}


void FilterImporterBalsa::addFilter(const KConfigGroup &grp)
{
    MailCommon::MailFilter *filter = new MailCommon::MailFilter();
    const QString name = grp.readEntry(QLatin1String("Name"));
    filter->pattern()->setName( name );
    filter->setToolbarName( name );

    //TODO not implemented in kmail.
    const QString popupText = grp.readEntry(QLatin1String("Popup-text"));


    const QString sound = grp.readEntry(QLatin1String("Sound"));
    if (!sound.isEmpty()) {
        const QString actionName = QLatin1String( "play sound" );
        createFilterAction( filter, actionName, sound );
    }

    const int actionType = grp.readEntry(QLatin1String("Action-type"),-1);
    const QString actionStr = grp.readEntry(QLatin1String("Action-string"));
    parseAction(actionType,actionStr,filter);

    const QString condition = grp.readEntry(QLatin1String("Condition"));
    parseCondition(condition,filter);

    appendFilter(filter);
}

void FilterImporterBalsa::parseCondition(const QString& condition,MailCommon::MailFilter *filter )
{
    QStringList conditionList;
    if (condition.startsWith(QLatin1String("OR "))) {
        conditionList = condition.split(QLatin1String("OR"));
        filter->pattern()->setOp( SearchPattern::OpOr );
    } else if (condition.startsWith(QLatin1String("AND "))) {
        conditionList = condition.split(QLatin1String("AND"));
        filter->pattern()->setOp( SearchPattern::OpAnd );
    } else {
        //no multi condition
        conditionList<< condition;
    }
    Q_FOREACH(QString cond, conditionList) {
        cond = cond.trimmed();
        if (cond.startsWith(QLatin1String("NOT"))) {
            cond = cond.right(cond.length()-3);
            cond = cond.trimmed();
        }
        qDebug()<<" cond"<<cond;

        //Date between
        QByteArray fieldName;
        if (cond.startsWith(QLatin1String("DATE"))) {
            fieldName = "<date>";
            cond = cond.right(cond.length()-4);
            cond = cond.trimmed();
            QStringList splitDate = cond.split(QLatin1Char(' '));
            qDebug()<<" splitDate "<<splitDate;
        } else if (cond.startsWith(QLatin1String("FLAG"))) {
            qDebug()<<" FLAG :";
        } else if (cond.startsWith(QLatin1String("STRING"))) {
            qDebug()<<" STRING";
        } else {
            qDebug()<<" condition not implemented :"<<cond;
        }

        //SearchRule::Ptr rule = SearchRule::createInstance( fieldName, functionName, line );
        //filter->pattern()->append( rule );
    }
}

void FilterImporterBalsa::parseAction(int actionType, const QString& action,MailCommon::MailFilter *filter)
{
    QString actionName;
    QString actionStr(action);
    switch(actionType) {
    case 0:
        break;
    case 1:
        //Copy
        actionName = QLatin1String( "copy" );
        break;
    case 2:
        //Move
        actionName = QLatin1String( "transfer" );
        break;
    case 3:
        //Print
        //Not implemented in kmail
        break;
    case 4:
        //Execute
        actionName = QLatin1String( "execute" );
        break;
    case 5:
        //Move to trash
        actionName = QLatin1String( "transfer" );
        //Special !
        break;
    case 6:
        //Put color
        break;
    default:
        qDebug()<<" unknown parse action type "<<actionType;
        break;
    }
    if (!actionName.isEmpty()) {
        //TODO adapt actionStr
        createFilterAction( filter, actionName, actionStr );
    }
}
