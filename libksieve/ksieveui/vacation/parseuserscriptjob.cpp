/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#include <KLocale>

using namespace KSieveUi;
ParseUserScriptJob::ParseUserScriptJob(QObject *parent)
    : QObject(parent)
{
}

ParseUserScriptJob::~ParseUserScriptJob()
{

}

void ParseUserScriptJob::scriptUrl(const KUrl &url)
{
    mCurrentUrl = url;
}

void ParseUserScriptJob::start()
{
    if (mCurrentUrl.isEmpty()) {
        Q_EMIT error(i18n("Path not specify."));
        return;
    }
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::get( mCurrentUrl );
    connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)) );
}

void ParseUserScriptJob::slotGetResult( KManageSieve::SieveJob *, bool, const QString & script, bool )
{
    if (script.isEmpty()) {
        Q_EMIT error(i18n("Script is empty."));
        return;
    }
    bool result;
    QDomDocument doc = ParsingUtil::parseScript(script, result);
    if (!result) {
        Q_EMIT error(i18n("Script parsing error"));
        return;
    }
    QStringList lstScript;
    //TODO parsing.
    Q_EMIT success(lstScript);
}


#include "parseuserscriptjob.moc"
