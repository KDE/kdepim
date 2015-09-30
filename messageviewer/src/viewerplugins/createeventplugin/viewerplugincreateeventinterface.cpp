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

#include <job/createeventjob.h>

using namespace MessageViewer;

ViewerPluginCreateEventInterface::ViewerPluginCreateEventInterface(KActionCollection *ac, QWidget *parent)
    : ViewerPluginInterface(parent),
      mAction(Q_NULLPTR)
{
    mEventEdit = new EventEdit(parent);
    connect(mEventEdit, &EventEdit::createEvent, this, &ViewerPluginCreateEventInterface::slotCreateEvent);
    mEventEdit->setObjectName(QStringLiteral("eventedit"));
    parent->layout()->addWidget(mEventEdit);
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

void ViewerPluginCreateEventInterface::showWidget()
{
    mEventEdit->showEventEdit();
}

void ViewerPluginCreateEventInterface::setMessageItem(const Akonadi::Item &item)
{
    mMessageItem = item;
}

bool ViewerPluginCreateEventInterface::needValidMessage() const
{
    return true;
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

void ViewerPluginCreateEventInterface::slotCreateEvent(const KCalCore::Event::Ptr &eventPtr, const Akonadi::Collection &collection)
{
    CreateEventJob *createJob = new CreateEventJob(eventPtr, collection, mMessageItem, this);
    createJob->start();
}
