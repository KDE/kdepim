/*
 * conversationdelegate.cpp
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
 
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QStyle>
#include <QBrush>
#include <QSize>

#include "conversationdelegate.h"

ConversationDelegate::ConversationDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{
}

void ConversationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   painter->setRenderHint(QPainter::Antialiasing);
//   painter->setPen(Qt::NoPen);

   if (option.state & QStyle::State_Selected)
       painter->setBrush(option.palette.highlight());
   else
       painter->setBrush(Qt::white);
   painter->drawRect(option.rect);

   if (option.state & QStyle::State_Selected)
       painter->setBrush(option.palette.highlightedText());
   else
       painter->setBrush(Qt::black);
   painter->drawText(5, index.row()*20+18, "Authors");   
   painter->drawText(85, index.row()*20+18, "Subject");   

}

QSize ConversationDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                             const QModelIndex & /* index */) const
{
   return QSize(200, 20);
}
