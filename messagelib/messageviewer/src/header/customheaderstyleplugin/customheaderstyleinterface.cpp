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

#include "customheaderstyleinterface.h"
#include <KToggleAction>
#include <KLocalizedString>
#include <KActionCollection>

using namespace MessageViewer;
CustomHeaderStyleInterface::CustomHeaderStyleInterface(MessageViewer::HeaderStylePlugin *plugin, QObject *parent)
    : MessageViewer::HeaderStyleInterface(plugin, parent)
{

}

CustomHeaderStyleInterface::~CustomHeaderStyleInterface()
{

}

void CustomHeaderStyleInterface::createAction(KActionMenu *menu, QActionGroup *actionGroup, KActionCollection *ac)
{
    KToggleAction *act = new KToggleAction(i18nc("View->headers->", "&Custom Headers"), this);
    ac->addAction(QStringLiteral("view_custom_headers"), act);
    connect(act, &KToggleAction::triggered, this, &CustomHeaderStyleInterface::slotStyleChanged);
    addHelpTextAction(act, i18n("Show custom headers"));
    mAction.append(act);
    addActionToMenu(menu, actionGroup);
}

void CustomHeaderStyleInterface::activateAction()
{
    mAction.at(0)->setChecked(true);
}

