/*
 * Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@ubiz.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "customtemplatesmenu.h"
#include "customtemplates.h"
#include "customtemplates_kfg.h"
#include "globalsettings_templateparser.h"

#include <KActionCollection>
#include <KActionMenu>
#include <QIcon>
#include <KLocalizedString>
#include <QMenu>

#include <QSignalMapper>

using namespace TemplateParser;
class TemplateParser::CustomTemplatesMenuPrivate
{
public:
    CustomTemplatesMenuPrivate()
        : mOwnerActionCollection(Q_NULLPTR),
          mCustomReplyActionMenu(Q_NULLPTR),
          mCustomReplyAllActionMenu(Q_NULLPTR),
          mCustomForwardActionMenu(Q_NULLPTR),
          mCustomReplyMapper(Q_NULLPTR),
          mCustomReplyAllMapper(Q_NULLPTR),
          mCustomForwardMapper(Q_NULLPTR)
    {

    }
    ~CustomTemplatesMenuPrivate()
    {
        delete mCustomReplyActionMenu;
        delete mCustomReplyAllActionMenu;
        delete mCustomForwardActionMenu;

        delete mCustomReplyMapper;
        delete mCustomReplyAllMapper;
        delete mCustomForwardMapper;
    }
    KActionCollection *mOwnerActionCollection;

    QStringList mCustomTemplates;
    QList<QAction *> mCustomTemplateActions;

    // Custom template actions menu
    KActionMenu *mCustomReplyActionMenu;
    KActionMenu *mCustomReplyAllActionMenu;
    KActionMenu *mCustomForwardActionMenu;

    // Signal mappers for custom template actions
    QSignalMapper *mCustomReplyMapper;
    QSignalMapper *mCustomReplyAllMapper;
    QSignalMapper *mCustomForwardMapper;
};

CustomTemplatesMenu::CustomTemplatesMenu(QWidget *owner, KActionCollection *ac)
    : d(new TemplateParser::CustomTemplatesMenuPrivate)
{
    d->mOwnerActionCollection = ac;

    d->mCustomForwardActionMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("mail-forward-custom")),
            i18n("With Custom Template"), owner);
    d->mOwnerActionCollection->addAction(QStringLiteral("custom_forward"), d->mCustomForwardActionMenu);

    d->mCustomReplyActionMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("mail-reply-custom")),
            i18n("Reply With Custom Template"), owner);
    d->mOwnerActionCollection->addAction(QStringLiteral("custom_reply"), d->mCustomReplyActionMenu);

    d->mCustomReplyAllActionMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("mail-reply-all-custom")),
            i18n("Reply to All With Custom Template"), owner);
    d->mOwnerActionCollection->addAction(QStringLiteral("custom_reply_all"), d->mCustomReplyAllActionMenu);

    d->mCustomForwardMapper = new QSignalMapper(this);
    connect(d->mCustomForwardMapper, SIGNAL(mapped(int)),
            this, SLOT(slotForwardSelected(int)));

    d->mCustomReplyMapper = new QSignalMapper(this);
    connect(d->mCustomReplyMapper, SIGNAL(mapped(int)),
            this, SLOT(slotReplySelected(int)));

    d->mCustomReplyAllMapper = new QSignalMapper(this);
    connect(d->mCustomReplyAllMapper, SIGNAL(mapped(int)),
            this, SLOT(slotReplyAllSelected(int)));

    update();
}

CustomTemplatesMenu::~CustomTemplatesMenu()
{
    clear();
    delete d;
}

KActionMenu *CustomTemplatesMenu::replyActionMenu() const
{
    return d->mCustomReplyActionMenu;
}

KActionMenu *CustomTemplatesMenu::replyAllActionMenu() const
{
    return d->mCustomReplyAllActionMenu;
}

KActionMenu *CustomTemplatesMenu::forwardActionMenu() const
{
    return d->mCustomForwardActionMenu;
}

void CustomTemplatesMenu::clear()
{
    QListIterator<QAction *> ait(d->mCustomTemplateActions);
    while (ait.hasNext()) {
        QAction *action = ait.next();
        d->mCustomReplyMapper->removeMappings(action);
        d->mCustomReplyAllMapper->removeMappings(action);
        d->mCustomForwardMapper->removeMappings(action);
    }
    qDeleteAll(d->mCustomTemplateActions);
    d->mCustomTemplateActions.clear();

    d->mCustomReplyActionMenu->menu()->clear();
    d->mCustomReplyAllActionMenu->menu()->clear();
    d->mCustomForwardActionMenu->menu()->clear();
    d->mCustomTemplates.clear();
}

void CustomTemplatesMenu::update()
{
    clear();

    const QStringList list = TemplateParserSettings::self()->customTemplates();
    QStringList::const_iterator it = list.constBegin();
    QStringList::const_iterator end = list.constEnd();
    int idx = 0;
    int replyc = 0;
    int replyallc = 0;
    int forwardc = 0;
    for (; it != end; ++it) {
        CTemplates t(*it);
        d->mCustomTemplates.append(*it);
        QString nameAction(*it);
        nameAction.replace(QLatin1Char('&'), QStringLiteral("&&"));

        const QString nameActionName = nameAction.replace(QLatin1Char(' '), QLatin1Char('_'));

        QAction *action;
        switch (t.type()) {
        case CustomTemplates::TReply:
            action = new QAction(nameAction, d->mOwnerActionCollection);   //krazy:exclude=tipsandthis
            d->mOwnerActionCollection->setDefaultShortcut(action, t.shortcut());
            d->mOwnerActionCollection->addAction(nameActionName, action);
            connect(action, SIGNAL(triggered(bool)), d->mCustomReplyMapper, SLOT(map()));
            d->mCustomReplyMapper->setMapping(action, idx);
            d->mCustomReplyActionMenu->addAction(action);
            d->mCustomTemplateActions.append(action);
            ++replyc;
            break;

        case CustomTemplates::TReplyAll:
            action = new QAction(nameAction, d->mOwnerActionCollection);   //krazy:exclude=tipsandthis
            d->mOwnerActionCollection->setDefaultShortcut(action, t.shortcut());
            d->mOwnerActionCollection->addAction(nameActionName, action);
            connect(action, SIGNAL(triggered(bool)), d->mCustomReplyAllMapper, SLOT(map()));
            d->mCustomReplyAllMapper->setMapping(action, idx);
            d->mCustomReplyAllActionMenu->addAction(action);
            d->mCustomTemplateActions.append(action);
            ++replyallc;
            break;

        case CustomTemplates::TForward:
            action = new QAction(nameAction, d->mOwnerActionCollection);   //krazy:exclude=tipsandthis
            d->mOwnerActionCollection->addAction(nameActionName, action);
            d->mOwnerActionCollection->setDefaultShortcut(action, t.shortcut());
            connect(action, SIGNAL(triggered(bool)), d->mCustomForwardMapper, SLOT(map()));
            d->mCustomForwardMapper->setMapping(action, idx);
            d->mCustomForwardActionMenu->addAction(action);
            d->mCustomTemplateActions.append(action);
            ++forwardc;
            break;

        case CustomTemplates::TUniversal:
            action = new QAction(nameAction, d->mOwnerActionCollection);   //krazy:exclude=tipsandthis
            d->mOwnerActionCollection->addAction(nameActionName, action);

            connect(action, SIGNAL(triggered(bool)), d->mCustomReplyMapper, SLOT(map()));
            d->mCustomReplyMapper->setMapping(action, idx);
            d->mCustomReplyActionMenu->addAction(action);
            d->mCustomTemplateActions.append(action);
            ++replyc;
            action = new QAction(nameAction, d->mOwnerActionCollection);   //krazy:exclude=tipsandthis
            connect(action, SIGNAL(triggered(bool)), d->mCustomReplyAllMapper, SLOT(map()));
            d->mCustomReplyAllMapper->setMapping(action, idx);
            d->mCustomReplyAllActionMenu->addAction(action);
            d->mCustomTemplateActions.append(action);
            ++replyallc;
            action = new QAction(nameAction, d->mOwnerActionCollection);   //krazy:exclude=tipsandthis
            connect(action, SIGNAL(triggered(bool)), d->mCustomForwardMapper, SLOT(map()));
            d->mCustomForwardMapper->setMapping(action, idx);
            d->mCustomForwardActionMenu->addAction(action);
            d->mCustomTemplateActions.append(action);
            ++forwardc;
            break;
        }

        ++idx;
    }

    if (!replyc) {
        QAction *noAction =
            d->mCustomReplyActionMenu->menu()->addAction(i18n("(no custom templates)"));
        noAction->setEnabled(false);
        d->mCustomReplyActionMenu->setEnabled(false);
    }
    if (!replyallc) {
        QAction *noAction =
            d->mCustomReplyAllActionMenu->menu()->addAction(i18n("(no custom templates)"));
        noAction->setEnabled(false);
        d->mCustomReplyAllActionMenu->setEnabled(false);
    }
    if (!forwardc) {
        QAction *noAction =
            d->mCustomForwardActionMenu->menu()->addAction(i18n("(no custom templates)"));
        noAction->setEnabled(false);
        d->mCustomForwardActionMenu->setEnabled(false);
    }
}

void CustomTemplatesMenu::slotReplySelected(int idx)
{
    Q_EMIT replyTemplateSelected(d->mCustomTemplates.at(idx));
}

void CustomTemplatesMenu::slotReplyAllSelected(int idx)
{
    Q_EMIT replyAllTemplateSelected(d->mCustomTemplates.at(idx));
}

void CustomTemplatesMenu::slotForwardSelected(int idx)
{
    Q_EMIT forwardTemplateSelected(d->mCustomTemplates.at(idx));
}

