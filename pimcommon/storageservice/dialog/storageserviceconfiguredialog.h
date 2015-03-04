/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef PIMCOMMON_STORAGESERVICECONFIGUREDIALOG_H
#define PIMCOMMON_STORAGESERVICECONFIGUREDIALOG_H

#include <KDialog>
#include "pimcommon_export.h"
namespace PimCommon {
class StorageServiceAbstract;
class StorageServiceConfigureWidget;
class PIMCOMMON_EXPORT StorageServiceConfigureDialog : public KDialog
{
    Q_OBJECT
public:
    explicit StorageServiceConfigureDialog(QWidget *parent=0);
    ~StorageServiceConfigureDialog();

    QMap<QString, PimCommon::StorageServiceAbstract *> listService() const;
    void setListService(const QMap<QString, PimCommon::StorageServiceAbstract *> &lst);


    virtual void writeSettings();

    void addExtraWidget(QWidget *w);
Q_SIGNALS:
    void serviceRemoved(const QString &);

protected:
    PimCommon::StorageServiceConfigureWidget *mStorageServiceConfigureWidget;
    virtual void loadSettings();

private:
    void readConfig();
    void writeConfig();
};
}

#endif // STORAGESERVICECONFIGUREDIALOG_H
