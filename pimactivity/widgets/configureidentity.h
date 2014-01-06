/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef CONFIGUREIDENTITY_H
#define CONFIGUREIDENTITY_H

#include "abstractconfigurewidget.h"
#include <QWidget>

class QListWidget;
namespace KPIMIdentities {
class IdentityManager;
}

namespace PimActivity {

class ConfigureIdentity : public QWidget, public AbstractConfigureWidget
{
    Q_OBJECT
public:
    explicit ConfigureIdentity(QWidget *parent);
    ~ConfigureIdentity();

    void writeConfig(const QString &id);

    void setDefault();

private:
    void readConfig(const QString &id);


Q_SIGNALS:
    void changed(bool b = true);

private:
    enum identityId {
        IdentityID = Qt::UserRole +1
    };

    void init();
    QListWidget *mListIdentity;
    KPIMIdentities::IdentityManager *mManager;
};

}

#endif // CONFIGUREIDENTITY_H
