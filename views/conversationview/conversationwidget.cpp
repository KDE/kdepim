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
  m_searchLine = new SearchLine;
  m_layout = new QGridLayout;

  m_layout->setSpacing(0);
  m_layout->setMargin(0);
  m_layout->addWidget(m_searchLine);
  m_layout->addWidget(m_view);

  connect(m_searchLine, SIGNAL(unreadToggled()), m_view, SLOT(toggleFilterUnread()));
  connect(m_searchLine, SIGNAL(textChanged(const QString &)), m_view, SLOT(changeFilter(const QString &)));

  setLayout(m_layout);
}


ConversationWidget::~ConversationWidget()
{
  delete m_layout;
  delete m_searchLine;
  delete m_view;
}

#include "conversationwidget.moc"
