/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEEDITORCONFIGURESERVERWIDGET_H
#define SIEVEEDITORCONFIGURESERVERWIDGET_H

#include <QWidget>

namespace Ui
{
class SieveEditorConfigureServerWidget;
}

class SieveEditorConfigureServerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SieveEditorConfigureServerWidget(QWidget *parent = Q_NULLPTR);
    ~SieveEditorConfigureServerWidget();

    void readConfig();
    void writeConfig();

private Q_SLOTS:
    void slotModifyServer();
    void slotAddServer();
    void slotDeleteServer();
    void slotItemSelectionChanged();

private:
    Ui::SieveEditorConfigureServerWidget *ui;
};

#endif // SIEVEEDITORCONFIGURESERVERWIDGET_H
