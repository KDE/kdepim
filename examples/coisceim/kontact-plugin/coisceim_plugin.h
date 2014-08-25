/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef COISCEIM_PLUGIN_H
#define COISCEIM_PLUGIN_H

#include "coisceimwidgetinterface.h"

#include <KontactInterface/UniqueAppHandler>

namespace KontactInterface
{
class Plugin;
}

class CoisceimUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
public:
    CoisceimUniqueAppHandler(KontactInterface::Plugin *plugin)
        : KontactInterface::UniqueAppHandler(plugin) {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};

class CoisceimPlugin : public KontactInterface::Plugin
{
    Q_OBJECT

public:
    CoisceimPlugin(KontactInterface::Core *core, const QVariantList &);
    ~CoisceimPlugin();
    int weight() const
    {
        return 600;
    }
    virtual bool isRunningStandalone() const;

private slots:
    void createTrip();
    org::kde::coisceim::CoisceimWidget *interface();

protected:
    KParts::ReadOnlyPart *createPart();
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;
    org::kde::coisceim::CoisceimWidget *m_interface;
};

#endif

