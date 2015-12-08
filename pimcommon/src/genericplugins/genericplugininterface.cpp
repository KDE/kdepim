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

#include "genericplugininterface.h"

using namespace PimCommon;

class PimCommon::GenericPluginInterfacePrivate
{
public:
    GenericPluginInterfacePrivate()
        : parentWidget(Q_NULLPTR)
    {

    }
    ActionType actionType;
    QWidget *parentWidget;
};

GenericPluginInterface::GenericPluginInterface(QObject *parent)
    : QObject(parent),
      d(new GenericPluginInterfacePrivate)
{

}

GenericPluginInterface::~GenericPluginInterface()
{
    delete d;
}

void GenericPluginInterface::setParentWidget(QWidget *parent)
{
    d->parentWidget = parent;
}

QWidget *GenericPluginInterface::parentWidget() const
{
    return d->parentWidget;
}

void GenericPluginInterface::setActionType(const ActionType &type)
{
    d->actionType = type;
}

ActionType GenericPluginInterface::actionType() const
{
    return d->actionType;
}

void GenericPluginInterface::setItems(const Akonadi::Item::List &items)
{
    Q_UNUSED(items);
}

void GenericPluginInterface::setCurrentCollection(const Akonadi::Collection &col)
{
    Q_UNUSED(col);
}

ActionType::ActionType(QAction *action, ActionType::Type type)
    : mAction(action),
      mType(type)
{
}

QAction *ActionType::action() const
{
    return mAction;
}

ActionType::Type ActionType::type() const
{
    return mType;
}
