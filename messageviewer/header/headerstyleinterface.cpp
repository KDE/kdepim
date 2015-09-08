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

#include "headerstyle.h"
#include "headerstyleinterface.h"
#include <QAction>
using namespace MessageViewer;

HeaderStyleInterface::HeaderStyleInterface(MessageViewer::HeaderStyle *headerStyle, MessageViewer::HeaderStrategy *headerStrategy, QObject *parent)
    : QObject(parent),
      mHeaderStyle(headerStyle),
      mHeaderStrategy(headerStrategy)
{

}

HeaderStyleInterface::~HeaderStyleInterface()
{
    qDeleteAll(mAction);
}

QList<KToggleAction *> HeaderStyleInterface::action() const
{
    return mAction;
}

void HeaderStyleInterface::addHelpTextAction(QAction *act, const QString &text)
{
    act->setStatusTip(text);
    act->setToolTip(text);
    if (act->whatsThis().isEmpty()) {
        act->setWhatsThis(text);
    }
}

