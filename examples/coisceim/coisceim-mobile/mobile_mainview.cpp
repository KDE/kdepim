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

#include "mobile_mainview.h"

#include <QBoxLayout>
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>


#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/ItemFetchScope>

#include "tripmodel.h"
#include <qdeclarative.h>
#include <mobile/lib/calendar/kcalitembrowseritem.h>
#include <itemselection.h>
#include <messageviewitem.h>
#include <QStandardPaths>

using namespace Akonadi;

MobileMainview::MobileMainview(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    resize(800, 480);
    QHBoxLayout *layout = new QHBoxLayout(this);

    ChangeRecorder *tripRec = new ChangeRecorder(this);
    tripRec->itemFetchScope().fetchFullPayload(true);
    TripModel *tripModel = new TripModel(tripRec, this);

    qmlRegisterType<CalendarSupport::KCal::KCalItemBrowserItem>("org.kde.kcal", 4, 5, "IncidenceView");
    qmlRegisterType<MessageViewer::MessageViewItem>("org.kde.messageviewer", 4, 5, "MessageView");

    QDeclarativeView *view = new QDeclarativeView;
    view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    QDeclarativeContext *context = view->engine()->rootContext();

    context->setContextProperty(QLatin1String("_tripModel"), tripModel);

    view->setSource(QUrl(QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("main.qml"))));

    layout->addWidget(view);
}
