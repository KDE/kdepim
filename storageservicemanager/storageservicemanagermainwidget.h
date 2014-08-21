/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef STORAGESERVICEMANAGERMAINWIDGET_H
#define STORAGESERVICEMANAGERMAINWIDGET_H

#include <QStackedWidget>
class StorageServiceConfigureStorageWidget;
class StorageServiceTabWidget;
class StorageServiceManagerMainWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit StorageServiceManagerMainWidget(QWidget *parent = 0);
    ~StorageServiceManagerMainWidget();

    StorageServiceTabWidget *storageServiceTabWidget() const;

Q_SIGNALS:
    void configureClicked();

private slots:
    void slotTabCountchanged(bool hasTab);

private:
    StorageServiceConfigureStorageWidget *mConfigureWidget;
    StorageServiceTabWidget *mStorageServiceTabWidget;
};

#endif // STORAGESERVICEMANAGERMAINWIDGET_H
