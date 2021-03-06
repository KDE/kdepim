/*
   Copyright (C) 2015-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMPORTEXPORTPROGRESSINDICATORBASE_H
#define IMPORTEXPORTPROGRESSINDICATORBASE_H

#include <QObject>
#include "pimsettingexporter_export.h"
class PIMSETTINGEXPORTER_EXPORT ImportExportProgressIndicatorBase : public QObject
{
    Q_OBJECT
public:
    explicit ImportExportProgressIndicatorBase(QObject *parent = Q_NULLPTR);
    ~ImportExportProgressIndicatorBase();

    virtual void increaseProgressDialog();
    virtual void createProgressDialog(const QString &title);

    virtual void setProgressDialogLabel(const QString &text);
    virtual bool wasCanceled() const;
    virtual int mergeConfigMessageBox(const QString &configName) const;
    virtual bool overwriteConfigMessageBox(const QString &configName) const;
    virtual bool overwriteDirectoryMessageBox(const QString &directory) const;
    virtual void showErrorMessage(const QString &message, const QString &title);

    void setNumberOfStep(int numberOfStep);

Q_SIGNALS:
    void info(const QString &text);
    void canceled();

protected:
    int mNumberOfStep;
};

#endif // IMPORTEXPORTPROGRESSINDICATORBASE_H
