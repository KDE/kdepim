/*
 * mailview.cpp
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

#include <QtGlobal>
#include <QtDebug>

#include <QTextFrame>
#include <QTextFrameFormat>
#include <QTextLength>
#include <QTextDocument>
#include <QTimer>
#include <QModelIndex>

#include "conversation.h"
#include "folderproxymodel.h"
#include "foldermodel.h"
#include "mailview.h"
#include "mailvieweventfilter.h"

MailView::MailView(FolderProxyModel *model, QWidget *parent) : QScrollArea(parent), m_model(model) 
{
  m_layout = new QVBoxLayout(this);
  m_edit1 = new QTextEdit(this);
  m_edit2 = new QTextEdit(this);

  m_edit1->setHtml("sdf sdfkjdhfkjshf");
  //m_edit1->setReadOnly(true); 
  m_edit1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_edit1->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_edit1->viewport()->installEventFilter(new MailViewEventFilter(m_edit1, this));
  m_edit1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  m_edit2->setHtml("df sdfsdfjsdf kdf d");
  //m_edit2->setReadOnly(true); 
  m_edit2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_edit2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_edit2->viewport()->installEventFilter(new MailViewEventFilter(m_edit2->viewport(), this));
  m_edit2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  m_layout->addWidget(m_edit1);
  m_layout->addWidget(m_edit2);
  m_layout->setMargin(15);
  m_layout->setSpacing(15);
//   m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
//  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  resize(100, 75);

  setLayout(m_layout);
  resize(100, 75);
  t.start(); 
  m_id = -1;
}

/*
 * This doesn't work as the QTextLength of the QTextFrameFormat of the root QTextFrame 
 * of the QTextDocument corresponding to QTextEdit (phew!) is of variable length and 
 * not of fixed length. I'll have to dive in and check if there is a way to transform 
 * that to a fixed length.
 * Another option is to increase the size until it no longer has a scrollbar, but that
 * is a BAD solution...
 * Hopefully, KHTML can do what I want. (Though it doesn't seem so.) Or I could convince 
 * the KHTML hackers this is useful, so they'll add it. Else I have to implement it 
 * myself. *Dreadfull thought*! :P
 */
int MailView::getNeededHeight() const
{
//  return qRound(document()->rootFrame()->frameFormat().height().rawValue());
  qDebug() << m_edit1->viewport()->sizeHint();
  return -1;
}

void MailView::updateHeight()
{
  setMinimumHeight(getNeededHeight());
}

/** 
 * Displays the conversation corresponding to the given index in this conversation display.
 **/
void MailView::setConversation(const QModelIndex &index)
{
  m_id = m_model->id(index);
  m_edit1->setHtml("");
  Conversation* c = m_model->conversation(m_id);
  int max = c->count()-1;
  QString tmp = "<H2><A NAME=top>";
  tmp.append(c->subject());
  tmp.append("</A></H2><HR>");
  m_edit1->append(tmp);
  for (int count = 0; count < max; ++count) {
    tmp = "<A NAME=";
    tmp.append(count);
    tmp.append("><B>");
    tmp.append(c->author(count));
    tmp.append("</B></A>, ");
    tmp.append(c->arrivalTimeInText(count));
    tmp.append("<BR>");
    tmp.append(c->htmlContent(count));
    m_edit1->append(tmp);
  }
  tmp = "<A NAME=";
  tmp.append(max);
  tmp.append("><B>");
  tmp.append(c->author(max));
  tmp.append("</B></A>, ");
  tmp.append(c->arrivalTimeInText(max));
  tmp.append("<BR>");
  tmp.append(c->content(max));
  m_edit1->append(tmp);
  m_edit1->scrollToAnchor("top");
  QTimer::singleShot(3000, this, SLOT(markAsRead()));
  t.restart();
}

void MailView::markAsRead()
{
  if (t.elapsed() >= 3000 && m_id >= 0)
    m_model->markConversationAsRead(m_id, true);
}

#include "mailview.moc"
