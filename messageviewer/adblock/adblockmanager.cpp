/* ============================================================
* Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>
* based on code from rekonq
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2012 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */


// Self Includes
#include "adblockmanager.h"

#include "settings/globalsettings.h"

#include "webpage.h"

// KDE Includes
#include <KIO/FileCopyJob>
#include <KStandardDirs>
#include <KNotification>

// Qt Includes
#include <QUrl>
#include <QTimer>
#include <QWebElement>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtConcurrentRun>
#include <QFile>
#include <QDateTime>
#include <QWebFrame>

using namespace MessageViewer;
QWeakPointer<AdBlockManager> AdBlockManager::s_adBlockManager;


AdBlockManager *AdBlockManager::self()
{
    if (s_adBlockManager.isNull())
    {
        s_adBlockManager = new AdBlockManager(qApp);
    }
    return s_adBlockManager.data();
}


// ----------------------------------------------------------------------------------------------


AdBlockManager::AdBlockManager(QObject *parent)
    : QObject(parent)
{
    // NOTE: launch this in a second thread so that it does not delay startup
    _settingsLoaded = QtConcurrent::run(this, &AdBlockManager::loadSettings);
}


AdBlockManager::~AdBlockManager()
{
    _whiteList.clear();
    _blackList.clear();
}


bool AdBlockManager::isEnabled()
{
    return GlobalSettings::self()->adBlockEnabled();
}


bool AdBlockManager::isHidingElements()
{
    return GlobalSettings::self()->hideAdsEnabled();
}

void AdBlockManager::reloadConfig()
{
    loadSettings();
}

void AdBlockManager::loadSettings()
{
    KConfig config(QLatin1String("messagevieweradblockrc"));
    // ----------------

    _hostWhiteList.clear();
    _hostBlackList.clear();

    _whiteList.clear();
    _blackList.clear();

    _elementHiding.clear();

    if (!isEnabled())
        return;
    // ----------------------------------------------------------

    QDateTime today = QDateTime::currentDateTime();
    const int days = GlobalSettings::self()->adBlockUpdateInterval();

    const QStringList itemList = config.groupList().filter( QRegExp( QLatin1String("FilterList \\d+") ) );
    Q_FOREACH(const QString &item, itemList) {
        KConfigGroup filtersGroup(&config, item);
        const bool isFilterEnabled = filtersGroup.readEntry(QLatin1String("FilterEnabled"), false);
        if (!isFilterEnabled) {
            continue;
        }
        const QString url = filtersGroup.readEntry(QLatin1String("url"));
        if (url.isEmpty()) {
            continue;
        }
        const QString path = filtersGroup.readEntry(QLatin1String("path"));
        if (path.isEmpty())
            continue;

        const QDateTime lastDateTime = filtersGroup.readEntry(QLatin1String("lastUpdate"), QDateTime());
        if (!lastDateTime.isValid() || today > lastDateTime.addDays(days) || !QFile(path).exists()) {
            updateSubscription(path, url, item);
        } else {
            loadRules(path);
        }
    }

    // load local rules
    const QString localRulesFilePath = KStandardDirs::locateLocal("appdata" , QLatin1String("adblockrules_local"));
    loadRules(localRulesFilePath);
}


void AdBlockManager::loadRules(const QString &rulesFilePath)
{
    QFile ruleFile(rulesFilePath);
    if (!ruleFile.open(QFile::ReadOnly | QFile::Text)) {
        kDebug() << "Unable to open rule file" << rulesFilePath;
        return;
    }

    QTextStream in(&ruleFile);
    while (!in.atEnd())
    {
        QString stringRule = in.readLine();
        loadRuleString(stringRule);
    }
}


void AdBlockManager::loadRuleString(const QString &stringRule)
{
    // ! rules are comments
    if (stringRule.startsWith(QLatin1Char('!')))
        return;

    // [ rules are ABP info
    if (stringRule.startsWith(QLatin1Char('[')))
        return;

    // empty rules are just dangerous..
    // (an empty rule in whitelist allows all, in blacklist blocks all..)
    if (stringRule.isEmpty())
        return;

    // white rules
    if (stringRule.startsWith(QLatin1String("@@")))
    {
        if (_hostWhiteList.tryAddFilter(stringRule))
            return;

        const QString filter = stringRule.mid(2);
        if (filter.isEmpty())
            return;

        AdBlockRule rule(filter);
        _whiteList << rule;
        return;
    }

    // hide (CSS) rules
    if (stringRule.contains(QLatin1String("##")))
    {
        _elementHiding.addRule(stringRule);
        return;
    }

    if (_hostBlackList.tryAddFilter(stringRule))
        return;

    AdBlockRule rule(stringRule);
    _blackList << rule;
}


bool AdBlockManager::blockRequest(const QNetworkRequest &request)
{
    if (!isEnabled())
        return false;

    // we (ad)block just http & https traffic
    if (request.url().scheme() != QLatin1String("http")
            && request.url().scheme() != QLatin1String("https"))
        return false;

    const QStringList whiteRefererList = GlobalSettings::self()->whiteReferer();
    const QString referer = QString::fromLatin1(request.rawHeader("referer"));
    Q_FOREACH(const QString & host, whiteRefererList)
    {
        if (referer.contains(host))
            return false;
    }

    QString urlString = request.url().toString();
    // We compute a lowercase version of the URL so each rule does not
    // have to do it.
    const QString urlStringLowerCase = urlString.toLower();
    const QString host = request.url().host();

    // check white rules before :)
    if (_hostWhiteList.match(host))
    {
        kDebug() << "ADBLOCK: WHITE RULE (@@) Matched by string: " << urlString;
        return false;
    }

    Q_FOREACH(const AdBlockRule & filter, _whiteList)
    {
        if (filter.match(request, urlString, urlStringLowerCase))
        {
            kDebug() << "ADBLOCK: WHITE RULE (@@) Matched by string: " << urlString;
            return false;
        }
    }

    // then check the black ones :(
    if (_hostBlackList.match(host))
    {
        kDebug() << "ADBLOCK: BLACK RULE Matched by string: " << urlString;
        return true;
    }

    Q_FOREACH(const AdBlockRule & filter, _blackList)
    {
        if (filter.match(request, urlString, urlStringLowerCase))
        {
            kDebug() << "ADBLOCK: BLACK RULE Matched by string: " << urlString;
            return true;
        }
    }

    // no match
    return false;
}


void AdBlockManager::updateSubscription(const QString &path, const QString &url, const QString &itemName)
{
    KUrl subUrl = KUrl(url);

    const QString rulesFilePath = path;
    KUrl destUrl = KUrl(rulesFilePath);

    KIO::FileCopyJob* job = KIO::file_copy(subUrl , destUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
    job->metaData().insert(QLatin1String("ssl_no_client_cert"), QLatin1String("TRUE"));
    job->metaData().insert(QLatin1String("ssl_no_ui"), QLatin1String("TRUE"));
    job->metaData().insert(QLatin1String("UseCache"), QLatin1String("false"));
    job->metaData().insert(QLatin1String("cookies"), QLatin1String("none"));
    job->metaData().insert(QLatin1String("no-auth"), QLatin1String("true"));
    job->setProperty("itemname", itemName);

    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotFinished(KJob*)));
}


void AdBlockManager::slotFinished(KJob *job)
{
    if (job->error()) {
        KNotification *notify = new KNotification( QLatin1String("adblock-list-download-failed") );
        notify->setComponentData( KComponentData("messageviewer") );
        notify->setText( i18n("Download new ad-block list was failed." ) );
        notify->sendEvent();
        return;
    }

    KNotification *notify = new KNotification( QLatin1String("adblock-list-download-done") );
    notify->setComponentData( KComponentData("messageviewer") );
    notify->setText( i18n("Download new ad-block list was done." ) );
    notify->sendEvent();
    const QString itemName = job->property("itemname").toString();
    if (!itemName.isEmpty()) {
        KConfig config(QLatin1String("messagevieweradblockrc"));
        if (config.hasGroup(itemName)) {
            KConfigGroup grp = config.group(itemName);
            grp.writeEntry(QLatin1String("lastUpdate"), QDateTime::currentDateTime());
        }
    }

    KIO::FileCopyJob *fJob = qobject_cast<KIO::FileCopyJob *>(job);
    KUrl url = fJob->destUrl();
    url.setProtocol(QString()); // this is needed to load local url well :(
    loadRules(url.url());
}


bool AdBlockManager::subscriptionFileExists(int i)
{
    const QString n = QString::number(i + 1);

    QString rulesFilePath = KStandardDirs::locateLocal("appdata" , QLatin1String("adblockrules_") + n);
    return QFile::exists(rulesFilePath);
}

void AdBlockManager::addCustomRule(const QString &stringRule, bool reloadPage)
{
    // at this point, the settings should be loaded
    _settingsLoaded.waitForFinished();

    // save rule in local filters
    const QString localRulesFilePath = KStandardDirs::locateLocal("appdata" , QLatin1String("adblockrules_local"));

    QFile ruleFile(localRulesFilePath);
    if (!ruleFile.open(QFile::ReadOnly)) {
        kDebug() << "Unable to open rule file" << localRulesFilePath;
        return;
    }

    QTextStream in(&ruleFile);
    while (!in.atEnd()) {
        QString readStringRule = in.readLine();
        if (stringRule == readStringRule) {
            ruleFile.close();
            return;
        }
    }
    ruleFile.close();
    if (!ruleFile.open(QFile::WriteOnly | QFile::Append)) {
        kDebug() << "Unable to open rule file" << localRulesFilePath;
        return;
    }

    QTextStream out(&ruleFile);
    out << stringRule << '\n';

    ruleFile.close();

    // load it
    loadRuleString(stringRule);

    // eventually reload page
    if (reloadPage)
        emit reloadCurrentPage();
}


bool AdBlockManager::isAdblockEnabledForHost(const QString &host)
{
    if (!isEnabled())
        return false;

    return ! _hostWhiteList.match(host);
}


void AdBlockManager::applyHidingRules(QWebFrame *frame)
{
    if (!frame)
        return;

    if (!isEnabled())
        return;

    connect(frame, SIGNAL(loadFinished(bool)), this, SLOT(applyHidingRules(bool)));
}


void AdBlockManager::applyHidingRules(bool ok)
{
    if (!ok)
        return;

    QWebFrame *frame = qobject_cast<QWebFrame *>(sender());
    if (!frame)
        return;
    MessageViewer::WebPage *page = qobject_cast<MessageViewer::WebPage *>(frame->page());
    if (!page)
        return;

    QString mainPageHost = page->loadingUrl().host();
    const QStringList hosts = GlobalSettings::self()->whiteReferer();
    if (hosts.contains(mainPageHost))
        return;

    QWebElement document = frame->documentElement();

    _elementHiding.apply(document, mainPageHost);
}

