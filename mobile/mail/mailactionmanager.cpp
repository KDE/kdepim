/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "mailactionmanager.h"

#include <akonadi/kmime/messagestatus.h>

#include <KLocale>
#include <KIcon>
#include <KAction>

#include <akonadi/entitytreemodel.h>
#include <KMime/KMimeMessage>

MailActionManager::MailActionManager(KActionCollection* actionCollection, QObject* parent)
  : QObject(parent), m_actionCollection(actionCollection), m_itemSelectionModel(0)
{
  KAction *action;
  action = actionCollection->addAction("mark_message_important");
  action->setText( i18n( "Important" ) );
  action->setIcon( KIcon( "emblem-important" ) );
  action->setCheckable(true);

  action = actionCollection->addAction("mark_message_action_item");
  action->setText( i18n( "Action Item" ) );
  action->setIcon( KIcon( "mail-mark-task" ) );
  action->setCheckable(true);

  action = actionCollection->addAction("write_new_email");
  action->setText( i18n("Write New Email") );

  action = actionCollection->addAction("message_reply");
  action->setText( i18n("Reply") );
  
  action = actionCollection->addAction("message_reply_to_all");
  action->setText( i18n("Reply to All") );

  action = actionCollection->addAction("message_reply_to_author");
  action->setText( i18n("Reply to Author") );

    action = actionCollection->addAction("message_reply_to_list");
  action->setText( i18n("Reply to Mailing List") );

  action = actionCollection->addAction("message_reply_variants");

  action = actionCollection->addAction("message_forward");
  action->setText( i18n("Forward") );

  action = actionCollection->addAction("message_forward_as_attachment");
  action->setText( i18n("Forward as Attachment") );

  action = actionCollection->addAction("message_redirect");
  action->setText( i18n("Redirect") );

  action = actionCollection->addAction("save_favorite");
  action->setText( i18n("Save Favorite") );

  action = actionCollection->addAction("message_send_again");
  action->setText( i18n("Send Again") );

}

void MailActionManager::setItemSelectionModel(QItemSelectionModel* selectionModel)
{
  m_itemSelectionModel = selectionModel;
  connect(m_itemSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(updateActions()));
  updateActions();
}

void MailActionManager::updateActions()
{
  if (!m_itemSelectionModel->hasSelection())
  {
    m_actionCollection->action("mark_message_important")->setEnabled(false);
    m_actionCollection->action("mark_message_action_item")->setEnabled(false);
    return;
  }
  const QModelIndexList list = m_itemSelectionModel->selectedRows();

  if (list.size() != 1)
    return;
  const QModelIndex idx = list.first();
  Akonadi::Item item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

  if (!item.isValid())
    return;

  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  Akonadi::MessageStatus status;
  status.setStatusFromFlags(item.flags());

  m_actionCollection->action("mark_message_important")->setEnabled(true);
  m_actionCollection->action("mark_message_action_item")->setEnabled(true);
  m_actionCollection->action("mark_message_important")->setChecked(status.isImportant());
  m_actionCollection->action("mark_message_action_item")->setChecked(status.isToAct());
}
