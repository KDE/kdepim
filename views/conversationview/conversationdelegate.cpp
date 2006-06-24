#include <QPainter>
#include <QStyleOptionViewItem>
#include <QmodelIndex>
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
   painter->setPen(Qt::NoPen);

   if (option.state & QStyle::State_Selected)
       painter->setBrush(option.palette.highlight());
   else
       painter->setBrush(QBrush(Qt::white));
   painter->drawRect(option.rect);

   if (option.state & QStyle::State_Selected)
       painter->setBrush(option.palette.highlightedText());
   else
       painter->setBrush(QBrush(Qt::black));
   painter->drawText(0, 0, "Authors");   
   painter->drawText(100, 20, "Subject");   

}

QSize ConversationDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                             const QModelIndex & /* index */) const
{
   return QSize(200, 20);
}
