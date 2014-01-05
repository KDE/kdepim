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
#ifndef CONFIGUREACTIVITYWIDGET_H
#define CONFIGUREACTIVITYWIDGET_H

#include "pimactivity_export.h"

#include <QWidget>

class KTabWidget;

namespace PimActivity {
class ConfigureActivityWidgetPrivate;
class ActivityManager;
class PIMACTIVITY_EXPORT ConfigureActivityWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigureActivityWidget(PimActivity::ActivityManager *manager, QWidget *parent = 0);
    ~ConfigureActivityWidget();

    enum ConfigureType {
        Identity,
        MailTransport,
        Collection
    };
    void readConfig();
    void writeConfig();
    void defaults();

Q_SIGNALS:
    void changed(bool);

private:
    friend class ConfigureActivityWidgetPrivate;
    ConfigureActivityWidgetPrivate * const d;
    Q_PRIVATE_SLOT( d, void slotActivityChanged(const QString &))
    Q_PRIVATE_SLOT( d, void slotModified())
};
}

#endif // CONFIGUREACTIVITYWIDGET_H
