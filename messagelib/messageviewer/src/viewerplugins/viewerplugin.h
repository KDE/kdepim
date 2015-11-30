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

#ifndef VIEWERPLUGIN_H
#define VIEWERPLUGIN_H

#include <QObject>
#include "messageviewer_export.h"
class KActionCollection;

namespace MessageViewer
{
class ViewerPluginPrivate;
class ViewerPluginInterface;
class MESSAGEVIEWER_EXPORT ViewerPlugin : public QObject
{
    Q_OBJECT
public:
    explicit ViewerPlugin(QObject *parent = Q_NULLPTR);
    ~ViewerPlugin();

    virtual MessageViewer::ViewerPluginInterface *createView(QWidget *parent, KActionCollection *ac) = 0;
    virtual QString viewerPluginName() const = 0;

private:
    ViewerPluginPrivate *const d;
};
}
#endif // VIEWERPLUGIN_H
