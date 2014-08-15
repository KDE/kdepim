/*
    This file is part of Akonadi.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

// READ THE README FILE

#include "mixedtreemodel.h"

#include <KMime/Message>
#include <KLocalizedString>
#include <KCalCore/Incidence>
#include "note.h"
#include <KABC/Addressee>

MixedTreeModel::MixedTreeModel(Akonadi::ChangeRecorder* monitor, QObject* parent)
  : EntityTreeModel(monitor, parent)
{

}

int MixedTreeModel::entityColumnCount(Akonadi::EntityTreeModel::HeaderGroup headerGroup) const
{
  if (headerGroup == CollectionTreeHeaders)
    return 1;

  if (headerGroup == ItemListHeaders)
    return 2;
  return 1;
}

QVariant MixedTreeModel::entityHeaderData(int section, Qt::Orientation orientation, int role, Akonadi::EntityTreeModel::HeaderGroup headerGroup) const
{
  if (headerGroup == CollectionTreeHeaders || role != Qt::DisplayRole || orientation == Qt::Vertical)
    return Akonadi::EntityTreeModel::entityHeaderData(section, orientation, role, headerGroup);

  switch (section)
  {
  case 0:
    return i18n("Name/Subject");
  case 1:
    return i18n("Detail");
  default:
    return QVariant();
  }
}


QVariant MixedTreeModel::entityData(const Akonadi::Item& item, int column, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (item.hasPayload<KMime::Message::Ptr>())
    {
      KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
      switch(column)
      {
      case 0:
        return message->subject()->asUnicodeString();
      case 1:
      {
        if (item.mimeType() == KMime::Message::mimeType())
          return message->from()->asUnicodeString();
        else
          return QString(message->mainBodyPart()->decodedText().mid(0, 30) + QLatin1String("..." ));
      }
      default:
        return QVariant();
      }
    }

    if (item.hasPayload<KABC::Addressee>())
    {
      KABC::Addressee addressee = item.payload<KABC::Addressee>();
      switch(column)
      {
      case 0:
        return addressee.name();
      case 1:
        return addressee.preferredEmail();
      default:
        return QVariant();
      }
    }

    if (item.hasPayload<KCalCore::Incidence::Ptr>()) {
      KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
      switch(column) {
      case 0:
        return incidence->summary();
      case 1:
        return incidence->description();
      default:
        return QVariant();
      }
    }
  }
  return EntityTreeModel::entityData(item, column, role);
}

QVariant MixedTreeModel::entityData(const Akonadi::Collection& collection, int column, int role) const
{
  return Akonadi::EntityTreeModel::entityData(collection, column, role);
}




