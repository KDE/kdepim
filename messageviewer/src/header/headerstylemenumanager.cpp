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

#include "headerstyleinterface.h"
#include "headerstylemenumanager.h"
#include "headerstyleplugin.h"
#include "headerstylepluginmanager.h"
#include "header/headerstyle.h"
#include "header/headerstrategy.h"
#include "messageviewer_debug.h"
#include <KActionMenu>
#include <KActionCollection>
#include <KLocalizedString>
#include <KToggleAction>
using namespace MessageViewer;

class MessageViewer::HeaderStyleMenuManagerPrivate
{
public:
    HeaderStyleMenuManagerPrivate(HeaderStyleMenuManager *qq)
        : group(Q_NULLPTR),
          headerMenu(Q_NULLPTR),
          q(qq)
    {
    }
    void initialize(KActionCollection *ac);
    void addHelpTextAction(QAction *act, const QString &text);
    void setPluginName(const QString &pluginName);
    QHash<QString, MessageViewer::HeaderStyleInterface *> lstInterface;
    QActionGroup *group;
    KActionMenu *headerMenu;
    HeaderStyleMenuManager *q;
};

void HeaderStyleMenuManagerPrivate::addHelpTextAction(QAction *act, const QString &text)
{
    act->setStatusTip(text);
    act->setToolTip(text);
    if (act->whatsThis().isEmpty()) {
        act->setWhatsThis(text);
    }
}

void HeaderStyleMenuManagerPrivate::setPluginName(const QString &pluginName)
{
    MessageViewer::HeaderStyleInterface *interface = lstInterface.value(pluginName);
    if (interface) {
        interface->activateAction();
    } else {
        if (lstInterface.isEmpty()) {
            qCCritical(MESSAGEVIEWER_LOG) << "No plugin found !";
        } else {
            interface = lstInterface.cbegin().value();
            interface->activateAction();
        }
    }
}

void HeaderStyleMenuManagerPrivate::initialize(KActionCollection *ac)
{
    headerMenu = new KActionMenu(i18nc("View->", "&Headers"), q);
    ac->addAction(QStringLiteral("view_headers"), headerMenu);
    addHelpTextAction(headerMenu, i18n("Choose display style of message headers"));
    group = new QActionGroup(q);

    const QVector<MessageViewer::HeaderStylePlugin *>  lstPlugin = MessageViewer::HeaderStylePluginManager::self()->pluginsList();
    Q_FOREACH (MessageViewer::HeaderStylePlugin *plugin, lstPlugin) {
        MessageViewer::HeaderStyleInterface *interface = plugin->createView(headerMenu, group, ac, q);
        lstInterface.insert(plugin->name(), interface);
        q->connect(interface, &HeaderStyleInterface::styleChanged, q, &HeaderStyleMenuManager::styleChanged);
        q->connect(interface, &HeaderStyleInterface::styleUpdated, q, &HeaderStyleMenuManager::styleUpdated);
    }
}

HeaderStyleMenuManager::HeaderStyleMenuManager(KActionCollection *ac, QObject *parent)
    : QObject(parent),
      d(new MessageViewer::HeaderStyleMenuManagerPrivate(this))
{
    d->initialize(ac);
}

HeaderStyleMenuManager::~HeaderStyleMenuManager()
{
    delete d;
}

KActionMenu *HeaderStyleMenuManager::menu() const
{
    return d->headerMenu;
}

void MessageViewer::HeaderStyleMenuManager::setPluginName(const QString &pluginName)
{
    d->setPluginName(pluginName);
}
