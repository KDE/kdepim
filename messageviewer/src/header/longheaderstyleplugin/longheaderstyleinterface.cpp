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

#include "longheaderstyleinterface.h"
#include <KToggleAction>
#include <KLocalizedString>
#include <KActionCollection>

using namespace MessageViewer;
LongHeaderStyleInterface::LongHeaderStyleInterface(MessageViewer::HeaderStylePlugin *plugin, QObject *parent)
    : MessageViewer::HeaderStyleInterface(plugin, parent)
{

}

LongHeaderStyleInterface::~LongHeaderStyleInterface()
{

}

void LongHeaderStyleInterface::createAction(KActionMenu *menu, QActionGroup *actionGroup, KActionCollection *ac)
{
    KToggleAction *act = new KToggleAction(i18nc("View->headers->", "&Long Headers"), this);
    ac->addAction(QStringLiteral("view_headers_long"), act);
    connect(act, &KToggleAction::triggered, this, &LongHeaderStyleInterface::slotStyleChanged);
    addHelpTextAction(act, i18n("Show long list of message headers"));
    mAction.append(act);
    addActionToMenu(menu, actionGroup);

}

void LongHeaderStyleInterface::activateAction()
{
    mAction.at(0)->setChecked(true);
}

