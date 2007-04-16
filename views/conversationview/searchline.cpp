/*
 * searchline.cpp
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

#include <QIcon>

#include <kiconloader.h>
#include <klocale.h>
#include <kapplication.h>

#include "searchline.h"

SearchLine::SearchLine(QWidget *parent) : QWidget(parent)
{
  m_clearButton = new QToolButton(this);
  QIcon icon = KIcon(KApplication::isRightToLeft() ? "clear-left" : "locationbar-erase" );
  m_clearButton->setIcon(icon);

  m_search = new QLabel(i18n("S&earch:"), this);
  m_searchLine = new KLineEdit(this);
  m_search->setBuddy(m_searchLine);

  m_unread = new QLabel(i18n("Only &unread:"), this);
  m_checkBox = new QCheckBox(this);
  m_unread->setBuddy(m_checkBox);

  connect(m_clearButton, SIGNAL(clicked()), m_searchLine, SLOT(clear()));
  connect(m_checkBox, SIGNAL(stateChanged(int)), this, SLOT(m_emitUnreadChanged()));
  connect(m_searchLine, SIGNAL(textChanged(const QString&)), this, SLOT(m_emitTextChanged(const QString&)));

  m_layout = new QHBoxLayout(this);
  m_layout->setMargin(0);
  m_layout->setSpacing(5);
  m_layout->addWidget(m_clearButton);
  m_layout->addWidget(m_search);
  m_layout->addWidget(m_searchLine);
  m_layout->addWidget(m_unread);
  m_layout->addWidget(m_checkBox);
}


SearchLine::~SearchLine()
{
  delete m_clearButton;
  delete m_searchLine;
  delete m_search;
  delete m_unread;
  delete m_checkBox;
  delete m_layout;
}

void SearchLine::m_emitUnreadChanged()
{
  emit unreadToggled();
}

void SearchLine::m_emitTextChanged(const QString &filter)
{
  emit textChanged(filter);
}

#include "searchline.moc"
