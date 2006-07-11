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
#include <QTextFrame>
#include <QTextFrameFormat>
#include <QTextLength>
#include <QtDebug>
#include <QTextDocument>

#include "mailview.h"

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
  return qRound(document()->rootFrame()->frameFormat().height().rawValue());
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
  setHtml("");
  Conversation* conversation = model->conversation((pmodel->mapFromSource(index)).row());
  int max = conversation->count()-1;
  QString tmp = "<H2><A NAME=top>";
  tmp.append(conversation->conversationTitle());
  tmp.append("</A></H2><HR>");
  append(tmp);
  for (int count = 0; count < max; ++count) {
    tmp = "<A NAME=";
    tmp.append(count);
    tmp.append("><B>");
    tmp.append(conversation->author(count));
    tmp.append("</B></A>, ");
    tmp.append(conversation->arrivalTimeInText(count));
    tmp.append("<BR>");
    tmp.append(conversation->content(count));
//                tmp.append("<HR>");
    append(tmp);
  }
  tmp = "<A NAME=";
  tmp.append(max);
  tmp.append("><B>");
  tmp.append(conversation->author(max));
  tmp.append("</B></A>, ");
  tmp.append(conversation->arrivalTimeInText(max));
  tmp.append("<BR>");
  tmp.append(conversation->content(max));
  append(tmp);
  scrollToAnchor("top");
}

#include "mailview.moc"
