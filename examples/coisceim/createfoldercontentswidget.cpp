/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

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

#include "createfoldercontentswidget.h"
#include "foldercontentswidget.h"
#include <QPushButton>

#include "trip.h"
#include <QIcon>

CreateFolderContentsWidget::CreateFolderContentsWidget(Trip *trip, int role, const QString &type, QWidget *parent)
    : QStackedWidget(parent), m_trip(trip), m_role(role)
{
    QPushButton *button = new QPushButton(QLatin1String("Select ") + type);

    QString iconName;

    switch (role) {
    case Trip::MailCollectionRole:
        iconName = QLatin1String("kmail");
        break;
    case Trip::NotesCollectionRole:
        iconName = QLatin1String("knotes");
        break;
    case Trip::TodoCollectionRole:
        iconName = QLatin1String("korg-todo");
        break;
    }
    QIcon icon = QIcon::fromTheme(iconName);
    button->setIcon(icon);
    button->setIconSize(QSize(128, 128));
    addWidget(button);
    m_widget = new FolderContentsWidget(trip, role, type);
    addWidget(m_widget);

    connect(button, SIGNAL(clicked(bool)), m_widget, SLOT(configure()));
    connect(m_trip, SIGNAL(monitoredCollectionsChanged()), SLOT(collectionSelected()));
}

void CreateFolderContentsWidget::collectionSelected()
{
    setCurrentIndex(m_trip->collection(m_role).isValid() ? 1 : 0);
}

void CreateFolderContentsWidget::clear()
{
//   m_widget->clear();
}
