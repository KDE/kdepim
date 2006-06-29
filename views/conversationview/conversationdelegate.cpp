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
#include <QDateTime>
#include <QtDebug>
#include <QRegExp>

#include "conversationdelegate.h"
#include "dummykonadiconversation.h"

ConversationDelegate::ConversationDelegate(FolderModel *folderModel, QSortFilterProxyModel *proxyModel, QStringList &me, QObject *parent) : QAbstractItemDelegate(parent)
{
  fmodel = folderModel;
  pmodel = proxyModel;
  listOfMe = me;
  lineWidth = 510;
  authorBaseWidth = 175;
  margin = 5;
}

ConversationDelegate::~ConversationDelegate()
{
}

void ConversationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  int lineHeight = option.fontMetrics.height() + 2;
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(Qt::NoPen);

  if (option.state & QStyle::State_Selected)
    painter->setBrush(option.palette.highlight());
  else {
  	if (index.row() % 2 == 1)
	    painter->setBrush(option.palette.background());
	  else
//	    painter->setBrush(option.palette.textBackground());
	    painter->setBrush(Qt::white);
  }
  painter->drawRect(option.rect);

  if (option.state & QStyle::State_Selected) {
    painter->setBrush(option.palette.highlightedText());
    painter->setPen(QPen(option.palette.highlightedText().color()));      
  } else {
    painter->setBrush(Qt::black);
    painter->setPen(Qt::black);
  }
  painter->setFont(option.font);
  DummyKonadiConversation *c = fmodel->conversation(pmodel->mapToSource(index).row());
  QString ctitle = c->conversationTitle();
  QString ctime = c->arrivalTimeInText();
  QString cauthors = c->author(0);
  QString me;
  foreach (me, listOfMe) {
    cauthors.replace(QRegExp(me), "me");
  }
  int messageCount = c->count();
  for (int count = 1; count < messageCount; ++count) {
    QString tmpAuthor = c->author(count);
    foreach (me, listOfMe) {
      tmpAuthor.replace(QRegExp(me), "me");
    }
    if (!cauthors.contains(tmpAuthor)) {
      cauthors.append(", ");
      cauthors.append(tmpAuthor);
    }
  }
	int tmpLineWidth = option.rect.width();
  int dotsWidth = option.fontMetrics.width("...");

  QString messageCountText = QString("(%L1)").arg(messageCount);
  int messageCountWidth = option.fontMetrics.width(messageCountText);

  int linePos = index.row() * lineHeight;
  linePos = option.rect.top(); // fixes an ugly bug!! :-)
  int authorWidth = qMin(authorBaseWidth - (messageCount > 1 ? messageCountWidth + margin : 0), 
                         tmpLineWidth - (messageCount > 1 ? messageCountWidth + 2*margin : 0) - margin);
  int authorPos = margin + option.rect.left();
  int tmpAuthorWidth = authorWidth;
  if (option.fontMetrics.width(cauthors) > authorWidth && authorWidth > dotsWidth) {
    tmpAuthorWidth = authorWidth - dotsWidth;
    painter->drawText(authorPos+tmpAuthorWidth, linePos, dotsWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, "...");
  }
  painter->drawText(authorPos, linePos, tmpAuthorWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, cauthors);

  if (messageCount > 1) {
    int messageCountPos = qMin(2*margin + authorWidth, tmpLineWidth - messageCountWidth - margin) + option.rect.left();
    painter->drawText(messageCountPos, linePos, messageCountWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, messageCountText);
  }

  int subjectPos = authorBaseWidth + 2*margin + option.rect.left();
  int timeWidth = option.fontMetrics.width(ctime);
  int subjectWidth = tmpLineWidth - subjectPos - timeWidth - 2*margin;
  int tmpSubjectWidth = subjectWidth;
  if (option.fontMetrics.width(ctitle) > subjectWidth && subjectWidth > dotsWidth) {
    tmpSubjectWidth = subjectWidth - dotsWidth;
    painter->drawText(subjectPos+tmpSubjectWidth, linePos, dotsWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, "...");
  }
  painter->drawText(subjectPos, linePos, tmpSubjectWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, ctitle);

  int timePos = qMax(tmpLineWidth - margin - timeWidth, authorBaseWidth + 2*margin)+option.rect.left();
  timeWidth = qMax(tmpLineWidth - (timePos + margin), 0);
  int tmpTimeWidth = timeWidth;
  if (option.fontMetrics.width(ctime) > timeWidth && timeWidth > dotsWidth) {
    tmpTimeWidth = timeWidth - dotsWidth;
    painter->drawText(timePos+tmpTimeWidth, linePos, dotsWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, "...");
  }
  painter->drawText(timePos, linePos, tmpTimeWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, ctime);
}

QSize ConversationDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
  int lineHeight = option.fontMetrics.height() + 2;
  int rLineWidth = qMax(lineWidth, 280);
  return QSize(rLineWidth, lineHeight);
}

void ConversationDelegate::updateWidth(int pos, int /*nouse*/)
{
  lineWidth = pos;
}
