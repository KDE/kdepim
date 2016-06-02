/*
   Copyright (C) 2013-2016 Montel Laurent <montel@kde.org>

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
#ifndef NEWTHEMEDIALOG_H
#define NEWTHEMEDIALOG_H

#include "grantleethemeeditor_export.h"
#include <QDialog>

namespace GrantleeThemeEditor
{
class NewThemeDialogPrivate;
class GRANTLEETHEMEEDITOR_EXPORT NewThemeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewThemeDialog(QWidget *parent = Q_NULLPTR);
    ~NewThemeDialog();

    QString themeName() const;
    QString directory() const;

private Q_SLOTS:
    void slotUpdateOkButton();

private:
    void readConfig();
    NewThemeDialogPrivate *const d;
};
}
#endif // NEWTHEMEDIALOG_H
