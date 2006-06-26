/*
 * conversationdelegate.h
 *
 * copyright (c) Aron Bostrom <Aron.Bostrom at gmail.com>, 2006 
 *
 * this library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef CONVERSATIONDELEGATE_H
#define CONVERSATIONDELEGATE_H

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QFontMetrics>
#include <QModelIndex>
#include <QSize>

#include "foldermodel.h"

class ConversationDelegate : public QAbstractItemDelegate
{
  Q_OBJECT
public:
  ConversationDelegate(DummyKonadiAdapter &adapter, QObject *parent = 0);
  ~ConversationDelegate();

  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

public slots:
  void updateWidth(int pos, int nouse = 0);

private:
  DummyKonadiAdapter backend;
  int lineWidth, authorBaseWidth, margin;
};

#endif
