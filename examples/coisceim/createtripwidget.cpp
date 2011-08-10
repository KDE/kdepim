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

#include "createtripwidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <kconfig.h>
#include <KMessageBox>
#include <KConfig>
#include <KSharedConfigPtr>

#include <Akonadi/ChangeRecorder>

#include "createfoldercontentswidget.h"
#include "eventselectorwidget.h"
#include "trip.h"

using namespace Akonadi;

CreateTripWidget::CreateTripWidget(Trip *trip, Akonadi::ChangeRecorder* monitor, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_trip(trip), m_monitor(monitor)
{
  QVBoxLayout *layout = new QVBoxLayout(this);

  m_eventSelector = new EventSelectorWidget;
  connect(m_eventSelector, SIGNAL(selected(Akonadi::Item)), SLOT(tripSelected(Akonadi::Item)));

  layout->addWidget(m_eventSelector);

  QHBoxLayout *configureLayout = new QHBoxLayout;

  layout->addLayout(configureLayout);

  m_mailWidget = createView("Mail", Trip::MailCollectionRole);
  m_todoWidget = createView("Todos", Trip::TodoCollectionRole);
  m_notesWidget = createView("Notes", Trip::NotesCollectionRole);

  configureLayout->addWidget(m_mailWidget);
  configureLayout->addWidget(m_todoWidget);
  configureLayout->addWidget(m_notesWidget);

  QPushButton *goButton = new QPushButton("Go!");
  connect(goButton, SIGNAL(clicked(bool)), SLOT(create()));

  layout->addWidget(goButton);
}

CreateFolderContentsWidget* CreateTripWidget::createView(const QString &type, int role)
{
  return new CreateFolderContentsWidget(m_trip, role, type);
}

void CreateTripWidget::tripSelected(const Akonadi::Item& item)
{
  m_tripItem = item;
}

void addTrip(QVariantList *list, Item::Id newId)
{
  bool found = false;
  foreach(const QVariant &id, *list) {
    if (id.toLongLong() == newId)
      found = true;
  }
  if (!found)
    list->append(newId);
}

void CreateTripWidget::create()
{
  if (!m_tripItem.isValid())
  {
    KMessageBox::error(this, "You need to select an event first");
    return;
  }
  KSharedConfigPtr config = KGlobal::config();

  KConfigGroup generalGroup( config, "General" );

  QVariantList trips = generalGroup.readEntry<QVariantList>("trips", QVariantList());

  addTrip(&trips, m_tripItem.id());

  generalGroup.writeEntry("trips", trips);

  KConfigGroup tripsGroup( config, "Trips" );

  QList<Collection::Id> ids;

  ids << m_trip->collection(Trip::MailCollectionRole).id();
  ids << m_trip->collection(Trip::TodoCollectionRole).id();
  ids << m_trip->collection(Trip::NotesCollectionRole).id();

  tripsGroup.writeEntry(QString::number(m_tripItem.id()), ids);

  config->sync();

  m_monitor->setItemMonitored(m_tripItem, true);

  m_trip->setCollection(Trip::MailCollectionRole, Collection());
  m_trip->setCollection(Trip::TodoCollectionRole, Collection());
  m_trip->setCollection(Trip::NotesCollectionRole, Collection());

  m_eventSelector->clear();

  m_tripItem = Item();
}
