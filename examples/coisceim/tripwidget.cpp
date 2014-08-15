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

#include "tripwidget.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QTextBrowser>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include <KLocalizedString>

#include "foldercontentswidget.h"
#include "trip.h"
#include "itemselection.h"
#include "itemviewerwidget.h"

using namespace Akonadi;

TripWidget::TripWidget(Trip *trip, QWidget *parent)
  : QWidget(parent), m_trip(trip)
{
  QVBoxLayout *layout = new QVBoxLayout(this);

  QSplitter *vSplitter = new QSplitter(Qt::Vertical);

  m_eventName = new QLabel;
  m_eventBrowser = new QTextBrowser;

  layout->addWidget(m_eventName);
  layout->addWidget(m_eventBrowser);

  updateDescription();
  updateName();

  connect(trip, SIGNAL(eventDescriptionChanged()), SLOT(updateDescription()));
  connect(trip, SIGNAL(eventNameChanged()), SLOT(updateName()));

  QLineEdit *filterLine = new QLineEdit;
  layout->addWidget(filterLine);
  connect(filterLine, SIGNAL(textChanged(QString)), m_trip, SLOT(setComponentFilter(QString)));

  layout->addWidget(vSplitter);

  QSplitter *splitter = new QSplitter;

  vSplitter->addWidget(splitter);

  FolderContentsWidget *mailWidget = createView(QLatin1String("Mail"), Trip::MailCollectionRole);
  FolderContentsWidget *todoWidget = createView(QLatin1String("Todos"), Trip::TodoCollectionRole);
  FolderContentsWidget *notesWidget = createView(QLatin1String("Notes"), Trip::NotesCollectionRole);

  ItemSelection *itemSelection = new ItemSelection(mailWidget->selectionModel(),
                                                   todoWidget->selectionModel(),
                                                   notesWidget->selectionModel(), this);

  splitter->addWidget(mailWidget);
  splitter->addWidget(todoWidget);
  splitter->addWidget(notesWidget);

  ItemViewerWidget *browser = new ItemViewerWidget(itemSelection, this);
  vSplitter->addWidget(browser);

  QPushButton *deleteButton = new QPushButton(i18n("Delete"));
  connect(deleteButton, &QPushButton::clicked, this, &TripWidget::doDeleteThis);
  layout->addWidget(deleteButton);
}

FolderContentsWidget* TripWidget::createView(const QString &type, int role)
{
  return new FolderContentsWidget(m_trip, role, type);
}

void TripWidget::doDeleteThis()
{
  const_cast<QAbstractItemModel*>(m_trip->index().model())->removeRow(m_trip->index().row());
}

void TripWidget::updateDescription()
{
  m_eventBrowser->setText(m_trip->eventDescription());
}

void TripWidget::updateName()
{
  m_eventName->setText(m_trip->eventName());
}
