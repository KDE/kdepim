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

#ifndef TEMPLATEPARSER_CUSTOMTEMPLATESMENU_H
#define TEMPLATEPARSER_CUSTOMTEMPLATESMENU_H

#include "templateparser_export.h"

#include <QList>
#include <QObject>
#include <QStringList>

class QSignalMapper;

class KActionCollection;
class QAction;
class KActionMenu;

namespace TemplateParser
{

class TEMPLATEPARSER_EXPORT CustomTemplatesMenu : public QObject
{
    Q_OBJECT

public:
    CustomTemplatesMenu(QWidget *parent, KActionCollection *ac);
    ~CustomTemplatesMenu();

    KActionMenu *replyActionMenu() const
    {
        return mCustomReplyActionMenu;
    }
    KActionMenu *replyAllActionMenu() const
    {
        return mCustomReplyAllActionMenu;
    }
    KActionMenu *forwardActionMenu() const
    {
        return mCustomForwardActionMenu;
    }

public Q_SLOTS:
    void update();

signals:
    void replyTemplateSelected(const QString &tmpl);
    void replyAllTemplateSelected(const QString &tmpl);
    void forwardTemplateSelected(const QString &tmpl);

private Q_SLOTS:
    void slotReplySelected(int idx);
    void slotReplyAllSelected(int idx);
    void slotForwardSelected(int idx);

private:
    void clear();

    KActionCollection *mOwnerActionCollection;

    QStringList mCustomTemplates;
    QList<QAction *> mCustomTemplateActions;

    // Custom template actions menu
    KActionMenu *mCustomReplyActionMenu, *mCustomReplyAllActionMenu, *mCustomForwardActionMenu;

    // Signal mappers for custom template actions
    QSignalMapper *mCustomReplyMapper, *mCustomReplyAllMapper, *mCustomForwardMapper;
};

}

#endif
