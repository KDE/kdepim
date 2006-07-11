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
#include <QLabel>
#include <QHBoxLayout>
#include <QCheckBox>

#include <kiconloader.h>
#include <klocale.h>
#include <kapplication.h>

#include "searchline.h"

SearchLine::SearchLine(QWidget *parent) : QWidget(parent)
{
  clearButton = new QToolButton(this);
  QIcon icon = SmallIconSet(KApplication::isRightToLeft() ? "clear_left" : "locationbar_erase" );
  clearButton->setIcon(icon);

  QLabel *search = new QLabel(i18n("S&earch:"), this);
  searchLine = new KLineEdit(this);
  search->setBuddy(searchLine);

  QLabel *unread = new QLabel(i18n("Only &unread:"), this);
  QCheckBox *checkBox = new QCheckBox(this);
  unread->setBuddy(checkBox);

  connect(clearButton, SIGNAL(clicked()), searchLine, SLOT(clear()));

  QHBoxLayout* layout = new QHBoxLayout( this );
  layout->setMargin(0);
  layout->setSpacing( 5 );
  layout->addWidget(clearButton);
  layout->addWidget(search);
  layout->addWidget(searchLine);
  layout->addWidget(unread);
  layout->addWidget(checkBox);
}


SearchLine::~SearchLine()
{
  delete clearButton;
  delete searchLine;
}

#include "searchline.moc"
