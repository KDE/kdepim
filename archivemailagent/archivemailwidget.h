/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ARCHIVEMAILWIDGET_H
#define ARCHIVEMAILWIDGET_H

#include "ui_archivemailwidget.h"

class ArchiveMailWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ArchiveMailWidget( QWidget *parent = 0 );
  ~ArchiveMailWidget();
private:
  Ui::ArchiveMailWidget *mWidget;
};


#endif /* ARCHIVEMAILWIDGET_H */

