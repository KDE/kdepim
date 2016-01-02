/*
  Copyright (c) 2014-2016 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEEDITORCENTRALWIDGET_H
#define SIEVEEDITORCENTRALWIDGET_H

#include <QStackedWidget>
class SieveEditorConfigureServerPage;
class SieveEditorMainWidget;
class SieveEditorCentralWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit SieveEditorCentralWidget(QWidget *parent = Q_NULLPTR);
    ~SieveEditorCentralWidget();

    SieveEditorMainWidget *sieveEditorMainWidget() const;

Q_SIGNALS:
    void configureClicked();

private Q_SLOTS:
    void slotServerSieveFound(bool hasServer);

private:
    SieveEditorConfigureServerPage *mConfigureWidget;
    SieveEditorMainWidget *mSieveEditorMainWidget;
};

#endif // SIEVEEDITORCENTRALWIDGET_H
