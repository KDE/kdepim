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

#include "webshortcutsmenumanager.h"
#include <QDesktopServices>
#include <KToolInvocation>
#include <KUriFilterData>
#include <KStringHandler>
#include <KLocalizedString>
#include <QMenu>
#include <QIcon>

using namespace PimCommon;

class PimCommon::WebShortcutsMenuManagerPrivate
{
public:
    WebShortcutsMenuManagerPrivate()
    {

    }

    QString mSelectedText;
};

WebShortcutsMenuManager::WebShortcutsMenuManager(QObject *parent)
    : QObject(parent),
      d(new PimCommon::WebShortcutsMenuManagerPrivate)
{

}

WebShortcutsMenuManager::~WebShortcutsMenuManager()
{
    delete d;
}

QString WebShortcutsMenuManager::selectedText() const
{
    return d->mSelectedText;
}

void WebShortcutsMenuManager::setSelectedText(const QString &selectedText)
{
    d->mSelectedText = selectedText;
}

void WebShortcutsMenuManager::slotConfigureWebShortcuts()
{
    KToolInvocation::kdeinitExec(QStringLiteral("kcmshell5"), QStringList() << QStringLiteral("webshortcuts"));
}

void WebShortcutsMenuManager::addWebShortcutsToMenu(QMenu *menu)
{
    if (d->mSelectedText.isEmpty()) {
        return;
    }

    QString searchText = d->mSelectedText;
    searchText = searchText.replace(QLatin1Char('\n'), QLatin1Char(' ')).replace(QLatin1Char('\r'), QLatin1Char(' ')).simplified();

    if (searchText.isEmpty()) {
        return;
    }

    KUriFilterData filterData(searchText);

    filterData.setSearchFilteringOptions(KUriFilterData::RetrievePreferredSearchProvidersOnly);

    if (KUriFilter::self()->filterSearchUri(filterData, KUriFilter::NormalTextFilter)) {
        const QStringList searchProviders = filterData.preferredSearchProviders();

        if (!searchProviders.isEmpty()) {
            QMenu *webShortcutsMenu = new QMenu(menu);
            webShortcutsMenu->setIcon(QIcon::fromTheme(QStringLiteral("preferences-web-browser-shortcuts")));

            const QString squeezedText = KStringHandler::rsqueeze(searchText, 21);
            webShortcutsMenu->setTitle(i18n("Search for '%1' with", squeezedText));

            QAction *action = Q_NULLPTR;

            Q_FOREACH (const QString &searchProvider, searchProviders) {
                action = new QAction(i18nc("@action:inmenu Search for <text> with", "%1", searchProvider), webShortcutsMenu);
                action->setIcon(QIcon::fromTheme(filterData.iconNameForPreferredSearchProvider(searchProvider)));
                action->setData(filterData.queryForPreferredSearchProvider(searchProvider));
                connect(action, &QAction::triggered, this, &WebShortcutsMenuManager::slotHandleWebShortcutAction);
                webShortcutsMenu->addAction(action);
            }

            webShortcutsMenu->addSeparator();

            action = new QAction(i18n("Configure Web Shortcuts..."), webShortcutsMenu);
            action->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
            connect(action, &QAction::triggered, this, &WebShortcutsMenuManager::slotConfigureWebShortcuts);
            webShortcutsMenu->addAction(action);

            menu->addMenu(webShortcutsMenu);
        }
    }
}

void WebShortcutsMenuManager::slotHandleWebShortcutAction()
{
    QAction *action = qobject_cast<QAction *>(sender());

    if (action) {
        KUriFilterData filterData(action->data().toString());
        if (KUriFilter::self()->filterSearchUri(filterData, KUriFilter::WebShortcutFilter)) {
            QDesktopServices::openUrl(filterData.uri());
        }
    }
}
