/*
 * Demo app which shows off the ConversationView
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

#include <QApplication>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QWidget>

#include "maildisplay.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  QWidget *widget = new QWidget;
  QVBoxLayout *layout = new QVBoxLayout(widget);
  widget->setLayout(layout);
  layout->addWidget(new MailDisplay);
  layout->addWidget(new MailDisplay);
  layout->addWidget(new MailDisplay);
  layout->addWidget(new MailDisplay);

//  QScrollArea *scrollarea = new QScrollArea;
//  scrollarea->setWidget(widget);
//  scrollarea->show();
  widget->show();
  return app.exec();
}
