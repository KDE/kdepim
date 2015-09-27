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

#include "viewerplugincreateeventinterface.h"
#include "widgets/eventedit.h"
#include <KLocalizedString>

#include <QHBoxLayout>

using namespace MessageViewer;

ViewerPluginCreateEventInterface::ViewerPluginCreateEventInterface(QWidget *parent)
    : ViewerPluginInterface(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    setLayout(hbox);
    EventEdit *edit = new EventEdit(this);
    edit->setObjectName(QStringLiteral("eventedit"));
    hbox->addWidget(edit);
}

ViewerPluginCreateEventInterface::~ViewerPluginCreateEventInterface()
{

}

void ViewerPluginCreateEventInterface::setText(const QString &text)
{
    Q_UNUSED(text);
    //Nothing
}

KToggleAction *ViewerPluginCreateEventInterface::action() const
{
    //TODO
    return Q_NULLPTR;
}

