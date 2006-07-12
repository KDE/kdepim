/*
 * searchline.h
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
#ifndef MYSEARCHLINE_H
#define MYSEARCHLINE_H

#include <QWidget>
#include <QToolButton>

#include <klineedit.h>

class SearchLine : public QWidget
{
  Q_OBJECT
public:
  SearchLine(QWidget *parent = 0);
  ~SearchLine();

signals:
  void unreadToggled();
  void textChanged(const QString &);

private slots:
  void m_emitUnreadChanged();
  void m_emitTextChanged(const QString &filter);

private:
  QToolButton *clearButton;
  KLineEdit *searchLine;
};

#endif
