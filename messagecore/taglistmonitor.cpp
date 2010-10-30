/* Copyright 2010 Casey Link <unnamedrambler@gmail.com>
   Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "taglistmonitor.h"

using namespace MessageCore;

class MessageCore::TagListMonitor::Private
{
  public:
    static QList<TagListMonitor*> mInstances;
};

QList<TagListMonitor*> TagListMonitor::Private::mInstances;

TagListMonitor::TagListMonitor( QObject *parent )
  : QObject( parent ), d( 0 )
{
  Private::mInstances.append( this );
}

TagListMonitor::~TagListMonitor()
{
  Private::mInstances.removeAll( this );
}

void TagListMonitor::triggerUpdate()
{
  foreach( TagListMonitor *instance, Private::mInstances ) {
    emit instance->tagsChanged();
  }
}
