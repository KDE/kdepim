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

#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <KConfigDialog>
#include "bilboblog.h"

namespace PimCommon {
class StorageServiceManager;
}

class ConfigureStorageServiceWidget;
class ConfigureDialog : public KConfigDialog
{
    Q_OBJECT
public:
    explicit ConfigureDialog(PimCommon::StorageServiceManager *storageManager, QWidget *parent, const QString& name, KConfigSkeleton *config );
    ~ConfigureDialog();

Q_SIGNALS:
    void blogRemoved(int);
    void blogAdded(const BilboBlog &);
    void blogEdited(const BilboBlog &);
    void dialogDestroyed(QObject*);
    void settingsChanged();

private:
    ConfigureStorageServiceWidget *mConfigStorageService;
};

#endif // CONFIGUREDIALOG_H
