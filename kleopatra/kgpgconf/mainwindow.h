/* -*- mode: c++; c-basic-offset:4 -*-
    mainwindow.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef KGPGCONF_MAINWINDOW_H
#define KGPGCONF_MAINWINDOW_H

#include <QHash>
#include <QMainWindow>

#include "ui_mainwidget.h"

class QTreeWidgetItem;

class Config;
class ConfigComponent;
class ConfigEntry;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~MainWindow();

private:
    enum Column {
        NameColumn = 0,
        ReadOnlyColumn = 1
    };

    enum Role {
        IsOptionRole = Qt::UserRole
    };

    void readConfiguration();

private Q_SLOTS:
    void treeWidgetItemSelectionChanged();
    void treeWidgetItemChanged(QTreeWidgetItem *item, int column);
    void readOnlyStateChanged(int state);
    void optionValueChanged();
    void useDefaultToggled(bool useDefault);
    void saveAs();
    void delayedInit();

private:
    void saveToFile(const QString &filename);

private:
    Ui::MainWidget m_ui;
    Config *m_config;
    QHash<QTreeWidgetItem *, ConfigEntry *> m_itemToEntry;
    QHash<ConfigEntry *, QTreeWidgetItem *> m_entryToItem;
    QHash<ConfigEntry *, ConfigComponent *> m_componentForEntry;
    ConfigEntry *m_selectedEntry;
};

#endif // KGPGCONF_MAINWINDOW_H

