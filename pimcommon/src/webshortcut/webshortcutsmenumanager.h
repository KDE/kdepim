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

#ifndef WEBSHORTCUTSMENUMANAGER_H
#define WEBSHORTCUTSMENUMANAGER_H

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
    /**
     * Constructs a webshorts menu manager.
     *
     * @param parent The QObject parent.
     */

    explicit WebShortcutsMenuManager(QObject *parent = Q_NULLPTR);
    ~WebShortcutsMenuManager();

    /**
     * @brief return the selected text
     */
    QString selectedText() const;
    /**
     * @brief Set selected text
     * @param selectedText
     */
    void setSelectedText(const QString &selectedText);

    /**
     * @brief addWebShortcutsToMenu Manage to add WebShortCut action to existing menu.
     * @param menu
     */
    void addWebShortcutsToMenu(QMenu *menu);

private Q_SLOTS:
    void slotConfigureWebShortcuts();
    void slotHandleWebShortcutAction();

private:
    WebShortcutsMenuManagerPrivate *const d;
};
}

#endif // WEBSHORTCUTSMENUMANAGER_H
