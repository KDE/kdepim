/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
ParseUserScriptJob::ParseUserScriptJob(const QUrl &url, QObject *parent)
    : QObject(parent)
    , mCurrentUrl(url)
    , mSieveJob(Q_NULLPTR)
{
}

ParseUserScriptJob::~ParseUserScriptJob()
{
    kill();
}

void ParseUserScriptJob::kill()
{
    if (mSieveJob) {
        mSieveJob->kill();
    }
    mSieveJob = Q_NULLPTR;
}

QUrl ParseUserScriptJob::scriptUrl() const
{
    return mCurrentUrl;
}

void ParseUserScriptJob::start()
{
    if (mCurrentUrl.isEmpty()) {
        emitError(i18n("Path is not specified."));
        return;
    }
    if (mSieveJob) {
        mSieveJob->kill();
    }
    mActiveScripts = QStringList();
    mError = QString();
    mSieveJob = KManageSieve::SieveJob::get(mCurrentUrl);
    connect(mSieveJob, &KManageSieve::SieveJob::result, this, &ParseUserScriptJob::slotGetResult);
}

void ParseUserScriptJob::slotGetResult(KManageSieve::SieveJob *, bool, const QString &script, bool)
{
    mSieveJob = Q_NULLPTR;
    if (script.isEmpty()) {
        emitError(i18n("Script is empty."));
        return;
    }
    bool result;
    const QStringList lst = parsescript(script, result);
    if (result) {
        emitSuccess(lst);
    } else {
        emitError(i18n("Script parsing error"));
    }
}

void ParseUserScriptJob::emitError(const QString &msgError)
{
    mError = msgError;
    Q_EMIT finished(this);
}

void ParseUserScriptJob::emitSuccess(const QStringList &activeScriptList)
{
    mActiveScripts = activeScriptList;
    Q_EMIT finished(this);
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

QStringList ParseUserScriptJob::activeScriptList() const
{
    return mActiveScripts;
}

QString ParseUserScriptJob::error() const
{
    return mError;
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
                if (e.hasAttribute(QStringLiteral("name"))) {
                    const QString actionName = e.attribute(QStringLiteral("name"));
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

