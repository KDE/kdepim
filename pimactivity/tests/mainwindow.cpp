/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "mainwindow.h"
#include "pimactivity/pimwidgets/identity/identitymanageractivity.h"
#include "pimactivity/pimwidgets/mailtransport/transportcomboboxactivity.h"
#include "pimactivity/pimwidgets/identity/identitycomboboxactivity.h"
#include "pimactivity/activitymanager.h"

#include <QVBoxLayout>

MainWindow::MainWindow()
    : QMainWindow()
{
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    PimActivity::TransportComboboxActivity *combobox = new PimActivity::TransportComboboxActivity;
    lay->addWidget(combobox);

    PimActivity::ActivityManager *manager = new PimActivity::ActivityManager(this);

    PimActivity::IdentityManagerActivity *identityManager = new PimActivity::IdentityManagerActivity(manager,false, this, "identitymanager");
    PimActivity::IdentityComboboxActivity *identityCombobox = new PimActivity::IdentityComboboxActivity(identityManager);
    lay->addWidget(identityCombobox);

    w->setLayout(lay);
    setCentralWidget(w);
}

