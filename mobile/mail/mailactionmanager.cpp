/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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

MailActionManager::MailActionManager( KActionCollection *actionCollection, QObject *parent )
  : QObject( parent ),
    m_actionCollection( actionCollection ),
    m_itemSelectionModel( 0 ),
    m_itemActionSelectionModel( 0 )
{
  KAction *action;
  action = actionCollection->addAction( QLatin1String("mark_message_important") );
  action->setText( i18n( "Important" ) );
  action->setIcon( KIcon( QLatin1String("emblem-important") ) );
  action->setCheckable(true);

  action = actionCollection->addAction( QLatin1String("mark_message_action_item") );
  action->setText( i18n( "Action Item" ) );
  action->setIcon( KIcon( QLatin1String("mail-mark-task") ) );
  action->setCheckable( true );

  action = actionCollection->addAction( QLatin1String("write_new_email") );
  action->setText( i18n( "Write New Email" ) );

  action = actionCollection->addAction( QLatin1String("send_queued_emails") );
  action->setText( i18n( "Send All Unsent Emails" ) );

  action = actionCollection->addAction( QLatin1String("send_queued_emails_via") );
  action->setText( i18n( "Send All Unsent Emails" ) );

  action = actionCollection->addAction( QLatin1String("message_reply") );
  action->setText( i18n( "Reply" ) );

  action = actionCollection->addAction( QLatin1String("message_reply_to_list") );
  action->setText( i18n( "Reply to Mailing List" ) );

  action = actionCollection->addAction( QLatin1String("message_reply_without_quoting") );
  action->setText( i18n( "Reply Without Quoting" ) );

  action = actionCollection->addAction( QLatin1String("message_reply_variants") );

  action = actionCollection->addAction( QLatin1String("message_forward_as_attachment") );
  action->setText( i18n( "Forward as Attachment" ) );

  action = actionCollection->addAction( QLatin1String("message_redirect") );
  action->setText( i18n( "Redirect" ) );

  action = actionCollection->addAction( QLatin1String("save_favorite") );
  action->setText( i18n( "Save Favorite" ) );

  action = actionCollection->addAction( QLatin1String("message_send_again") );
  action->setText( i18n( "Send Again" ) );

  action = actionCollection->addAction( QLatin1String("message_save_as") );
  action->setText( i18n( "Save Email As" ) );

  action = actionCollection->addAction( QLatin1String("message_edit") );
  action->setText( i18n( "Edit Email" ) );

  action = actionCollection->addAction( QLatin1String("message_find_in") );
  action->setText( i18n( "Find in Email" ) );

  action = actionCollection->addAction( QLatin1String("prefer_html_to_plain") );
  action->setText( i18n( "Prefer HTML To Plain Text" ) );
  action->setCheckable( true );
  action->setChecked( false );

  action = actionCollection->addAction( QLatin1String("prefer_html_to_plain_viewer") );
  action->setText( i18n( "Prefer HTML To Plain Text" ) );
  action->setCheckable(true);
  action->setChecked(false);

  action = actionCollection->addAction( QLatin1String("load_external_ref") );
  action->setText( i18n( "Load External References" ) );
  action->setCheckable( true );
  action->setChecked( false );

  action = actionCollection->addAction( QLatin1String("message_fixed_font") );
  action->setText( i18n( "Use Fixed Font" ) );
  action->setCheckable( true );
  action->setChecked( false );

  action = actionCollection->addAction( QLatin1String("move_all_to_trash") );
  action->setText( i18n( "Move Displayed Emails To Trash" ) );

  action = actionCollection->addAction( QLatin1String("create_todo_reminder") );
  action->setText( i18n( "Create Task From Email" ) );

  action = actionCollection->addAction( QLatin1String("create_event") );
  action->setText( i18n( "Create Event From Email" ) );

  action = actionCollection->addAction( QLatin1String("apply_filters") );
  action->setText( i18n( "Apply Filters" ) );

  action = actionCollection->addAction( QLatin1String("apply_filters_bulk_action") );
  action->setText( i18n( "Apply Filters" ) );
  action->setEnabled( false );

  action = actionCollection->addAction( QLatin1String("new_filter") );
  action->setText( i18n( "New Filter" ) );
}

void MailActionManager::setItemSelectionModel( QItemSelectionModel *selectionModel )
{
  m_itemSelectionModel = selectionModel;
  connect( m_itemSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(updateActions()) );
  updateActions();
}

void MailActionManager::setItemActionSelectionModel( QItemSelectionModel *selectionModel )
{
  m_itemActionSelectionModel = selectionModel;
  connect( m_itemActionSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(updateActions()) );
  updateActions();
}

void MailActionManager::updateActions()
{
  if ( m_itemActionSelectionModel )
    m_actionCollection->action( QLatin1String("apply_filters_bulk_action") )->setEnabled( m_itemActionSelectionModel->hasSelection() );

  if ( !m_itemSelectionModel->hasSelection() ) {
    m_actionCollection->action( QLatin1String("mark_message_important") )->setEnabled( false );
    m_actionCollection->action( QLatin1String("mark_message_action_item") )->setEnabled( false );
    return;
  }

  const QModelIndexList list = m_itemSelectionModel->selectedRows();
  if ( list.size() != 1 )
    return;

  const QModelIndex index = list.first();
  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  if ( !item.isValid() )
    return;

  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;

  Akonadi::MessageStatus status;
  status.setStatusFromFlags( item.flags() );

  m_actionCollection->action( QLatin1String("mark_message_important") )->setEnabled( true );
  m_actionCollection->action( QLatin1String("mark_message_action_item") )->setEnabled( true );
  m_actionCollection->action( QLatin1String("mark_message_important") )->setChecked( status.isImportant() );
  m_actionCollection->action( QLatin1String("mark_message_action_item") )->setChecked( status.isToAct() );
}
