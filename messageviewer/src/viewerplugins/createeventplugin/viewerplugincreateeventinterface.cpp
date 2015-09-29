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

#include <KActionCollection>
#include <QHBoxLayout>
#include <QIcon>
#include <QAction>

using namespace MessageViewer;

ViewerPluginCreateEventInterface::ViewerPluginCreateEventInterface(KActionCollection *ac, QWidget *parent)
    : ViewerPluginInterface(parent),
      mAction(Q_NULLPTR)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    setLayout(hbox);
    mEventEdit = new EventEdit(this);
    mEventEdit->setObjectName(QStringLiteral("eventedit"));
    hbox->addWidget(mEventEdit);
    mEventEdit->hide();
    createAction(ac);
}

ViewerPluginCreateEventInterface::~ViewerPluginCreateEventInterface()
{

}

void ViewerPluginCreateEventInterface::setText(const QString &text)
{
    Q_UNUSED(text);
    //Nothing
}

QAction *ViewerPluginCreateEventInterface::action() const
{
    return mAction;
}

void ViewerPluginCreateEventInterface::setMessage(const KMime::Message::Ptr &value)
{
    mEventEdit->setMessage(value);
}

void ViewerPluginCreateEventInterface::closePlugin()
{
    mEventEdit->slotCloseWidget();
}

void ViewerPluginCreateEventInterface::createAction(KActionCollection *ac)
{
    if (ac) {
        mAction = new QAction(QIcon::fromTheme(QStringLiteral("appointment-new")), i18n("Create Event..."), this);
        mAction->setIconText(i18n("Create Event"));
        addHelpTextAction(mAction, i18n("Allows you to create a calendar Event"));
        ac->addAction(QStringLiteral("create_event"), mAction);
        ac->setDefaultShortcut(mAction, QKeySequence(Qt::CTRL + Qt::Key_E));
        connect(mAction, &QAction::triggered, this, &ViewerPluginCreateEventInterface::slotActivatePlugin);
    }
}
