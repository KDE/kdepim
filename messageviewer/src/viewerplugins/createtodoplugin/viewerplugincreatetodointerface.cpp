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

#include "viewerplugincreatetodointerface.h"
#include "todoedit.h"

#include <KLocalizedString>

#include <KActionCollection>
#include <QHBoxLayout>
#include <QAction>

#include <job/createtodojob.h>

using namespace MessageViewer;

ViewerPluginCreateTodoInterface::ViewerPluginCreateTodoInterface(KActionCollection *ac, QWidget *parent)
    : ViewerPluginInterface(parent),
      mAction(Q_NULLPTR)
{
    mTodoEdit = new TodoEdit(parent);
    mTodoEdit->setObjectName(QStringLiteral("todoedit"));
    connect(mTodoEdit, &TodoEdit::createTodo, this, &ViewerPluginCreateTodoInterface::slotCreateTodo);
    parent->layout()->addWidget(mTodoEdit);
    mTodoEdit->hide();
    createAction(ac);
}

ViewerPluginCreateTodoInterface::~ViewerPluginCreateTodoInterface()
{

}

void ViewerPluginCreateTodoInterface::setText(const QString &text)
{
    Q_UNUSED(text);
    //Nothing
}

QAction *ViewerPluginCreateTodoInterface::action() const
{
    return mAction;
}

void ViewerPluginCreateTodoInterface::setMessage(const KMime::Message::Ptr &value)
{
    mTodoEdit->setMessage(value);
}

void ViewerPluginCreateTodoInterface::closePlugin()
{
    mTodoEdit->slotCloseWidget();
}

void ViewerPluginCreateTodoInterface::showWidget()
{
    mTodoEdit->showToDoWidget();
}

void ViewerPluginCreateTodoInterface::setMessageItem(const Akonadi::Item &item)
{
    mMessageItem = item;
}

bool ViewerPluginCreateTodoInterface::needValidMessageItem() const
{
    return true;
}

void ViewerPluginCreateTodoInterface::createAction(KActionCollection *ac)
{
    if (ac) {
        mAction = new QAction(QIcon::fromTheme(QStringLiteral("task-new")), i18n("Create Todo"), this);
        mAction->setIconText(i18n("Create To-do"));
        addHelpTextAction(mAction, i18n("Allows you to create a calendar to-do or reminder from this message"));
        mAction->setWhatsThis(i18n("This option starts the KOrganizer to-do editor with initial values taken from the currently selected message. Then you can edit the to-do to your liking before saving it to your calendar."));
        ac->addAction(QStringLiteral("create_todo"), mAction);
        ac->setDefaultShortcut(mAction, QKeySequence(Qt::CTRL + Qt::Key_T));
        connect(mAction, &QAction::triggered, this, &ViewerPluginCreateTodoInterface::slotActivatePlugin);
    }
}

void ViewerPluginCreateTodoInterface::slotCreateTodo(const KCalCore::Todo::Ptr &todoPtr, const Akonadi::Collection &collection)
{
    CreateTodoJob *createJob = new CreateTodoJob(todoPtr, collection, mMessageItem, this);
    createJob->start();
}
