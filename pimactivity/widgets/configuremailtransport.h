/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef CONFIGUREMAILTRANSPORT_H
#define CONFIGUREMAILTRANSPORT_H

#include "abstractconfigurewidget.h"
#include <QWidget>
class QListWidget;

namespace PimActivity {
class ConfigureMailtransport : public QWidget, public AbstractConfigureWidget
{
    Q_OBJECT
public:
    explicit ConfigureMailtransport(QWidget *parent = 0);
    ~ConfigureMailtransport();

    void writeConfig(const QString &id);
    void setDefault();

private:
    void readConfig(const QString &id);

Q_SIGNALS:
    void changed(bool b = true);

private:
    enum transportId {
        TransportID = Qt::UserRole +1
    };

    void init();
    QListWidget *mListTransport;
};
}

#endif // CONFIGUREMAILTRANSPORT_H
