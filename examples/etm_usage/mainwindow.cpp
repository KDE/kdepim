/*
    This file is part of Akonadi.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "mainwindow.h"


#include "tab1widget.h"
#include "tab2widget.h"
#include "tab2_5widget.h"
#include "tab3widget.h"
#include "tab4widget.h"
#include "tab5widget.h"
#include "tab6widget.h"


MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags)
{
  QTabWidget *tabWidget = new QTabWidget(this);

  tabWidget->addTab(new Tab1Widget(tabWidget), "EntityTreeModel");
  tabWidget->addTab(new Tab2Widget(tabWidget), "setRootIndex");
  tabWidget->addTab(new Tab2_5Widget(tabWidget), "Type specific data");
  tabWidget->addTab(new Tab3Widget(tabWidget), "KSelectionProxyModel");
  tabWidget->addTab(new Tab4Widget(tabWidget), "KSelectionProxyModel Filtered");
  tabWidget->addTab(new Tab5Widget(tabWidget), "Categorized Items");
  tabWidget->addTab(new Tab6Widget(tabWidget), "Checkable Collections");

  setCentralWidget(tabWidget);
}
