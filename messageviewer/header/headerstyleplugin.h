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

#ifndef HEADERSTYLEPLUGIN_H
#define HEADERSTYLEPLUGIN_H

#include <QObject>

#include "messageviewer_export.h"
class KToggleAction;
class KActionCollection;
namespace MessageViewer
{
class HeaderStyle;
class HeaderStrategy;
class HeaderStylePluginPrivate;
class MESSAGEVIEWER_EXPORT HeaderStylePlugin : public QObject
{
    Q_OBJECT
public:
    explicit HeaderStylePlugin(QObject *parent = Q_NULLPTR);
    ~HeaderStylePlugin();

    virtual HeaderStyle *headerStyle() const = 0;
    virtual HeaderStrategy *headerStrategy() const = 0;
    virtual KToggleAction *createAction(KActionCollection *ac) = 0;

private:
    HeaderStylePluginPrivate *const d;
};
}
#endif // HEADERSTYLEPLUGIN_H
