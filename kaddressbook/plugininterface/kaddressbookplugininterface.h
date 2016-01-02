/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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

#ifndef KADDRESSBOOKPLUGININTERFACE_H
#define KADDRESSBOOKPLUGININTERFACE_H

#include <QObject>
#include <QVector>

#include <pimcommon/genericplugininterface.h>
#include <pimcommon/plugininterface.h>
class KActionCollection;
class QAction;
class MainWidget;
class KAddressBookPluginInterface : public PimCommon::PluginInterface
{
    Q_OBJECT
public:
    explicit KAddressBookPluginInterface(KActionCollection *ac, QObject *parent = Q_NULLPTR);
    ~KAddressBookPluginInterface();

    void setMainWidget(MainWidget *mainWidget);

    void initializeInterfaceRequires(PimCommon::GenericPluginInterface *interface) Q_DECL_OVERRIDE;
private:
    MainWidget *mMainWidget;
};

#endif // KADDRESSBOOKPLUGININTERFACE_H
