/*
  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "options.h"

Options::Options() : m_action(ActionNone), m_stripOldAlarms(false)
{
}

void Options::setAction(Options::Action action)
{
    m_action = action;
}

Options::Action Options::action() const
{
    return m_action;
}

QList<Akonadi::Entity::Id> Options::collections() const
{
    return m_collectionIds;
}

void Options::setCollections(const QList<Akonadi::Collection::Id> &collections)
{
    m_collectionIds = collections;
}

bool Options::testCollection(Akonadi::Entity::Id id) const
{
    return m_collectionIds.isEmpty() || m_collectionIds.contains(id);
}

bool Options::stripOldAlarms() const
{
    return m_stripOldAlarms;
}

void Options::setStripOldAlarms(bool strip)
{
    m_stripOldAlarms = strip;
}
