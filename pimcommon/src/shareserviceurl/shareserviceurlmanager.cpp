/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "shareserviceurlmanager.h"

#include "pimcommon_debug.h"
#include <KActionMenu>
#include <KLocalizedString>
#include <QMenu>
#include <QDesktopServices>

using namespace PimCommon;

class PimCommon::ShareServiceUrlManagerPrivate
{
public:
    ShareServiceUrlManagerPrivate(ShareServiceUrlManager *qq)
        : mMenu(Q_NULLPTR),
          q(qq)
    {

    }
    QIcon typeToIcon(ShareServiceUrlManager::ServiceType type);
    QString typeToI18n(ShareServiceUrlManager::ServiceType type);
    void initializeMenu();
    KActionMenu *mMenu;
    ShareServiceUrlManager *q;
};

void ShareServiceUrlManagerPrivate::initializeMenu()
{
    mMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("document-share")), i18n("Share On..."), q);
    for (int i = 0; i < ShareServiceUrlManager::ServiceEndType; ++i) {
        const ShareServiceUrlManager::ServiceType type = static_cast<ShareServiceUrlManager::ServiceType>(i);
        QAction *action = new QAction(i18nc("@action:inmenu Share On...", "%1", typeToI18n(type)), q);
        action->setData(QVariant::fromValue(type));
        const QIcon icon = typeToIcon(type);
        if (!icon.isNull()) {
            action->setIcon(icon);
        }
        mMenu->addAction(action);
    }
    q->connect(mMenu->menu(), &QMenu::triggered, q, &ShareServiceUrlManager::slotSelectServiceUrl);
}

QIcon ShareServiceUrlManagerPrivate::typeToIcon(ShareServiceUrlManager::ServiceType type)
{
    QIcon icon;
    switch (type) {
    case ShareServiceUrlManager::Fbook:
        icon = QIcon::fromTheme(QStringLiteral("im-facebook"));
        break;
    case ShareServiceUrlManager::Twitter:
        icon = QIcon::fromTheme(QStringLiteral("im-twitter"));
        break;
    case ShareServiceUrlManager::GooglePlus:
        icon = QIcon::fromTheme(QStringLiteral("im-google"));
        break;
    case ShareServiceUrlManager::MailTo:
        icon = QIcon::fromTheme(QStringLiteral("kmail"));
        break;
    case ShareServiceUrlManager::LinkedIn:
        break;
    case ShareServiceUrlManager::Evernote:
        break;
    case ShareServiceUrlManager::Pocket:
        break;
    case ShareServiceUrlManager::LiveJournal:
        break;
    case ShareServiceUrlManager::ServiceEndType:
        qCWarning(PIMCOMMON_LOG) << "icon for unknow type requested. It's a bug.";
        break;
    }

    return icon;
}

QString ShareServiceUrlManagerPrivate::typeToI18n(ShareServiceUrlManager::ServiceType type)
{
    QString str;
    switch (type) {
    case ShareServiceUrlManager::Fbook:
        str = i18n("Facebook");
        break;
    case ShareServiceUrlManager::Twitter:
        str = i18n("Twitter");
        break;
    case ShareServiceUrlManager::GooglePlus:
        str = i18n("Google Plus");
        break;
    case ShareServiceUrlManager::MailTo:
        str = i18n("Mail");
        break;
    case ShareServiceUrlManager::LinkedIn:
        str = i18n("LinkedIn");
        break;
    case ShareServiceUrlManager::Evernote:
        str = i18n("Evernote");
        break;
    case ShareServiceUrlManager::Pocket:
        str = i18n("Pocket");
        break;
    case ShareServiceUrlManager::LiveJournal:
        str = i18n("LiveJournal");
        break;
    case ShareServiceUrlManager::ServiceEndType:
        break;
    }
    return str;
}

ShareServiceUrlManager::ShareServiceUrlManager(QObject *parent)
    : QObject(parent),
      d(new PimCommon::ShareServiceUrlManagerPrivate(this))
{
    d->initializeMenu();
}

ShareServiceUrlManager::~ShareServiceUrlManager()
{
    delete d;
}

KActionMenu *ShareServiceUrlManager::menu() const
{
    return d->mMenu;
}

void ShareServiceUrlManager::slotSelectServiceUrl(QAction *act)
{
    if (act) {
        const ServiceType type = act->data().value<PimCommon::ShareServiceUrlManager::ServiceType>();
        Q_EMIT serviceUrlSelected(type);
    }
}

QUrl ShareServiceUrlManager::generateServiceUrl(const QString &link, const QString &title, ServiceType type)
{
    QUrl url;
    if (link.isEmpty()) {
        return url;
    }
    switch (type) {
    case Fbook: {
        url.setUrl(QStringLiteral("https://www.facebook.com/sharer.php"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("u"), link);
        urlQuery.addQueryItem(QStringLiteral("t"), title);
        url.setQuery(urlQuery);
        break;
    }
    case Twitter: {
        url.setUrl(QStringLiteral("https://twitter.com/share"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("url"), link);
        urlQuery.addQueryItem(QStringLiteral("text"), title);
        url.setQuery(urlQuery);
        break;
    }
    case GooglePlus: {
        url.setUrl(QStringLiteral("https://plus.google.com/share"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("url"), link);
        url.setQuery(urlQuery);
        break;
    }
    case MailTo: {
        url.setUrl(QStringLiteral("mailto:"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("subject"), title);
        urlQuery.addQueryItem(QStringLiteral("body"), link);
        url.setQuery(urlQuery);
        break;
    }
    case LinkedIn: {
        url.setUrl(QStringLiteral("http://www.linkedin.com/shareArticle"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("mini"), QStringLiteral("true"));
        urlQuery.addQueryItem(QStringLiteral("url"), link);
        urlQuery.addQueryItem(QStringLiteral("title"), title);
        url.setQuery(urlQuery);
        break;
    }
    case Evernote: {
        url.setUrl(QStringLiteral("https://www.evernote.com/clip.action"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("url"), link);
        urlQuery.addQueryItem(QStringLiteral("title"), title);
        url.setQuery(urlQuery);
        break;
    }
    case Pocket: {
        url.setUrl(QStringLiteral("https://getpocket.com/save"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("url"), link);
        urlQuery.addQueryItem(QStringLiteral("title"), title);
        url.setQuery(urlQuery);
        break;
    }
    case LiveJournal: {
        url.setUrl(QStringLiteral("http://www.livejournal.com/update.bml"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("event"), link);
        urlQuery.addQueryItem(QStringLiteral("subject"), title);
        url.setQuery(urlQuery);
        break;
    }
    case ServiceEndType:
        break;
    }
    return url;
}

void ShareServiceUrlManager::openUrl(const QUrl &url)
{
    if (url.isValid()) {
        QDesktopServices::openUrl(url);
    }
}
