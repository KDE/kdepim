/*
 * conversationwidget.cpp
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

#include "conversationwidget.h"

ConversationWidget::ConversationWidget(ConversationView *view, QWidget *parent) : QWidget(parent), m_view(view)
{
  searchLine = new SearchLine;
  layout = new QGridLayout;

  layout->setSpacing(0);
  layout->setMargin(0);
  layout->addWidget(searchLine);
  layout->addWidget(view);

  connect(searchLine, SIGNAL(unreadToggled()), view, SLOT(toggleFilterUnread()));
  connect(searchLine, SIGNAL(textChanged(const QString &)), view, SLOT(changeFilter(const QString &)));

  setLayout(layout);
}


ConversationWidget::~ConversationWidget()
{
  delete layout;
  delete searchLine;
}

#include "conversationwidget.moc"
