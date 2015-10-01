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

#ifndef WEBSHORTCUTMENUMANAGER_H
#define WEBSHORTCUTMENUMANAGER_H

#include <QObject>
#include "pimcommon_export.h"
class QMenu;
namespace PimCommon
{
class WebShortcutsMenuManagerPrivate;
class PIMCOMMON_EXPORT WebShortcutsMenuManager : public QObject
{
    Q_OBJECT
public:
    explicit WebShortcutsMenuManager(QObject *parent = Q_NULLPTR);
    ~WebShortcutsMenuManager();

    QString selectedText() const;
    void setSelectedText(const QString &selectedText);

    void addWebShortcutsToMenu(QMenu *menu);

private Q_SLOTS:
    void slotConfigureWebShortcuts();
    void slotHandleWebShortcutAction();

private:
    WebShortcutsMenuManagerPrivate *const d;
};
}

#endif // WEBSHORTCUTMENUMANAGER_H
