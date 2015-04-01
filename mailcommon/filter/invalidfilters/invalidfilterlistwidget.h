/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef INVALIDFILTERLISTWIDGET_H
#define INVALIDFILTERLISTWIDGET_H

#include <QListWidget>
#include "mailcommon_export.h"

namespace MailCommon
{
class InvalidFilterListWidgetItem : public QListWidgetItem
{
public:
    explicit InvalidFilterListWidgetItem(QListWidget *parent = Q_NULLPTR);
    ~InvalidFilterListWidgetItem();
    void setInformation(const QString &information);
private:
    QString mInformation;
};

class MAILCOMMON_EXPORT InvalidFilterListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit InvalidFilterListWidget(QWidget *parent = Q_NULLPTR);
    ~InvalidFilterListWidget();
    void setInvalidFilter(const QStringList &lst);
};
}
#endif // INVALIDFILTERLISTWIDGET_H
