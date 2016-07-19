/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

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

#ifndef SELECTPROGRAMPAGE_H
#define SELECTPROGRAMPAGE_H

#include <QWidget>

class QListWidgetItem;

namespace Ui
{
class SelectProgramPage;
}

class SelectProgramPage : public QWidget
{
    Q_OBJECT

public:
    explicit SelectProgramPage(QWidget *parent = Q_NULLPTR);
    ~SelectProgramPage();

    void setFoundProgram(const QStringList &list);
    void disableSelectProgram();

Q_SIGNALS:
    void programSelected(const QString &);
    void doubleClicked();
    void selectManualSelectionChanged(bool);

private:
    void slotItemSelectionChanged();
    void slotItemDoubleClicked(QListWidgetItem *item);

    void slotSelectManualSelectionChanged(bool b);
    Ui::SelectProgramPage *ui;
};

#endif // SELECTPROGRAMPAGE_H
