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

#ifndef BACKUPMAILWIDGET_H
#define BACKUPMAILWIDGET_H

#include <QWidget>
class QListWidget;

namespace KPIM {
  class CustomLogWidget;
}

class BackupMailWidget : public QWidget
{
public:
  explicit BackupMailWidget(QWidget *parent);
  ~BackupMailWidget();
  void addInfoLogEntry( const QString& log );
  void addErrorLogEntry( const QString& log );
  void clear();

private:
  KPIM::CustomLogWidget *mCustomLogWidget;
};

#endif // BACKUPMAILWIDGET_H
