/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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


#include "parseuserscriptjob.h"
#include "ksieveui/scriptsparsing/parsingutil.h"
#include <kmanagesieve/sievejob.h>

#include <KLocalizedString>


using namespace KSieveUi;
ParseUserScriptJob::ParseUserScriptJob(QObject *parent)
    : QObject(parent),
      mSieveJob(0)
{
}

ParseUserScriptJob::~ParseUserScriptJob()
{
    if ( mSieveJob )
        mSieveJob->kill();
    mSieveJob = 0;
}

void ParseUserScriptJob::scriptUrl(const QUrl &url)
{
    mCurrentUrl = url;
}

void ParseUserScriptJob::start()
{
    if (mCurrentUrl.isEmpty()) {
        Q_EMIT error(i18n("Path is not specified."));
        return;
    }
    if ( mSieveJob )
        mSieveJob->kill();
    mSieveJob = KManageSieve::SieveJob::get( mCurrentUrl );
    connect( mSieveJob, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)) );
}

void ParseUserScriptJob::slotGetResult( KManageSieve::SieveJob *, bool, const QString & script, bool )
{
    mSieveJob = 0;
    if (script.isEmpty()) {
        Q_EMIT error(i18n("Script is empty."));
        return;
    }
    bool result;
    const QStringList lst = parsescript(script, result);
    if (result)
        Q_EMIT success(lst);
    else
        Q_EMIT error(i18n("Script parsing error"));
}

QStringList ParseUserScriptJob::parsescript(const QString &script, bool &result)
{
    QStringList lst;
    const QDomDocument doc = ParsingUtil::parseScript(script, result);
    if (result) {
        lst = extractActiveScript(doc);
    }
    return lst;
}

QStringList ParseUserScriptJob::extractActiveScript(const QDomDocument &doc)
{
    QStringList lstScript;
    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("action")) {
                if (e.hasAttribute(QLatin1String("name"))) {
                    const QString actionName = e.attribute(QLatin1String("name"));
                    if (actionName == QLatin1String("include")) {
                        //Load includes
                        const QString str = loadInclude(e);
                        if (!str.isEmpty()) {
                            if (!lstScript.contains(str)) {
                                lstScript.append(str);
                            }
                        }
                    }
                }
            }
        }
        n = n.nextSibling();
    }
    return lstScript;
}

QString ParseUserScriptJob::loadInclude(const QDomElement &element)
{
    QString scriptName;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                scriptName = e.text();
            }
        }
        node = node.nextSibling();
    }
    return scriptName;
}

