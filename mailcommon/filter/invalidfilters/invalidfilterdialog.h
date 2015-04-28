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

#ifndef INVALIDFILTERDIALOG_H
#define INVALIDFILTERDIALOG_H

#include "mailcommon_export.h"
#include "invalidfilterinfo.h"
#include <QDialog>

namespace MailCommon
{
class InvalidFilterWidget;
class InvalidFilterInfoWidget;
class MAILCOMMON_EXPORT InvalidFilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InvalidFilterDialog(QWidget *parent = Q_NULLPTR);
    ~InvalidFilterDialog();

    void setInvalidFilters(const QVector<InvalidFilterInfo> &lst);
private:
    void writeConfig();
    void readConfig();
    InvalidFilterWidget *mInvalidFilterWidget;
    InvalidFilterInfoWidget *mInvalidFilterInfoWidget;
};
}
#endif // INVALIDFILTERDIALOG_H
