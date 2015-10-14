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

#ifndef VIEWERPLUGINTOOLMANAGER_H
#define VIEWERPLUGINTOOLMANAGER_H

#include <Item>
#include <QObject>
#include "messageviewer_export.h"
#include "viewerplugininterface.h"
class KActionCollection;
class QAction;
namespace MessageViewer
{
class ViewerPluginToolManagerPrivate;
class ViewerPluginInterface;
class MESSAGEVIEWER_EXPORT ViewerPluginToolManager : public QObject
{
    Q_OBJECT
public:
    explicit ViewerPluginToolManager(QWidget *parentWidget, QObject *parent = Q_NULLPTR);
    ~ViewerPluginToolManager();

    void closeAllTools();

    void createView();
    void setActionCollection(KActionCollection *ac);

    QList<QAction *> viewerPluginActionList(ViewerPluginInterface::SpecificFeatureTypes features) const;

    void updateActions(const Akonadi::Item &messageItem);
Q_SIGNALS:
    void activatePlugin(MessageViewer::ViewerPluginInterface *);

private:
    ViewerPluginToolManagerPrivate *const d;
};
}
#endif // VIEWERPLUGINTOOLMANAGER_H
