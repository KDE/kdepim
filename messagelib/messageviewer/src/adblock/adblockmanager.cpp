/* ============================================================
* Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>
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
#include "messageviewer_debug.h"
#include "settings/messageviewersettings.h"
#include "adblock/adblockutil.h"

#include "webpage.h"

// KDE Includes
#include <KIO/FileCopyJob>
#include <QStandardPaths>
#include <KNotification>
#include <KLocalizedString>
// Qt Includes
#include <QUrl>
#include <QTimer>
#include <QWebElement>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtConcurrent/QtConcurrentRun>
#include <QFile>
#include <QDateTime>
#include <QWebFrame>
#include <QRegularExpression>

using namespace MessageViewer;
QWeakPointer<AdBlockManager> AdBlockManager::s_adBlockManager;

AdBlockManager *AdBlockManager::self()
{
    if (s_adBlockManager.isNull()) {
        s_adBlockManager = new AdBlockManager(qApp);
    }
    return s_adBlockManager.data();
}

// ----------------------------------------------------------------------------------------------

AdBlockManager::AdBlockManager(QObject *parent)
    : QObject(parent)
{
    loadSettings();
}

AdBlockManager::~AdBlockManager()
{
    _whiteList.clear();
    _blackList.clear();
}

bool AdBlockManager::isEnabled()
{
    return MessageViewer::MessageViewerSettings::self()->adBlockEnabled();
}

bool AdBlockManager::isHidingElements()
{
    return MessageViewer::MessageViewerSettings::self()->hideAdsEnabled();
}

void AdBlockManager::reloadConfig()
{
    loadSettings();
}

void AdBlockManager::loadSettings()
{
    KConfig config(QStringLiteral("messagevieweradblockrc"));
    // ----------------

    _hostWhiteList.clear();
    _hostBlackList.clear();

    _whiteList.clear();
    _blackList.clear();

    _elementHiding.clear();

    if (!isEnabled()) {
        return;
    }
    // ----------------------------------------------------------

    QDateTime today = QDateTime::currentDateTime();
    const int days = MessageViewer::MessageViewerSettings::self()->adBlockUpdateInterval();

    const QStringList itemList = config.groupList().filter(QRegularExpression(QStringLiteral("FilterList \\d+")));
    Q_FOREACH (const QString &item, itemList) {
        KConfigGroup filtersGroup(&config, item);
        const bool isFilterEnabled = filtersGroup.readEntry(QStringLiteral("FilterEnabled"), false);
        if (!isFilterEnabled) {
            continue;
        }
        const QString url = filtersGroup.readEntry(QStringLiteral("url"));
        if (url.isEmpty()) {
            continue;
        }
        const QString path = filtersGroup.readEntry(QStringLiteral("path"));
        if (path.isEmpty()) {
            continue;
        }

        const QDateTime lastDateTime = filtersGroup.readEntry(QStringLiteral("lastUpdate"), QDateTime());
        if (!lastDateTime.isValid() || today > lastDateTime.addDays(days) || !QFile(path).exists()) {
            updateSubscription(path, url, item);
        } else {
            loadRules(path);
        }
    }

    // load local rules
    const QString localRulesFilePath = MessageViewer::AdBlockUtil::localFilterPath();
    loadRules(localRulesFilePath);
}

void AdBlockManager::loadRules(const QString &rulesFilePath)
{
    QFile ruleFile(rulesFilePath);
    if (!ruleFile.open(QFile::ReadOnly | QFile::Text)) {
        qCDebug(MESSAGEVIEWER_LOG) << "Unable to open rule file" << rulesFilePath;
        return;
    }

    QTextStream in(&ruleFile);
    while (!in.atEnd()) {
        QString stringRule = in.readLine();
        loadRuleString(stringRule);
    }
}

void AdBlockManager::loadRuleString(const QString &stringRule)
{
    // ! rules are comments
    if (stringRule.startsWith(QLatin1Char('!'))) {
        return;
    }

    // [ rules are ABP info
    if (stringRule.startsWith(QLatin1Char('['))) {
        return;
    }

    // empty rules are just dangerous..
    // (an empty rule in whitelist allows all, in blacklist blocks all..)
    if (stringRule.isEmpty()) {
        return;
    }

    // white rules
    if (stringRule.startsWith(QStringLiteral("@@"))) {
        if (_hostWhiteList.tryAddFilter(stringRule)) {
            return;
        }

        const QString filter = stringRule.mid(2);
        if (filter.isEmpty()) {
            return;
        }

        AdBlockRule rule(filter);
        _whiteList << rule;
        return;
    }

    // hide (CSS) rules
    if (stringRule.contains(QStringLiteral("##"))) {
        _elementHiding.addRule(stringRule);
        return;
    }

    if (_hostBlackList.tryAddFilter(stringRule)) {
        return;
    }

    AdBlockRule rule(stringRule);
    _blackList << rule;
}

bool AdBlockManager::blockRequest(const QNetworkRequest &request)
{
    if (!isEnabled()) {
        return false;
    }

    // we (ad)block just http & https traffic
    if (request.url().scheme() != QLatin1String("http")
            && request.url().scheme() != QLatin1String("https")) {
        return false;
    }

    const QStringList whiteRefererList = MessageViewer::MessageViewerSettings::self()->whiteReferer();
    const QString referer = QString::fromLatin1(request.rawHeader("referer"));
    Q_FOREACH (const QString &host, whiteRefererList) {
        if (referer.contains(host)) {
            return false;
        }
    }

    QString urlString = request.url().toString();
    // We compute a lowercase version of the URL so each rule does not
    // have to do it.
    const QString urlStringLowerCase = urlString.toLower();
    const QString host = request.url().host();

    // check white rules before :)
    if (_hostWhiteList.match(host)) {
        qCDebug(MESSAGEVIEWER_LOG) << "ADBLOCK: WHITE RULE (@@) Matched by string: " << urlString;
        return false;
    }

    Q_FOREACH (const AdBlockRule &filter, _whiteList) {
        if (filter.match(request, urlString, urlStringLowerCase)) {
            qCDebug(MESSAGEVIEWER_LOG) << "ADBLOCK: WHITE RULE (@@) Matched by string: " << urlString;
            return false;
        }
    }

    // then check the black ones :(
    if (_hostBlackList.match(host)) {
        qCDebug(MESSAGEVIEWER_LOG) << "ADBLOCK: BLACK RULE Matched by string: " << urlString;
        return true;
    }

    Q_FOREACH (const AdBlockRule &filter, _blackList) {
        if (filter.match(request, urlString, urlStringLowerCase)) {
            qCDebug(MESSAGEVIEWER_LOG) << "ADBLOCK: BLACK RULE Matched by string: " << urlString;
            return true;
        }
    }

    // no match
    return false;
}

void AdBlockManager::updateSubscription(const QString &path, const QString &url, const QString &itemName)
{
    QUrl subUrl = QUrl(url);

    const QString rulesFilePath = path;
    QUrl destUrl = QUrl::fromLocalFile(rulesFilePath);

    KIO::FileCopyJob *job = KIO::file_copy(subUrl, destUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
    KIO::MetaData metadata = job->metaData();
    metadata.insert(QStringLiteral("ssl_no_client_cert"), QStringLiteral("TRUE"));
    metadata.insert(QStringLiteral("ssl_no_ui"), QStringLiteral("TRUE"));
    metadata.insert(QStringLiteral("UseCache"), QStringLiteral("false"));
    metadata.insert(QStringLiteral("cookies"), QStringLiteral("none"));
    metadata.insert(QStringLiteral("no-auth"), QStringLiteral("true"));
    job->setMetaData(metadata);
    job->setProperty("itemname", itemName);

    connect(job, &KIO::FileCopyJob::finished, this, &AdBlockManager::slotFinished);
}

void AdBlockManager::slotFinished(KJob *job)
{
    if (job->error()) {
        KNotification *notify = new KNotification(QStringLiteral("adblock-list-download-failed"));
        notify->setComponentName(QStringLiteral("messageviewer"));
        notify->setText(i18n("Download new ad-block list was failed."));
        notify->sendEvent();
        return;
    }

    KNotification *notify = new KNotification(QStringLiteral("adblock-list-download-done"));
    notify->setComponentName(QStringLiteral("messageviewer"));
    notify->setText(i18n("Download new ad-block list was done."));
    notify->sendEvent();
    const QString itemName = job->property("itemname").toString();
    if (!itemName.isEmpty()) {
        KConfig config(QStringLiteral("messagevieweradblockrc"));
        if (config.hasGroup(itemName)) {
            KConfigGroup grp = config.group(itemName);
            grp.writeEntry(QStringLiteral("lastUpdate"), QDateTime::currentDateTime());
        }
    }

    KIO::FileCopyJob *fJob = qobject_cast<KIO::FileCopyJob *>(job);
    QUrl url = fJob->destUrl();
    url.setScheme(QString()); // this is needed to load local url well :(
    loadRules(url.url());
}

bool AdBlockManager::subscriptionFileExists(int i)
{
    const QString n = QString::number(i + 1);

    const QString rulesFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kmail2/adblockrules_") + n;
    return QFile::exists(rulesFilePath);
}

void AdBlockManager::addCustomRule(const QString &stringRule, bool reloadPage)
{
    // save rule in local filters
    const QString localRulesFilePath = MessageViewer::AdBlockUtil::localFilterPath();

    QFile ruleFile(localRulesFilePath);
    if (!ruleFile.open(QFile::ReadOnly)) {
        qCDebug(MESSAGEVIEWER_LOG) << "Unable to open rule file" << localRulesFilePath;
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
        qCDebug(MESSAGEVIEWER_LOG) << "Unable to open rule file" << localRulesFilePath;
        return;
    }

    QTextStream out(&ruleFile);
    out << stringRule << '\n';

    ruleFile.close();

    // load it
    loadRuleString(stringRule);

    // eventually reload page
    if (reloadPage) {
        Q_EMIT reloadCurrentPage();
    }
}

bool AdBlockManager::isAdblockEnabledForHost(const QString &host)
{
    if (!isEnabled()) {
        return false;
    }

    return ! _hostWhiteList.match(host);
}

void AdBlockManager::applyHidingRules(QWebFrame *frame)
{
    if (!frame) {
        return;
    }

    if (!isEnabled()) {
        return;
    }

    connect(frame, SIGNAL(loadFinished(bool)), this, SLOT(applyHidingRules(bool)));
}

void AdBlockManager::applyHidingRules(bool ok)
{
    if (!ok) {
        return;
    }

    QWebFrame *frame = qobject_cast<QWebFrame *>(sender());
    if (!frame) {
        return;
    }
    MessageViewer::WebPage *page = qobject_cast<MessageViewer::WebPage *>(frame->page());
    if (!page) {
        return;
    }

    QString mainPageHost = page->loadingUrl().host();
    const QStringList hosts = MessageViewer::MessageViewerSettings::self()->whiteReferer();
    if (hosts.contains(mainPageHost)) {
        return;
    }

    QWebElement document = frame->documentElement();

    _elementHiding.apply(document, mainPageHost);
}

