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

	if (index.column() == 0)
		paintAuthors(painter, option, fmodel->conversation(pmodel->mapToSource(index).row()));
	else
		paintRest(painter, option, fmodel->conversation(pmodel->mapToSource(index).row()));
}

void ConversationDelegate::paintAuthors(QPainter *painter, const QStyleOptionViewItem &option, const DummyKonadiConversation *c) const
{
	int flags = Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine;

  QString ctitle = c->conversationTitle();
  QString ctime = c->arrivalTimeInText();
  int messageCount = c->count();
  QString messageCountText = QString("(%L1)").arg(messageCount);
	QRect countBox = messageCount > 1 ? getCountBox(option, messageCountText) : QRect();
	QRect authorsBox = getAuthorsBox(option, countBox);
  QString cauthors = getAuthors(option, c, authorsBox.width());

  painter->drawText(authorsBox, flags, cauthors);
  if (messageCount > 1)
    painter->drawText(countBox, flags, messageCountText);
}

void ConversationDelegate::paintRest(QPainter *painter, const QStyleOptionViewItem &option, const DummyKonadiConversation *c) const
{
	int flags = Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine;

  QString ctitle = c->conversationTitle();
  QString ctime = c->arrivalTimeInText();
  QRect timeBox = getRightBox(option, option.fontMetrics.width(ctime)); 
	QRect subjectBox = getMiddleBox(option, timeBox);

	if (subjectBox.width() > margin) {
	  chop(option, ctitle, subjectBox.width());
  	painter->drawText(subjectBox, flags, ctitle);
  }
	if (timeBox.width() > margin) {
		chop(option, ctime, timeBox.width());
  	painter->drawText(timeBox, flags, ctime);
	}
}

inline QRect ConversationDelegate::getAuthorsBox(const QStyleOptionViewItem &option, const QRect &decoBox) const
{
  int y = option.rect.top();
  int x = margin + option.rect.left();
  int decoWidth = decoBox.isNull() ? 0 : decoBox.width() + margin;
  int width = option.rect.width() - decoWidth;
	int height = option.fontMetrics.height();
  return QRect(x, y, width, height);
}

inline QString ConversationDelegate::getAuthors(const QStyleOptionViewItem &option, const DummyKonadiConversation *conversation, const int maxWidth) const
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
  chop(option, authors, maxWidth);
  return authors;
}

inline void ConversationDelegate::chop(const QStyleOptionViewItem &option, QString &orig, int width) const
{
  QString dots = QString("...");
  while (option.fontMetrics.width(orig) > width && orig.size() > 4) {
    orig.chop(4);
    orig.append(dots);
  }
}

inline QRect ConversationDelegate::getCountBox(const QStyleOptionViewItem &option, const QString &count) const
{
  int right = option.rect.right();
	int width = option.fontMetrics.width(count);
  int x = right - width;
  int y = option.rect.top();
	int height = option.fontMetrics.height();
  return QRect(x, y, width, height);
}

inline QRect ConversationDelegate::getMiddleBox(const QStyleOptionViewItem &option, /*const QRect &left,*/ const QRect &right) const
{
//  int x1 = left.left() + left.width() + margin;
  int x1 = option.rect.left() + margin;
  int x2 = right.left() - margin;
  int width = x2 - x1;
	if (width <= 0) return QRect();
  int y = option.rect.top();
	int height = option.fontMetrics.height();
  return QRect(x1, y, width, height);
}

inline QRect ConversationDelegate::getRightBox(const QStyleOptionViewItem &option, int neededWidth) const
{
  int x2 = option.rect.right() - margin;
  int x1 = qMax(x2 - neededWidth, option.rect.left() + margin);
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
  int lineHeight = option.fontMetrics.height() + 2;
	int rLineWidth;// = option.fontMetrics.width(text) + 2*margin;
  DummyKonadiConversation *c = fmodel->conversation(pmodel->mapToSource(index).row());
  if (index.column() == 0) {
	  QString text = c->author(0);
	  QString me;
	  foreach (me, listOfMe) {
	    text.replace(QRegExp(me), "me");
	  }
	  int max = c->count();
	  for (int count = 1; count < max; ++count) {
	    QString tmpAuthor = c->author(count);
	    foreach (me, listOfMe) {
	      tmpAuthor.replace(QRegExp(me), tr("me"));
	    }
	    if (!text.contains(tmpAuthor)) {
	      text.append(", ");
	      text.append(tmpAuthor);
	    }
	  }
	  if (max > 1)
	  	text.append(QString("(%L1)").arg(max));
		rLineWidth = option.fontMetrics.width(text) + 3*margin;
	  
  } else {
  	QString text = c->conversationTitle();
  	text.append(c->arrivalTimeInText());
		rLineWidth = option.fontMetrics.width(text) + 3*margin;
  }
  return QSize(rLineWidth, lineHeight);
}

void ConversationDelegate::updateWidth(int pos, int /*nouse*/)
{
  lineWidth = pos;
}

inline bool ConversationDelegate::isOdd(int row) const { return row & 0x1; }
