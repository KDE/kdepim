/*
    This file is part of Akonadi.

    Copyright (c) 2012 Volker Krause <vkrause@kde.org>

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

#ifndef INSTANCESELECTOR_H
#define INSTANCESELECTOR_H

#include "mainwindow.h"

#include <QDialog>
#include <KConfigGroup>

namespace Ui
{
class InstanceSelector;
}

/** Check if there are multiple instances of Akonadi running, and if so present
 *  a list to select the one to connect to.
 */
class InstanceSelector : public QDialog
{
    Q_OBJECT
public:
    explicit InstanceSelector(const QString &remoteHost, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Q_NULLPTR);
    virtual ~InstanceSelector();

private Q_SLOTS:
    void slotAccept();
    void slotReject();

private:
    static QStringList instances();

private:
    QScopedPointer<Ui::InstanceSelector> ui;
    QString m_remoteHost;
    QString m_instance;
    MainWindow *mWindow;
};

#endif // INSTANCESELECTOR_H
