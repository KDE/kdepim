/*
   Copyright (C) 2016 Montel Laurent <montel@kde.org>

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

#ifndef AKREGATORCENTRALWIDGET_H
#define AKREGATORCENTRALWIDGET_H

#include <QStackedWidget>
#include "crashwidget/crashwidget.h"
namespace Akregator
{
class MainWidget;
class AkregatorCentralWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit AkregatorCentralWidget(QWidget *parent = Q_NULLPTR);
    ~AkregatorCentralWidget();

    void needToRestoreCrashedSession();

    void setMainWidget(Akregator::MainWidget *mainWidget);
    bool previousSessionCrashed() const;

Q_SIGNALS:
    void restoreSession(Akregator::CrashWidget::CrashAction type);

private Q_SLOTS:
    void slotRestoreSession(Akregator::CrashWidget::CrashAction type);

private:
    Akregator::CrashWidget *mCrashWidget;
    Akregator::MainWidget *mMainWidget;
};
}
#endif // AKREGATORCENTRALWIDGET_H
