/*
  Copyright (c) 2011 Volker Krause <vkrause@kde.org>

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

#include "../core/messageitem_p.h"

#include <qtest_kde.h>
#include <QObject>

using namespace MessageList::Core;

class ItemSizeTest : public QObject
{
  Q_OBJECT
  private slots:

    void testItemSize()
    {
      qDebug() << sizeof(Item);
      QVERIFY( sizeof(Item) <= 16 );
      qDebug() << sizeof(ItemPrivate);
      QVERIFY( sizeof(ItemPrivate) <= 96 );
      qDebug() << sizeof(MessageItem);
      QVERIFY( sizeof(MessageItem) <= 32 );
      qDebug() << sizeof(MessageItemPrivate);
      QVERIFY( sizeof(MessageItemPrivate) <= 152 );
    }
};

QTEST_KDEMAIN( ItemSizeTest, GUI )

#include "itemsizetest.moc"
