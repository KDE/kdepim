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

#ifndef CUSTOMTOOLSPLUGIN_H
#define CUSTOMTOOLSPLUGIN_H

#include "pimcommon_export.h"
#include <QObject>
class KActionCollection;
namespace PimCommon
{
class CustomToolsWidgetNg;
class CustomToolsViewInterface;
class CustomToolsPluginPrivate;
class PIMCOMMON_EXPORT CustomToolsPlugin : public QObject
{
    Q_OBJECT
public:
    explicit CustomToolsPlugin(QObject *parent = Q_NULLPTR);
    ~CustomToolsPlugin();

    virtual PimCommon::CustomToolsViewInterface *createView(KActionCollection *ac, CustomToolsWidgetNg *parent = Q_NULLPTR) = 0;
    virtual QString customToolName() const = 0;

private:
    CustomToolsPluginPrivate *const d;
};
}
#endif // CUSTOMTOOLSPLUGIN_H
