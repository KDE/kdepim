/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef STORAGESERVICENAVIGATIONBAR_H
#define STORAGESERVICENAVIGATIONBAR_H

#include <QToolBar>
class QAction;
class StorageServiceNavigationBar : public QToolBar
{
    Q_OBJECT
public:
    explicit StorageServiceNavigationBar(QWidget *parent=0);
    ~StorageServiceNavigationBar();

Q_SIGNALS:
    void goHome();
    void goToFolder(const QString &parent);

private:
    QAction *mGoBack;
    QAction *mGoForward;
};

#endif // STORAGESERVICENAVIGATIONBAR_H
