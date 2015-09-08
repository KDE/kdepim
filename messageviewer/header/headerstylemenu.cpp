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
#include "headerstylemenu.h"
#include "headerstyleplugin.h"
#include "headerstylepluginmanager.h"
#include <KActionMenu>
#include <KActionCollection>
#include <KLocalizedString>
#include <KToggleAction>

using namespace MessageViewer;

class MessageViewer::HeaderStyleMenuPrivate
{
public:
    HeaderStyleMenuPrivate(HeaderStyleMenu *qq)
        : group(Q_NULLPTR),
          headerMenu(Q_NULLPTR),
          q(qq)
    {

    }
    void initialize(KActionCollection *ac);
    void addHelpTextAction(QAction *act, const QString &text);
    QList<MessageViewer::HeaderStyleInterface *> lstInterface;
    QActionGroup *group;
    KActionMenu *headerMenu;
    HeaderStyleMenu *q;
};

void HeaderStyleMenuPrivate::addHelpTextAction(QAction *act, const QString &text)
{
    act->setStatusTip(text);
    act->setToolTip(text);
    if (act->whatsThis().isEmpty()) {
        act->setWhatsThis(text);
    }
}

void HeaderStyleMenuPrivate::initialize(KActionCollection *ac)
{
    headerMenu = new KActionMenu(i18nc("View->", "&Headers"), q);
    ac->addAction(QStringLiteral("view_headers"), headerMenu);
    addHelpTextAction(headerMenu, i18n("Choose display style of message headers"));
    group = new QActionGroup(q);

    const QVector<MessageViewer::HeaderStylePlugin *>  lstPlugin = MessageViewer::HeaderStylePluginManager::self()->pluginsList();
    Q_FOREACH(MessageViewer::HeaderStylePlugin *plugin, lstPlugin) {
        MessageViewer::HeaderStyleInterface *interface = plugin->createView(ac, q);
        lstInterface.append(interface);
        Q_FOREACH(KToggleAction *taction, interface->action()) {
            headerMenu->addAction(taction);
            group->addAction(taction);
        }
    }
}

HeaderStyleMenu::HeaderStyleMenu(KActionCollection *ac, QObject *parent)
    : QObject(parent),
      d(new MessageViewer::HeaderStyleMenuPrivate(this))
{
    d->initialize(ac);
}

HeaderStyleMenu::~HeaderStyleMenu()
{
    delete d;
}


KActionMenu *MessageViewer::HeaderStyleMenu::menu() const
{
    return d->headerMenu;
}

