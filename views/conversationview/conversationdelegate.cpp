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
#include <QRect>
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
//  lineWidth = 510;
  leftBaseWidth = 175;
  margin = 5;
}

ConversationDelegate::~ConversationDelegate()
{
}

void ConversationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(Qt::NoPen);
  painter->setFont(option.font);
	int flags = Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine;

  if (option.state & QStyle::State_Selected)
    painter->setBrush(option.palette.highlight());
  else {
		if (isOdd(index.row()))
	    painter->setBrush(option.palette.alternateBase());
	  else
	    painter->setBrush(option.palette.base());
  }
  painter->drawRect(option.rect);

  if (option.state & QStyle::State_Selected) {
    painter->setPen(option.palette.highlightedText().color());      
  } else {
    painter->setPen(option.palette.text().color());
  }

  DummyKonadiConversation *c = fmodel->conversation(pmodel->mapToSource(index).row());
  QString ctitle = c->conversationTitle();
  QString ctime = c->arrivalTimeInText();
	int tmpLineWidth = option.rect.width();
  int dotsWidth = option.fontMetrics.width("...");

  int messageCount = c->count();
  QString messageCountText = QString("(%L1)").arg(messageCount);
  int messageCountWidth = option.fontMetrics.width(messageCountText);
  
	QRect countBox = messageCount > 1 ? getCountBox(option, messageCountText) : QRect();
	QRect authorsBox = getAuthorsBox(option, countBox);
  QString cauthors = getAuthors(option, c, authorsBox.width());//c->author(0);
	
  int linePos = option.rect.top();
  int authorWidth = qMin(leftBaseWidth - (messageCount > 1 ? messageCountWidth + margin : 0), 
                         tmpLineWidth - (messageCount > 1 ? messageCountWidth + 2*margin : 0) - margin);
//  int authorPos = margin + option.rect.left();
  int tmpAuthorWidth = authorWidth;
  if (option.fontMetrics.width(cauthors) > authorWidth && authorWidth > dotsWidth) {
    tmpAuthorWidth = authorWidth - dotsWidth;
//    painter->drawText(authorPos+tmpAuthorWidth, linePos, dotsWidth, option.fontMetrics.height(), flags, "...");
  }
//  painter->drawText(authorPos, linePos, tmpAuthorWidth, option.fontMetrics.height(), flags, cauthors);

  painter->drawText(authorsBox, flags, cauthors);
  if (messageCount > 1) {
    painter->drawText(countBox, flags, messageCountText);
  }

  int subjectPos = leftBaseWidth + 2*margin + option.rect.left();
  int timeWidth = option.fontMetrics.width(ctime);
  int subjectWidth = tmpLineWidth - subjectPos - timeWidth - 2*margin;
  int tmpSubjectWidth = subjectWidth;
  if (option.fontMetrics.width(ctitle) > subjectWidth && subjectWidth > dotsWidth) {
    tmpSubjectWidth = subjectWidth - dotsWidth;
    painter->drawText(subjectPos+tmpSubjectWidth, linePos, dotsWidth, option.fontMetrics.height(), flags, "...");
  }
  painter->drawText(subjectPos, linePos, tmpSubjectWidth, option.fontMetrics.height(), flags, ctitle);

  int timePos = qMax(tmpLineWidth - margin - timeWidth, leftBaseWidth + 2*margin)+option.rect.left();
  timeWidth = qMax(tmpLineWidth - (timePos + margin), 0);
  int tmpTimeWidth = timeWidth;
  if (option.fontMetrics.width(ctime) > timeWidth && timeWidth > dotsWidth) {
    tmpTimeWidth = timeWidth - dotsWidth;
    painter->drawText(timePos+tmpTimeWidth, linePos, dotsWidth, option.fontMetrics.height(), flags, "...");
  }
  painter->drawText(timePos, linePos, tmpTimeWidth, option.fontMetrics.height(), flags, ctime);
}

inline QRect ConversationDelegate::getAuthorsBox(const QStyleOptionViewItem &option, const QRect &decoBox) const
{
  int y = option.rect.top();
  int x = margin + option.rect.left();
  int decoWidth = decoBox.isNull() ? 0 : decoBox.width() + margin;
  int width = leftBaseWidth - decoWidth;
	int height = option.fontMetrics.height();
  return QRect(x, y, width, height);
}

inline QString ConversationDelegate::getAuthors(const QStyleOptionViewItem &option, const DummyKonadiConversation *conversation, int maxWidth) const
{
  QString authors = conversation->author(0);
  QString me;
  foreach (me, listOfMe) {
    authors.replace(QRegExp(me), "me");
  }
  int messageCount = conversation->count();
  for (int count = 1; count < messageCount; ++count) {
    QString tmpAuthor = conversation->author(count);
    foreach (me, listOfMe) {
      tmpAuthor.replace(QRegExp(me), tr("me"));
    }
    if (!authors.contains(tmpAuthor)) {
      authors.append(", ");
      authors.append(tmpAuthor);
    }
  }
  QString dots = QString("...");
  while (option.fontMetrics.width(authors) > maxWidth && authors.size() > 1) {
    authors.chop(4);
    authors.append("...");
  }
  return authors;
}

inline QRect ConversationDelegate::getCountBox(const QStyleOptionViewItem &option, const QString &count) const
{
  int y = option.rect.top();
  QRect tmpAuthorBox = getAuthorsBox(option);
  int right = tmpAuthorBox.left() + tmpAuthorBox.width();
	int width = option.fontMetrics.width(count);
  int x = right - width;
	int height = option.fontMetrics.height();
  return QRect(x, y, width, height);
}

inline QRect ConversationDelegate::getBox(const QStyleOptionViewItem &option, const QRect &left, const QRect &right) const
{
  int x1 = left.right() + margin;
  int x2 = right.left() + margin;
  int width = option.rect.width() - x1 - x2;
	if (width <= option.rect.width()) return QRect();
  int y = option.rect.top();
	int height = option.fontMetrics.height();
  return QRect(x1, y, width, height);
}

inline void ConversationDelegate::resizeBox(QRect &box, const QRect &deco) const
{
	box.setWidth(box.width() - margin - deco.width());
}

inline bool ConversationDelegate::printDecoBox(const QRect &box, const QRect &deco) const
{
	return (box.width() >= margin + 2*deco.width());
}

/*inline QString ConversationDelegate::getTitle(const QStyleOptionViewItem &option, const) const
{
	return QRect();
}*/


QSize ConversationDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
  int lineHeight = option.fontMetrics.height() + 2;
  int rLineWidth = qMax(lineWidth, leftBaseWidth+margin);
  return QSize(rLineWidth, lineHeight);
}

void ConversationDelegate::updateWidth(int pos, int /*nouse*/)
{
  lineWidth = pos;
}

inline bool ConversationDelegate::isOdd(int row) const { return row & 0x1; }
