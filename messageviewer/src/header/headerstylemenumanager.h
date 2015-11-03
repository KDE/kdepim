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

#ifndef HEADERSTYLEMENU_H
#define HEADERSTYLEMENU_H

#include "messageviewer_export.h"

#include <QObject>
class KActionMenu;
class KActionCollection;
namespace MessageViewer
{
class HeaderStylePlugin;
class HeaderStyleMenuManagerPrivate;
class MESSAGEVIEWER_EXPORT HeaderStyleMenuManager : public QObject
{
    Q_OBJECT
public:
    explicit HeaderStyleMenuManager(KActionCollection *ac, QObject *parent = Q_NULLPTR);
    ~HeaderStyleMenuManager();

    KActionMenu *menu() const;

    void setPluginName(const QString &pluginName);
    void readConfig();

public Q_SLOTS:
    void slotStyleChanged(MessageViewer::HeaderStylePlugin *plugin);
Q_SIGNALS:
    void styleChanged(MessageViewer::HeaderStylePlugin *plugin);
    void styleUpdated();

private:
    HeaderStyleMenuManagerPrivate *const d;
};
}
#endif // HEADERSTYLEMENU_H
