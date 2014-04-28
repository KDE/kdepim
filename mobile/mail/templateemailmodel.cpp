/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "templateemailmodel.h"
#include <AkonadiCore/entitytreemodel.h>
#include <KMime/kmime_message.h>

QVariant TemplateEmailModel::data(const QModelIndex& index, int role) const
{
  if ( role == Qt::DisplayRole ) {
    KMime::Message::Ptr message = Akonadi::SelectionProxyModel::data(index, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>().payload<KMime::Message::Ptr>();
    return message->subject()->asUnicodeString();
  } else {
    return Akonadi::SelectionProxyModel::data(index, role);
  }
}
