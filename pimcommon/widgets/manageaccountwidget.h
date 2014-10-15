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

#ifndef MANAGEACCOUNTWIDGET_H
#define MANAGEACCOUNTWIDGET_H

#include <QWidget>
#include "pimcommon_export.h"
namespace Ui {
class ManageAccountWidget;
}
namespace Akonadi {
class AgentInstance;
}
namespace PimCommon {
class PIMCOMMON_EXPORT ManageAccountWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ManageAccountWidget(QWidget *parent);
    ~ManageAccountWidget();

    void setSpecialCollectionIdentifier(const QString &identifier);

    QString mimeTypeFilter() const;
    void setMimeTypeFilter(const QString &mimeTypeFilter);

    QString capabilityFilter() const;
    void setCapabilityFilter(const QString &capabilityFilter);

    QStringList excludeCapabilities() const;
    void setExcludeCapabilities(const QStringList &excludeCapabilities);

private slots:
    void slotAccountSelected(const Akonadi::AgentInstance &current);
    void slotRemoveSelectedAccount();
    void slotRestartSelectedAccount();
    void slotModifySelectedAccount();
    void slotAddAccount();
private:
    QString mSpecialCollectionIdentifier;

    QString mMimeTypeFilter;
    QString mCapabilityFilter;
    QStringList mExcludeCapabilities;

    Ui::ManageAccountWidget *mWidget;
};
}

#endif // MANAGEACCOUNTWIDGET_H
