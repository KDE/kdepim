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
#include <QFont>
#include <QFontMetrics>

#include "conversationdelegate.h"
#include "conversation.h"

ConversationDelegate::ConversationDelegate(FolderModel *folderModel, QSortFilterProxyModel *proxyModel, QObject *parent) : QAbstractItemDelegate(parent)
{
  fmodel = folderModel;
  pmodel = proxyModel;
  margin = 5;
}

ConversationDelegate::~ConversationDelegate()
{
}

void ConversationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Conversation *c = fmodel->conversation(pmodel->mapToSource(index).row());
	painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(Qt::NoPen);
	painter->setFont(option.font);
 
  if (option.state & QStyle::State_Selected)
    painter->setBrush(option.palette.highlight());
  else
	  painter->setBrush(isOdd(index.row()) ? option.palette.alternateBase() : option.palette.base());
  painter->drawRect(option.rect);

  if (option.state & QStyle::State_Selected)
    painter->setPen(option.palette.highlightedText().color());      
  else
    painter->setPen(option.palette.text().color());

	if (index.column() == 0)
		paintAuthors(painter, option, c);
	else
		paintRest(painter, option, c);
}

void ConversationDelegate::paintAuthors(QPainter *painter, const QStyleOptionViewItem &option, const Conversation *c) const
{
	int flags = Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine;
	QFont possiblyBoldFont = option.font;
  if (c->isUnread())
  	possiblyBoldFont.setBold(true);
	QFontMetrics possiblyBoldMetrics(possiblyBoldFont);
  int messageCount = c->count();
  QString messageCountText = QString("(%L1)").arg(messageCount);
	QRect countBox = messageCount > 1 ? getCountBox(option, possiblyBoldMetrics.width(messageCountText)) : QRect();
	QRect authorsBox = getAuthorsBox(option, countBox);
  QString cauthors = getAuthors(option, c, authorsBox.width());

  painter->drawText(authorsBox, flags, cauthors);
  if (messageCount > 1) {
  	painter->setFont(possiblyBoldFont);
    painter->drawText(countBox, flags, messageCountText);
  	painter->setFont(option.font);
  }
}

void ConversationDelegate::paintRest(QPainter *painter, const QStyleOptionViewItem &option, const Conversation *c) const
{
	int flags = Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine;
	QFont possiblyBoldFont = option.font;
  if (c->isUnread())
  	possiblyBoldFont.setBold(true);
	QFontMetrics possiblyBoldMetrics(possiblyBoldFont);
  QString subject = c->conversationTitle();
  int subjectWidth = possiblyBoldMetrics.width(subject);
  QString time = c->arrivalTimeInText();
  int timeWidth = possiblyBoldMetrics.width(time);
  QRect timeBox = getRightBox(option, timeWidth); 
	QRect subjectBox = getMiddleBox(option, timeBox);

	painter->setFont(possiblyBoldFont);
	if (timeBox.width() > margin) {
		chop(option.fontMetrics, time, timeBox.width());
  	painter->drawText(timeBox, flags, time);
	}

	if (subjectBox.width() > margin) {
	  chop(possiblyBoldMetrics, subject, subjectBox.width());
  	painter->drawText(subjectBox, flags, subject);
  	if (subjectWidth < subjectBox.width() - margin) {
	    painter->setFont(option.font);
	    painter->setPen(option.palette.shadow().color());
	    QRect tmp = getSnippetBox(option, subjectBox, subjectWidth);
	    QString snippet = c->content(0);
	    painter->drawText(tmp, flags, snippet);
	  }
  }
}

inline QRect ConversationDelegate::getSnippetBox(const QStyleOptionViewItem &option, const QRect &parentBox, int parentWidth) const
{
	int width = parentBox.width() - margin - parentWidth;
  int x = option.direction == Qt::LeftToRight ? parentBox.right() - width : parentBox.left();
  int y = option.rect.top();
	int height = option.fontMetrics.height();
  return QRect(x, y, width, height);
}

inline QRect ConversationDelegate::getAuthorsBox(const QStyleOptionViewItem &option, const QRect &decoBox) const
{
  int y = option.rect.top();
  int decoWidth = decoBox.isNull() ? 0 : decoBox.width() + margin;
  int x = option.direction == Qt::LeftToRight ? option.rect.left() + margin : option.rect.left() - margin + decoWidth;
  int width = option.rect.width() - decoWidth;
	int height = option.fontMetrics.height();
  return QRect(x, y, width, height);
}

inline QString ConversationDelegate::getAuthors(const QStyleOptionViewItem &option, const Conversation *conversation, const int maxWidth) const
{
  QString authors = conversation->authors();  
  chop(option.fontMetrics, authors, maxWidth);
  return authors;
}

inline void ConversationDelegate::chop(const QFontMetrics &metrics, QString &orig, int width) const
{
  QString dots = QString("...");
  while (metrics.width(orig) > width && orig.size() > 4) {
    orig.chop(4);
    orig.append(dots);
  }
}

inline QRect ConversationDelegate::getCountBox(const QStyleOptionViewItem &option, int neededWidth) const
{
  int right = option.rect.right();
  int x = option.direction == Qt::LeftToRight ? right - neededWidth : option.rect.left();
  int y = option.rect.top();
	int height = option.fontMetrics.height();
  return QRect(x, y, neededWidth, height);
}

inline QRect ConversationDelegate::getMiddleBox(const QStyleOptionViewItem &option, /*const QRect &left,*/ const QRect &right) const
{
//  int x1 = left.left() + left.width() + margin;
  int x1 = (option.direction == Qt::LeftToRight ? option.rect.left() : right.right()) + margin;
  int x2 = (option.direction == Qt::LeftToRight ? right.left() : option.rect.right()) - margin;
  int width = x2 - x1;
	if (width <= 0) return QRect();
  int y = option.rect.top();
	int height = option.fontMetrics.height();
  return QRect(x1, y, width, height);
}

inline QRect ConversationDelegate::getRightBox(const QStyleOptionViewItem &option, int neededWidth) const
{
  int x2 = option.direction == Qt::LeftToRight ? 
  						option.rect.right() - margin : 
  						qMin(neededWidth + margin, option.rect.right() - margin);
  int x1 = option.direction == Qt::LeftToRight ? 
  						qMax(x2 - neededWidth, option.rect.left() + margin) :
  						option.rect.left() + margin;
  int width = x2 - x1;
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

QSize ConversationDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  int lineHeight = option.fontMetrics.lineSpacing();
	int rLineWidth;// = option.fontMetrics.width(text) + 2*margin;
  Conversation *c = fmodel->conversation(pmodel->mapToSource(index).row());
  if (index.column() == 0) {
	  QString text = c->authors();
	  if (c->count() > 1)
	  	text.append(QString("(%L1)").arg(c->count()));
		rLineWidth = option.fontMetrics.width(text) + 3*margin;
  } else {
  	QString text = c->conversationTitle();
  	text.append(c->arrivalTimeInText());
		rLineWidth = option.fontMetrics.width(text) + 2*margin;
  }
  return QSize(rLineWidth, lineHeight);
}

void ConversationDelegate::updateWidth(int pos, int /*nouse*/)
{
  lineWidth = pos;
}

inline bool ConversationDelegate::isOdd(int row) const { return row & 0x1; }

#include "conversationdelegate.moc"
