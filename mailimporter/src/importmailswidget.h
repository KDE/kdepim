/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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
#ifndef IMPORTMAILSWIDGET_H
#define IMPORTMAILSWIDGET_H

#include <QWidget>
#include "mailimporter_export.h"
class QListWidgetItem;
namespace MailImporter
{
class ImportMailsWidgetPrivate;
class MAILIMPORTER_EXPORT ImportMailsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImportMailsWidget(QWidget *parent = Q_NULLPTR);
    ~ImportMailsWidget();

    void setStatusMessage(const QString &status);
    void setFrom(const QString &from);
    void setTo(const QString &to);
    void setCurrent(const QString &current);
    void setCurrent(int percent);
    void setOverall(int percent);
    void addItem(QListWidgetItem *item);
    void setLastCurrentItem();
    void clear();
    void addInfoLogEntry(const QString &log);
    void addErrorLogEntry(const QString &log);

private:
    ImportMailsWidgetPrivate *const d;
};
}

#endif // IMPORTMAILSWIDGET_H
